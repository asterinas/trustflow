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

from pylib.trustflow.attestation.common import AttestationPolicy, AttestationReport
from . import verifier  # type: ignore


def report_verify(
    report: AttestationReport, policy: AttestationPolicy
) -> verifier.Status:
    report_json = report.to_json()
    policy_json = policy.to_json()
    return verifier.attestation_report_verify(report_json, policy_json)
