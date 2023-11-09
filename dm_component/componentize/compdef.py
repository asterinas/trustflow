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

from componentize.component.component import Component, IoType
from componentize.component.data_utils import DistDataType

data_upload_comp = Component(
    "upload",
    domain="data_management",
    version="0.0.1",
    desc="data upload.",
)

data_upload_comp.str_attr(
    name="uploader/domain_id",
    desc="uploader domain id",
    is_list=False,
    is_optional=False,
    default_value="",
    allowed_values=None,
)

data_upload_comp.io(
    io_type=IoType.INPUT,
    name="uploader_input",
    desc="Input table need to be uploaded.",
    types=[DistDataType.INDIVIDUAL_TABLE],
    col_params=None,
)

data_upload_comp.io(
    io_type=IoType.OUTPUT,
    name="receive_output",
    desc="Output table",
    types=[DistDataType.INDIVIDUAL_TABLE],
    col_params=None,
)

data_auth_comp = Component(
    "authorize",
    domain="data_management",
    version="0.0.1",
    desc="data upload.",
)

data_auth_comp.str_attr(
    name="owner/domain_id",
    desc="owner domain id",
    is_list=False,
    is_optional=False,
    default_value="",
    allowed_values=None,
)

data_auth_comp.str_attr(
    name="authorization_info/domain_ids",
    desc="authorization_info domain_ids",
    is_list=True,
    is_optional=False,
    default_value="",
    allowed_values=None,
)

data_auth_comp.str_attr(
    name="authorization_info/root_certs",
    desc="authorization_info root_certs",
    is_list=True,
    is_optional=False,
    default_value="",
    allowed_values=None,
)

data_auth_comp.str_attr(
    name="authorization_info/project_id",
    desc="authorization_info project id",
    is_list=False,
    is_optional=False,
    default_value="",
    allowed_values=None,
)

data_auth_comp.str_attr(
    name="authorization_info/columns",
    desc="authorization_info columns",
    is_list=True,
    is_optional=False,
    default_value="",
    allowed_values=None,
)

data_auth_comp.io(
    io_type=IoType.INPUT,
    name="authorize_input",
    desc="Input table need to be authorized.",
    types=[DistDataType.INDIVIDUAL_TABLE],
    col_params=None,
)

data_export_comp = Component(
    "download",
    domain="data_management",
    version="0.0.1",
    desc="data download.",
)

data_export_comp.str_attr(
    name="receiver/domain_id",
    desc="receiver domain id",
    is_list=False,
    is_optional=False,
    default_value="",
    allowed_values=None,
)

data_export_comp.str_attr(
    name="receiver/vote_result",
    desc="receiver vote result",
    is_list=False,
    is_optional=False,
    default_value="",
    allowed_values=None,
)


data_export_comp.io(
    io_type=IoType.INPUT,
    name="sender_input",
    desc="Input table need to be exported.",
    types=[DistDataType.INDIVIDUAL_TABLE],
    col_params=None,
)

data_export_comp.io(
    io_type=IoType.OUTPUT,
    name="receiver_output",
    desc="Output table",
    types=[DistDataType.INDIVIDUAL_TABLE],
    col_params=None,
)

data_delete_comp = Component(
    "delete",
    domain="data_management",
    version="0.0.1",
    desc="data delete.",
)

data_delete_comp.str_attr(
    name="deleter/domain_id",
    desc="deleter domain id",
    is_list=False,
    is_optional=False,
    default_value="",
    allowed_values=None,
)

data_delete_comp.io(
    io_type=IoType.INPUT,
    name="delete_input",
    desc="Input table need to be deleted.",
    types=[DistDataType.INDIVIDUAL_TABLE],
    col_params=None,
)

data_unauth_comp = Component(
    "unauthorize",
    domain="data_management",
    version="0.0.1",
    desc="data unauthorize.",
)

data_unauth_comp.str_attr(
    name="owner/domain_id",
    desc="owner domain id",
    is_list=False,
    is_optional=False,
    default_value="",
    allowed_values=None,
)

data_unauth_comp.io(
    io_type=IoType.INPUT,
    name="unauthorize_input",
    desc="Input table policy need to be unauthorized.",
    types=[DistDataType.INDIVIDUAL_TABLE],
    col_params=None,
)
