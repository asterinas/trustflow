# /bin/python3

# Copyright 2023 Ant Group Co., Ltd.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      https://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import argparse
import logging
import os
import sys
from typing import List
from urllib import parse

import requests
from componentize import compdef, constants, tools
from componentize.component.eval_param_reader import EvalParamReader
from componentize.kuscia import configmanager, datamesh, kusica_task_config
from google.protobuf import json_format
from requests_toolbelt.multipart.encoder import MultipartEncoder
from sdc.capsule_manager_frame import CapsuleManagerFrame
from sdc.util import crypto, tool
from tenacity import retry, stop_after_attempt, wait_exponential


def get_domain_data_id(uri: str) -> str:
    split_uri = parse.urlsplit(uri)
    assert split_uri.scheme == "dm", "only support dm sheme uri"
    query = dict(parse.parse_qsl(split_uri.query))
    return query["id"]


def start_server(port: int, data_mesh_endpoint: str):
    # start server
    import subprocess

    dir_path = os.path.dirname(os.path.realpath(__file__))
    dmserver_cmd = [
        os.path.join(dir_path, "dmserver"),
        "-p",
        f"{port}",
        "-d",
        f"{data_mesh_endpoint}",
    ]
    process = subprocess.run(dmserver_cmd, capture_output=True, shell=False)

    if process.returncode != 0:
        err_msg = f"Failed to start server, start command: {dmserver_cmd}, stderr: {process.stderr}"
        logging.critical(err_msg)
        logging.critical("This process will exit now!")
        sys.exit(-1)
    else:
        if process.stdout:
            logging.info(process.stdout.decode(errors="ignore"))
        logging.info(f"Succeeded to exit.")


def eval_upload(
    task_conf: kusica_task_config.KusicaTaskConfig, certs: List[bytes], private_key: str
):
    assert (
        len(task_conf.task_cluster_def.parties) == 2
    ), "Only two parties are enabled for uploading."
    reader = EvalParamReader(
        instance=task_conf.sf_node_eval_param,
        definition=compdef.data_upload_comp.definition(),
    )
    logging.info("##### task config parsed #####")

    party_name = task_conf.party_name
    logging.info(f"self party: {party_name}")
    uploader_party_name = reader.get_attr("uploader/domain_id")

    # if this is uploader, then upload file
    if party_name == uploader_party_name:
        logging.info("this is client")
        # 1. parse `task_cluster_def` to get peer endpoint.
        peer_party_idx = task_conf.party_idx ^ 1
        peer_party_name = task_conf.task_cluster_def.parties[peer_party_idx].name
        logging.info(f"peer party name: {peer_party_name}")
        peer_endpoint = ""
        for party in task_conf.task_cluster_def.parties:
            if party.name == peer_party_name:
                for service in party.services:
                    if service.port_name == "tee-dm":
                        peer_endpoint = service.endpoints[0]
                        break
        logging.info(f"peer endpoint: {peer_endpoint}")

        # 2. get input info
        input = reader.get_input(name="uploader_input")
        domain_data_id = get_domain_data_id(input.data_refs[0].uri)
        logging.info(f"domain data id: {domain_data_id}")

        # 3. request data mesh to get domain data
        stub = datamesh.create_domain_data_service_stub(args.data_mesh_endpoint_grpc)
        domain_data = datamesh.get_domain_data(stub, domain_data_id)
        domain_data_json_obj = json_format.MessageToJson(
            domain_data, preserving_proto_field_name=True
        )

        # 4. request data mesh to get domain data source
        stub = datamesh.create_domain_data_source_service_stub(
            args.data_mesh_endpoint_grpc
        )
        domain_data_source = datamesh.get_domain_data_source(
            stub, domain_data.datasource_id
        )

        # 5. concat data source directory with relative_uri to get source file path
        # NOTE: so far as, data mesh only support `localfs` data source
        file_path = os.path.join(
            domain_data_source.info.localfs.path, domain_data.relative_uri
        )
        logging.info(f"source file path: {file_path}")

        # 6. encrypt file
        logging.info("##### encrypting file #####")
        data_key = crypto.gen_key(constants.AES_KEY_LEN_IN_BYTES)
        encrypted_file_path = file_path + ".encrypted"
        crypto.encrypt_file(file_path, encrypted_file_path, data_key)
        logging.info("##### encrypting file succeed #####")

        # 7. upload encrypted file
        output_uri = reader.get_output_uri("receive_output")
        logging.info("##### file sending #####")

        # TODO: make retry scheme configurable
        @retry(
            wait=wait_exponential(multiplier=1, min=4, max=10),
            stop=stop_after_attempt(5),
        )
        def upload_file():
            with open(encrypted_file_path, "rb") as encrypted_file:
                files = MultipartEncoder(
                    fields={
                        # `file` is the encrypted file
                        # `store_path` is the path the file saved in receiver
                        # `domain_data` is the meta info of this file
                        "file": (
                            os.path.basename(encrypted_file_path),
                            encrypted_file,
                            "application/octet-stream",
                        ),
                        "store_path": (None, output_uri),
                        "domain_data": (None, domain_data_json_obj),
                    }
                )
                response = requests.post(
                    tools.make_url("http", peer_endpoint, "upload"),
                    data=files,
                    headers={"Content-Type": files.content_type},
                )
                assert response.status_code == 200, f"err msg: {response.text}"

        upload_file()
        logging.info("##### file send success #####")

        # 8. shutdown server if send successfully
        logging.info("shudown server")
        response = requests.post(tools.make_url("http", peer_endpoint, "shutdown"))
        assert response.status_code == 200, f"err msg: {response.text}"
        logging.info("shudown server success")

        # 9. register to capsule manager
        logging.info("##### register to capsule manager #####")
        (capsule_manager_host, capsule_manager_port) = tools.split_host_and_port(
            task_conf.capsule_manager_endpoint
        )
        cm = CapsuleManagerFrame(
            f"{capsule_manager_host}:{capsule_manager_port}", "", None, True
        )

        party_id = tool.generate_party_id_from_cert(certs[-1])
        cm.create_data_keys(
            party_id, [get_domain_data_id(output_uri)], [data_key], certs, private_key
        )
    # if
    else:
        logging.info("this is server")
        server_port = 0
        for port in task_conf.task_allocated_ports.ports:
            if port.name == "tee-dm":
                server_port = port.port
        start_server(server_port, args.data_mesh_endpoint)


def eval_authorize(
    task_conf: kusica_task_config.KusicaTaskConfig, certs: List[bytes], private_key: str
):
    assert (
        len(task_conf.task_cluster_def.parties) == 1
    ), "Only one party is enabled for authorizing."
    reader = EvalParamReader(
        instance=task_conf.sf_node_eval_param,
        definition=compdef.data_auth_comp.definition(),
    )
    logging.info("##### task config parsed #####")

    project_id = reader.get_attr("authorization_info/project_id")
    # last cert will be party id
    owner_party_id = tool.generate_party_id_from_cert(certs[-1])
    # get input
    input = reader.get_input(name="authorize_input")
    domain_data_id = get_domain_data_id(input.data_refs[0].uri)
    # columns
    columns = reader.get_attr("authorization_info/columns")
    # root certificates of the authorized parties
    root_certs = reader.get_attr("authorization_info/root_certs")
    root_certs = list(map(lambda x: base64.b64decode(x), root_certs))
    # authorized parties
    grantee_party_ids = list(
        map(lambda x: tool.generate_party_id_from_cert(x), root_certs)
    )
    (capsule_manager_host, capsule_manager_port) = tools.split_host_and_port(
        task_conf.capsule_manager_endpoint
    )
    cm = CapsuleManagerFrame(
        f"{capsule_manager_host}:{capsule_manager_port}", "", None, True
    )
    cm.create_data_policy(
        owner_party_id=owner_party_id,
        scope=project_id,
        data_uuid=domain_data_id,
        rule_ids=["default_rule_id"],
        grantee_party_ids=[grantee_party_ids],
        columns=[columns],
        op_constraints_name=["*"],
        cert_pems=certs,
        private_key=private_key,
    )
    logging.info("create data policy success")


def eval_data_export(
    task_conf: kusica_task_config.KusicaTaskConfig, certs: List[bytes], private_key: str
):
    assert (
        len(task_conf.task_cluster_def.parties) == 2
    ), "Only the two parties are enabled in data export."

    reader = EvalParamReader(
        instance=task_conf.sf_node_eval_param,
        definition=compdef.data_export_comp.definition(),
    )
    logging.info("##### task config parsed #####")

    party_name = task_conf.party_name
    logging.info(f"self party: {party_name}")

    receiver_party_name = reader.get_attr("receiver/domain_id")
    # if this is not receiver, then start server
    if party_name == receiver_party_name:
        logging.info("this is receiver")

        peer_party_idx = task_conf.party_idx ^ 1
        peer_party_name = task_conf.task_cluster_def.parties[peer_party_idx].name
        logging.info(f"peer party name: {peer_party_name}")
        peer_endpoint = ""
        for party in task_conf.task_cluster_def.parties:
            if party.name == peer_party_name:
                for service in party.services:
                    if service.port_name == "tee-dm":
                        peer_endpoint = service.endpoints[0]
                        break
        logging.info(f"peer endpoint: {peer_endpoint}")

        # get input
        input = reader.get_input(name="sender_input")
        download_uri = input.data_refs[0].uri
        domain_data_id = get_domain_data_id(download_uri)

        # 1. get export data key from capsule manager
        logging.info("##### getting export data key from capsule manager #####")
        (capsule_manager_host, capsule_manager_port) = tools.split_host_and_port(
            task_conf.capsule_manager_endpoint
        )
        cm = CapsuleManagerFrame(
            f"{capsule_manager_host}:{capsule_manager_port}", "", None, True
        )
        # party id is derived from last cert
        party_id = tool.generate_party_id_from_cert(certs[-1])
        vote_result = reader.get_attr("receiver/vote_result")
        # call rpc
        data_key = cm.get_export_data_key(
            request_party_id=party_id,
            resource_uri=domain_data_id,
            data_export_certificate=vote_result,
            cert_pems=certs,
            private_key=private_key,
        )
        logging.info("##### get export data key successfully #####")

        # 2. download the encrytped data from sender
        logging.info("##### file downloading #####")

        # TODO: make retry scheme configurable
        @retry(
            wait=wait_exponential(multiplier=1, min=4, max=10),
            stop=stop_after_attempt(5),
        )
        def download_file(local_filename: str):
            response = requests.post(
                tools.make_url("http", peer_endpoint, "download"),
                json={"uri": download_uri},
            )
            assert response.status_code == 200, f"err msg: {response.text}"
            with open(local_filename, "wb") as f:
                for chunk in response.iter_content(chunk_size=8192):
                    f.write(chunk)

        output_uri = reader.get_output_uri("receiver_output")
        split_uri = parse.urlsplit(output_uri)
        assert split_uri.scheme == "dm", "only support dm sheme uri"
        query = dict(parse.parse_qsl(split_uri.query))
        local_file_relative_path = query["uri"]
        domain_data_id = query["id"]
        data_source_id = query["datasource_id"]
        download_file(local_file_relative_path)
        logging.info("##### file download succeed #####")

        # 3. decrypt data to dest path
        logging.info("##### Start to decrypt data #####")
        # decrypt data to dest file path
        stub = datamesh.create_domain_data_source_service_stub(
            args.data_mesh_endpoint_grpc
        )
        domain_data_source = datamesh.get_domain_data_source(stub, data_source_id)
        # concat data source directory with relative_uri to get source file path
        # NOTE: so far as, data mesh only support `localfs` data source
        file_path = os.path.join(
            domain_data_source.info.localfs.path, local_file_relative_path
        )
        logging.info(f"file path: {file_path}")
        crypto.decrypt_file(local_file_relative_path, file_path, data_key)
        logging.info("##### Decrypt data succeed #####")

        # 4. get meta data from sender
        # TODO: make retry scheme configurable
        @retry(
            wait=wait_exponential(multiplier=1, min=4, max=10),
            stop=stop_after_attempt(5),
        )
        def get_peer_domain_data(uri: str) -> datamesh.DomainData:
            response = requests.post(
                tools.make_url("http", peer_endpoint, "get_domain_data"),
                json={"uri": download_uri},
            )
            assert response.status_code == 200, f"err msg: {response.text}"
            print(f"{response.json()}")
            return json_format.ParseDict(
                response.json(), datamesh.DomainData(), ignore_unknown_fields=True
            )

        # 5. register data meta to data mesh
        logging.info("##### Start to register data to data mesh #####")
        domain_data = get_peer_domain_data(uri=download_uri)
        domain_data.domaindata_id = domain_data_id
        domain_data.relative_uri = local_file_relative_path

        stub = datamesh.create_domain_data_service_stub(args.data_mesh_endpoint_grpc)
        datamesh.create_domain_data(stub, domain_data)
        logging.info("##### Register data to data mesh succeed #####")

        # 6. shutdown server
        logging.info("shudown server")
        response = requests.post(tools.make_url("http", peer_endpoint, "shutdown"))
        assert response.status_code == 200, f"err msg: {response.text}"
        logging.info("shudown server success")
    else:
        logging.info("this is sender")
        server_port = 0
        for port in task_conf.task_allocated_ports.ports:
            if port.name == "tee-dm":
                server_port = port.port
        start_server(server_port, args.data_mesh_endpoint)


def eval_data_delete(
    task_conf: kusica_task_config.KusicaTaskConfig, certs: List[bytes], private_key: str
):
    assert (
        len(task_conf.task_cluster_def.parties) == 1
    ), "Only one party is enabled for deleting."
    reader = EvalParamReader(
        instance=task_conf.sf_node_eval_param,
        definition=compdef.data_delete_comp.definition(),
    )
    logging.info("##### task config parsed #####")

    from sdc.util import tool

    # last cert will be party id
    party_id = tool.generate_party_id_from_cert(certs[-1])
    # get input
    input = reader.get_input(name="delete_input")
    domain_data_id = get_domain_data_id(input.data_refs[0].uri)

    (capsule_manager_host, capsule_manager_port) = tools.split_host_and_port(
        task_conf.capsule_manager_endpoint
    )
    cm = CapsuleManagerFrame(
        f"{capsule_manager_host}:{capsule_manager_port}", "", None, True
    )
    cm.delete_data_key(
        owner_party_id=party_id,
        resource_uri=domain_data_id,
        cert_pems=certs,
        private_key=private_key,
    )
    logging.info("delete data key success")


def eval_data_unauthorize(
    task_conf: kusica_task_config.KusicaTaskConfig, certs: List[bytes], private_key: str
):
    assert (
        len(task_conf.task_cluster_def.parties) == 1
    ), "Only one party is enabled for eval_data_unauthorizing."
    reader = EvalParamReader(
        instance=task_conf.sf_node_eval_param,
        definition=compdef.data_unauth_comp.definition(),
    )
    logging.info("##### task config parsed #####")

    # last cert will be party id
    party_id = tool.generate_party_id_from_cert(certs[-1])
    # get input
    input = reader.get_input(name="unauthorize_input")
    domain_data_id = get_domain_data_id(input.data_refs[0].uri)

    (capsule_manager_host, capsule_manager_port) = tools.split_host_and_port(
        task_conf.capsule_manager_endpoint
    )
    cm = CapsuleManagerFrame(
        f"{capsule_manager_host}:{capsule_manager_port}", "", None, True
    )
    cm.delete_data_policy(
        owner_party_id=party_id,
        scope=task_conf.scope,
        data_uuid=domain_data_id,
        cert_pems=certs,
        private_key=private_key,
    )
    logging.info("unauthorize data success")


if __name__ == "__main__":

    parser = argparse.ArgumentParser(
        prog="TEE data management",
        description="TEE data management",
        epilog="TEE data management",
    )
    parser.add_argument("--task_config_path")
    parser.add_argument("--data_mesh_endpoint", default="datamesh:8070")
    parser.add_argument("--data_mesh_endpoint_grpc", default="datamesh:8071")
    parser.add_argument("--config_manager_endpoint", default="confmanager:8060")
    logging.getLogger("urllib3").setLevel(logging.ERROR)

    args = parser.parse_args()
    root = logging.getLogger()
    root.setLevel(logging.DEBUG)

    handler = logging.StreamHandler(sys.stdout)
    handler.setLevel(logging.DEBUG)
    formatter = logging.Formatter(
        "%(asctime)s - %(name)s - %(levelname)s - %(message)s"
    )
    handler.setFormatter(formatter)
    root.addHandler(handler)

    task_conf = kusica_task_config.KusicaTaskConfig.from_file(args.task_config_path)

    logging.info("##### getting private key #####")
    # get sub (cert, private key)-pairs from config manager
    (certs, private_key) = configmanager.get_cert_from_config_manager(
        args.config_manager_endpoint,
        {"common_name": task_conf.sf_node_eval_param.name},
    )
    import base64

    certs = list(map(lambda x: base64.b64decode(x), certs))
    logging.info(f"certs: {certs}")
    private_key = base64.b64decode(private_key)
    logging.info("##### getting private key succeed #####")

    if task_conf.sf_node_eval_param.name == "upload":
        eval_upload(task_conf, certs, private_key)
    elif task_conf.sf_node_eval_param.name == "authorize":
        eval_authorize(task_conf, certs, private_key)
    elif task_conf.sf_node_eval_param.name == "download":
        eval_data_export(task_conf, certs, private_key)
    elif task_conf.sf_node_eval_param.name == "delete":
        eval_data_delete(task_conf, certs, private_key)
    elif task_conf.sf_node_eval_param.name == "unauthorize":
        eval_data_unauthorize(task_conf, certs, private_key)
    else:
        raise Exception("unsupported component")
