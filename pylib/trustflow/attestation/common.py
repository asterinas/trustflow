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

import json
from dataclasses import dataclass, fields
from typing import Optional, List, Any


def _filter_none_values(obj: Any) -> Any:
    """Recursively filter out None values from dataclass objects to match protobuf behavior."""
    if hasattr(obj, "__dataclass_fields__"):
        # This is a dataclass instance
        result = {}
        for field in fields(obj):
            value = getattr(obj, field.name)
            if value is not None:
                filtered_value = _filter_none_values(value)
                # Only include the field if it's not None after filtering
                if filtered_value is not None:
                    result[field.name] = filtered_value
        return result
    elif isinstance(obj, list):
        # Handle lists by filtering each item
        filtered_list = []
        for item in obj:
            filtered_item = _filter_none_values(item)
            if filtered_item is not None:
                filtered_list.append(filtered_item)
        return filtered_list
    else:
        # Primitive values
        return obj


@dataclass
class AttestationGenerationParams:
    """Parameters for generating attestation reports.

    Maps to UnifiedAttestationGenerationParams in ual.proto
    """

    # For which TEE instance to generate the unified attestation report (proto field 1)
    tee_identity: str
    # Which type of unified attestation report to be generated (proto field 2)
    report_type: str
    # Provide freshness if necessary (proto field 3)
    report_hex_nonce: Optional[str] = None
    # Special parameters for different TEE platforms (proto field 4)
    report_params: Optional["AttestationReportParams"] = None

    def to_json(self) -> str:
        """Convert to JSON string."""
        filtered_data = _filter_none_values(self)
        return json.dumps(filtered_data, indent=2)

    @classmethod
    def from_json(cls, json_str: str) -> "AttestationGenerationParams":
        """Create from JSON string."""
        data = json.loads(json_str)
        if "report_params" in data and data["report_params"] is not None:
            data["report_params"] = AttestationReportParams(**data["report_params"])
        return cls(**data)


@dataclass
class AttestationReportParams:
    """Special parameters for different TEE platforms.

    Maps to UnifiedAttestationReportParams in ual.proto
    """

    # The identity string for the report instance which is cached inside TEE (proto field 1)
    str_report_identity: Optional[str] = None
    # The user data in some TEE platforms, Max to 64 Bytes of HEX string (proto field 2)
    hex_user_data: Optional[str] = None
    # The JSON serialized string of UnifiedAttestationNestedReports (proto field 3)
    json_nested_reports: Optional[str] = None
    # User specified public key instead of UAK to be put into report_data (proto field 4)
    pem_public_key: Optional[str] = None
    # Service Provider ID for SGX1 only (proto field 10)
    hex_spid: Optional[str] = None

    def to_json(self) -> str:
        """Convert to JSON string."""
        filtered_data = _filter_none_values(self)
        return json.dumps(filtered_data, indent=2)

    @classmethod
    def from_json(cls, json_str: str) -> "AttestationReportParams":
        """Create from JSON string."""
        data = json.loads(json_str)
        return cls(**data)


@dataclass
class AttestationReport:
    """Unified attestation report.

    Maps to UnifiedAttestationReport in ual.proto
    """

    # For compatibility and update later, the current version is "1.0" (proto field 1)
    str_report_version: str
    # Valid type string: "BackgroundCheck"|"Passport"|"Uas" (proto field 2)
    str_report_type: str
    # The TEE platform name (proto field 3)
    str_tee_platform: str
    # Different JSON serialized string for each TEE platform (proto field 4)
    json_report: str
    # The JSON serialized string of UnifiedAttestationNestedReports (proto field 9)
    json_nested_reports: Optional[str] = None

    def to_json(self) -> str:
        """Convert to JSON string."""
        filtered_data = _filter_none_values(self)
        return json.dumps(filtered_data, indent=2)

    @classmethod
    def from_json(cls, json_str: str) -> "AttestationReport":
        """Create from JSON string."""
        data = json.loads(json_str)
        return cls(**data)


@dataclass
class AttestationAttribute:
    """Collects all attributes information of trusted application.

    Maps to UnifiedAttestationAttributes in ual.proto
    """

    # The TEE platform, in case some verifier needs to verify this (proto field 1)
    str_tee_platform: Optional[str] = None
    # The TEE platform hardware-related identity or version (proto field 2)
    hex_platform_hw_version: Optional[str] = None
    # The TEE platform TCB software-related version (proto field 3)
    hex_platform_sw_version: Optional[str] = None
    # The TEE platform security-related attributes or flags (proto field 4)
    hex_secure_flags: Optional[str] = None
    # The measurement of TEE implement internal stuff (proto field 5)
    hex_platform_measurement: Optional[str] = None
    # The measurement of TEE instance boot time stuff (proto field 6)
    hex_boot_measurement: Optional[str] = None
    # The name of this tee instance (proto field 8)
    str_tee_name: Optional[str] = None
    # The TEE instance or trust application identity when generating the report (proto field 9)
    str_tee_identity: Optional[str] = None
    # The static measurement of trust application when loading the code (proto field 10)
    hex_ta_measurement: Optional[str] = None
    # The dynamical measurement of trust application code (proto field 11)
    hex_ta_dyn_measurement: Optional[str] = None
    # The measurement or other identity of the trust application signer (proto field 12)
    hex_signer: Optional[str] = None
    # The product ID of the TEE instance or trust application (proto field 13)
    hex_prod_id: Optional[str] = None
    # The minimal ISV SVN of the TEE instance or trust application (proto field 14)
    str_min_isvsvn: Optional[str] = None
    # The bool string "0" for debuggable, "1" for not debuggable (proto field 15)
    bool_debug_disabled: Optional[str] = None
    # The user data for generating the attestation report (proto field 20)
    hex_user_data: Optional[str] = None
    # hex string hash or original pem public key (proto field 21)
    hex_hash_or_pem_pubkey: Optional[str] = None
    # The independent freshness value besides what is in user data (proto field 22)
    hex_nonce: Optional[str] = None
    # The service provider id, e.g. use in sgx1, 64 bytes hex string (proto field 30)
    hex_spid: Optional[str] = None
    # The report verified time set by verifier if there is trust time (proto field 40)
    str_verified_time: Optional[str] = None

    def to_json(self) -> str:
        """Convert to JSON string."""
        filtered_data = _filter_none_values(self)
        return json.dumps(filtered_data, indent=2)

    @classmethod
    def from_json(cls, json_str: str) -> "AttestationAttribute":
        """Create from JSON string."""
        data = json.loads(json_str)
        return cls(**data)


@dataclass
class AttestationPolicy:
    """Policy used when verifying the attestation report.

    Maps to UnifiedAttestationPolicy in ual.proto
    """

    # Assume one public key is bound to one report, specify it here (proto field 1)
    pem_public_key: Optional[str] = None
    # For the main attester (proto field 2)
    main_attributes: Optional[List[AttestationAttribute]] = None
    # For submodule attesters (proto field 3)
    nested_policies: Optional["AttestationNestedPolicies"] = None

    def to_json(self) -> str:
        """Convert to JSON string."""
        filtered_data = _filter_none_values(self)
        return json.dumps(filtered_data, indent=2)

    @classmethod
    def from_json(cls, json_str: str) -> "AttestationPolicy":
        """Create from JSON string."""
        data = json.loads(json_str)
        if "main_attributes" in data and data["main_attributes"] is not None:
            data["main_attributes"] = [
                AttestationAttribute(**attr) for attr in data["main_attributes"]
            ]
        if "nested_policies" in data and data["nested_policies"] is not None:
            data["nested_policies"] = AttestationNestedPolicies.from_json(
                json.dumps(data["nested_policies"])
            )
        return cls(**data)


@dataclass
class AttestationNestedPolicies:
    """Match rules for nested reports verification.

    Maps to UnifiedAttestationNestedPolicies in ual.proto
    """

    # The group name is used for group attestation (proto field 1)
    str_group_name: Optional[str] = None
    # The group id is used for group attestation (proto field 2)
    str_group_id: Optional[str] = None
    # List of nested policies (proto field 3)
    policies: Optional[List["AttestationNestedPolicy"]] = None

    def to_json(self) -> str:
        """Convert to JSON string."""
        filtered_data = _filter_none_values(self)
        return json.dumps(filtered_data, indent=2)

    @classmethod
    def from_json(cls, json_str: str) -> "AttestationNestedPolicies":
        """Create from JSON string."""
        data = json.loads(json_str)
        if "policies" in data and data["policies"] is not None:
            data["policies"] = [
                AttestationNestedPolicy.from_json(json.dumps(policy))
                for policy in data["policies"]
            ]
        return cls(**data)


@dataclass
class AttestationNestedPolicy:
    """Match rules for nested report verification.

    Maps to UnifiedAttestationNestedPolicy in ual.proto
    """

    # List of sub-attributes for nested report verification (proto field 3)
    sub_attributes: Optional[List[AttestationAttribute]] = None

    def to_json(self) -> str:
        """Convert to JSON string."""
        filtered_data = _filter_none_values(self)
        return json.dumps(filtered_data, indent=2)

    @classmethod
    def from_json(cls, json_str: str) -> "AttestationNestedPolicy":
        """Create from JSON string."""
        data = json.loads(json_str)
        if "sub_attributes" in data and data["sub_attributes"] is not None:
            data["sub_attributes"] = [
                AttestationAttribute(**attr) for attr in data["sub_attributes"]
            ]
        return cls(**data)
