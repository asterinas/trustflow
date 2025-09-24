# Copyright 2025 Ant Group Co., Ltd.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

from .. import (
    AttestationGenerationParams,
    AttestationReport,
)
from . import generator  # type: ignore


def generate_report(params: AttestationGenerationParams) -> AttestationReport:
    attestation_generator = generator.create_attestation_generator()

    params_json = params.to_json()
    report_json = attestation_generator.generate_report_json(params_json)
    report = AttestationReport.from_json(report_json)
    return report
