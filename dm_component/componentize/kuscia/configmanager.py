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

import os
from typing import List, Tuple

import requests


def make_url(scheme: str, endpoint, *res):
    url = scheme + "://" + endpoint
    for r in res:
        url = "{}/{}".format(url, r)
    return url


def get_cert_from_config_manager(endpoint: str, data: dict) -> Tuple[List[str], str]:
    """Get private key and corresponding certificate from config manager

    Args:
        endpoint (str): config manager endpoint
        data (dict): request paramters

    Returns:
        Tuple[str, str]: (certificate, private_key)
    """
    env_client_cert_file = os.environ.get("CLIENT_CERT_FILE", "")
    env_client_key_file = os.environ.get("CLIENT_PRIVATE_KEY_FILE", "")
    env_trusted_ca_file = os.environ.get("TRUSTED_CA_FILE", "")

    # mTLS enabled.
    r = requests.post(
        make_url("https", endpoint, "api/v1/cm/certificate/generate"),
        verify=env_trusted_ca_file,
        cert=(env_client_cert_file, env_client_key_file),
        json=data,
    )
    assert r.status_code == 200, f"error msg: {r.json()}"
    content = r.json()
    assert content["status"]["code"] == 0

    return (content["cert_chain"], content["key"])
