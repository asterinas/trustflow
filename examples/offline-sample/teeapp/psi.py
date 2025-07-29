import pandas
import requests
from google.protobuf import json_format
from secretflowapis.v2.sdc.data_capsule_proxy import data_capsule_proxy_pb2

# replace with your capsule manager endpoint
_CAPSULE_MANAGER_ENDPOINT = "172.31.9.1:8888"
_GET_INPUT_DATA_URL = "http://data-capsule-proxy:50003/DataCapsuleProxy/GetInputData"
_PUT_RESULT_DATA_URL = "http://data-capsule-proxy:50003/DataCapsuleProxy/PutResultData"

_CONTENT_TYPE = "Content-Type"
_JSON_CONTENT_TYPE = "application/json"

_DEFAULT_SCOPE = "default"
_OP_NAME = "psi"
_ALICE_RESOURCE_URI = "breast_cancer_alice"
_BOB_RESOURCE_URI = "breast_cancer_bob"

_ALICE_ENC_FILE_PATH = "/shared/data/alice.csv.enc"
_BOB_ENC_FILE_PATH = "/shared/data/bob.csv.enc"
_DEST_DIR = "/shared/data"
_ALICE_DEC_FILE_PATH = "/shared/data/alice.csv"
_BOB_DEC_FILE_PATH = "/shared/data/bob.csv"

_JOIN_FILE_PATH = "/shared/data/join.csv"
_JOIN_RESOURCE_URI = "join_uuid"


def get_alice_data():
    request = data_capsule_proxy_pb2.GetInputDataRequest()
    resource_config = data_capsule_proxy_pb2.CmResourceConfig()
    resource_config.endpoint = _CAPSULE_MANAGER_ENDPOINT
    resource_config.scope = _DEFAULT_SCOPE
    resource_config.op_name = _OP_NAME
    resource_config.resource_uri = _ALICE_RESOURCE_URI
    request.cm_resource_config.CopyFrom(resource_config)
    # You can use s3_config instead of local_fs_config.
    # And then you can download alice.csv.enc from oss.
    request.local_fs_config.path = _ALICE_ENC_FILE_PATH
    request.dest_config.path = _DEST_DIR

    headers = {_CONTENT_TYPE: _JSON_CONTENT_TYPE}
    response = requests.post(
        _GET_INPUT_DATA_URL, headers=headers, data=json_format.MessageToJson(request)
    )
    assert (
        response.status_code == 200
    ), f"status code: {response.status_code}, response: {response.text}"


def get_bob_data():
    request = data_capsule_proxy_pb2.GetInputDataRequest()
    resource_config = data_capsule_proxy_pb2.CmResourceConfig()
    resource_config.endpoint = _CAPSULE_MANAGER_ENDPOINT
    resource_config.scope = _DEFAULT_SCOPE
    resource_config.op_name = _OP_NAME
    resource_config.resource_uri = _BOB_RESOURCE_URI
    request.cm_resource_config.CopyFrom(resource_config)
    # You can use s3_config instead of local_fs_config.
    # And then you can download bob.csv.enc from oss.
    request.local_fs_config.path = _BOB_ENC_FILE_PATH
    request.dest_config.path = _DEST_DIR

    headers = {_CONTENT_TYPE: _JSON_CONTENT_TYPE}
    response = requests.post(
        _GET_INPUT_DATA_URL, headers=headers, data=json_format.MessageToJson(request)
    )
    assert (
        response.status_code == 200
    ), f"status code: {response.status_code}, response: {response.text}"


def psi():
    alice_df = pandas.read_csv(_ALICE_DEC_FILE_PATH)
    bob_df = pandas.read_csv(_BOB_DEC_FILE_PATH)

    merged_df = pandas.merge(alice_df, bob_df, on="id")
    merged_df.to_csv(_JOIN_FILE_PATH, index=False)


def put_result_data():
    request = data_capsule_proxy_pb2.PutResultDataRequest()

    cm_result_config = data_capsule_proxy_pb2.CmResultConfig()
    cm_result_config.endpoint = _CAPSULE_MANAGER_ENDPOINT
    cm_result_config.scope = _DEFAULT_SCOPE
    cm_result_config.resource_uri = _JOIN_RESOURCE_URI
    cm_result_config.ancestor_uuids.append(_ALICE_RESOURCE_URI)
    cm_result_config.ancestor_uuids.append(_BOB_RESOURCE_URI)

    request.cm_result_config.CopyFrom(cm_result_config)
    request.source_config.path = _JOIN_FILE_PATH
    # You can use s3_config instead of local_fs_config.
    # And then you can upload join.csv.enc to oss.
    request.local_fs_config.path = _DEST_DIR

    headers = {_CONTENT_TYPE: _JSON_CONTENT_TYPE}
    response = requests.post(
        _PUT_RESULT_DATA_URL, headers=headers, data=json_format.MessageToJson(request)
    )
    assert (
        response.status_code == 200
    ), f"status code: {response.status_code}, response: {response.text}"


if __name__ == "__main__":
    get_alice_data()
    get_bob_data()
    psi()
    put_result_data()
