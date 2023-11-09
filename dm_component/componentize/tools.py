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

import re
from typing import Optional, Tuple

# . 一次或多次,()结果整体为一个分组
_netloc_re = re.compile(r"^(.+):(\d+)$")


def split_host_and_port(endpoint: str) -> Tuple[str, Optional[int]]:
    """
    取出IP地址和端口 返回：`(host, port)` 元组
    """
    match = _netloc_re.match(endpoint)
    if match:
        host = match.group(1)
        port = int(match.group(2))  # type: Optional[int]
    else:
        host = endpoint
        port = 80
    return (host, port)


def make_url(scheme: str, endpoint, *res):
    url = scheme + "://" + endpoint
    for r in res:
        url = "{}/{}".format(url, r)
    return url
