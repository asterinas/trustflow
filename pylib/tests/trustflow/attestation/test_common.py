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

import unittest
import json
from typing import

from secretflowapis.v2.sdc import ual_pb2
from google.protobuf import json_format

from trustflow.attestation.common import (
    AttestationGenerationParams,
    AttestationReportParams,
    AttestationReport,
    AttestationAttribute,
    AttestationPolicy,
    AttestationNestedPolicies,
    AttestationNestedPolicy,
)


class TestAttestationJsonConversion(unittest.TestCase):
    """Test JSON conversion methods alignment with sdc-apis protobuf models."""

    def _compare_json_structures(self, json1: str, json2: str) -> bool:
        """Compare two JSON strings for exact equality."""
        try:
            obj1 = json.loads(json1)
            obj2 = json.loads(json2)
            return obj1 == obj2
        except json.JSONDecodeError:
            return False

    def test_attestation_generation_params_alignment(self):
        """Test AttestationGenerationParams JSON conversion aligns with protobuf."""
        # Create protobuf model
        proto_params = ual_pb2.UnifiedAttestationGenerationParams()
        proto_params.tee_identity = "test_tee_identity"
        proto_params.report_type = "test_report_type"
        proto_params.report_hex_nonce = "abcdef1234567890"

        # Create nested report params
        proto_report_params = proto_params.report_params
        proto_report_params.str_report_identity = "test_report_identity"
        proto_report_params.hex_user_data = "fedcba0987654321"
        proto_report_params.json_nested_reports = '{"nested": "report"}'
        proto_report_params.pem_public_key = (
            "-----BEGIN PUBLIC KEY-----\ntest_key\n-----END PUBLIC KEY-----"
        )
        proto_report_params.hex_spid = "spid1234567890abcd"

        # Convert to JSON
        proto_json = json_format.MessageToJson(proto_params)

        # Create dataclass model with same data
        dataclass_report_params = AttestationReportParams(
            str_report_identity="test_report_identity",
            hex_user_data="fedcba0987654321",
            json_nested_reports='{"nested": "report"}',
            pem_public_key="-----BEGIN PUBLIC KEY-----\ntest_key\n-----END PUBLIC KEY-----",
            hex_spid="spid1234567890abcd",
        )

        dataclass_params = AttestationGenerationParams(
            tee_identity="test_tee_identity",
            report_type="test_report_type",
            report_hex_nonce="abcdef1234567890",
            report_params=dataclass_report_params,
        )

        dataclass_json = dataclass_params.to_json()

        # Compare JSON structures
        self.assertTrue(
            self._compare_json_structures(proto_json, dataclass_json),
            "AttestationGenerationParams JSON should align with protobuf structure",
        )

        # Test round-trip conversion
        reconstructed = AttestationGenerationParams.from_json(dataclass_json)
        self.assertEqual(reconstructed.tee_identity, dataclass_params.tee_identity)
        self.assertEqual(reconstructed.report_type, dataclass_params.report_type)
        self.assertEqual(
            reconstructed.report_hex_nonce, dataclass_params.report_hex_nonce
        )
        self.assertIsNotNone(reconstructed.report_params)
        self.assertEqual(
            reconstructed.report_params.str_report_identity,
            dataclass_params.report_params.str_report_identity,
        )

    def test_attestation_report_params_alignment(self):
        """Test AttestationReportParams JSON conversion aligns with protobuf."""
        # Create protobuf model
        proto_params = ual_pb2.UnifiedAttestationReportParams()
        proto_params.str_report_identity = "test_report_identity"
        proto_params.hex_user_data = "abcdef1234567890"
        proto_params.json_nested_reports = '{"test": "data"}'
        proto_params.pem_public_key = (
            "-----BEGIN PUBLIC KEY-----\ntest\n-----END PUBLIC KEY-----"
        )
        proto_params.hex_spid = "spid1234567890"

        # Convert to JSON
        proto_json = json_format.MessageToJson(proto_params)

        # Create dataclass model with same data
        dataclass_params = AttestationReportParams(
            str_report_identity="test_report_identity",
            hex_user_data="abcdef1234567890",
            json_nested_reports='{"test": "data"}',
            pem_public_key="-----BEGIN PUBLIC KEY-----\ntest\n-----END PUBLIC KEY-----",
            hex_spid="spid1234567890",
        )

        dataclass_json = dataclass_params.to_json()

        # Compare JSON structures
        self.assertTrue(
            self._compare_json_structures(proto_json, dataclass_json),
            "AttestationReportParams JSON should align with protobuf structure",
        )

        # Test round-trip conversion
        reconstructed = AttestationReportParams.from_json(dataclass_json)
        self.assertEqual(
            reconstructed.str_report_identity, dataclass_params.str_report_identity
        )
        self.assertEqual(reconstructed.hex_user_data, dataclass_params.hex_user_data)
        self.assertEqual(
            reconstructed.json_nested_reports, dataclass_params.json_nested_reports
        )
        self.assertEqual(reconstructed.pem_public_key, dataclass_params.pem_public_key)
        self.assertEqual(reconstructed.hex_spid, dataclass_params.hex_spid)

    def test_attestation_report_alignment(self):
        """Test AttestationReport JSON conversion aligns with protobuf."""
        # Create protobuf model
        proto_report = ual_pb2.UnifiedAttestationReport()
        proto_report.str_report_version = "1.0"
        proto_report.str_report_type = "SGX_QUOTE"
        proto_report.str_tee_platform = "SGX_DCAP"
        proto_report.json_report = '{"quote": "test_quote_data"}'
        proto_report.json_nested_reports = '{"nested": "reports"}'

        # Convert to JSON
        proto_json = json_format.MessageToJson(proto_report)

        # Create dataclass model with same data
        dataclass_report = AttestationReport(
            str_report_version="1.0",
            str_report_type="SGX_QUOTE",
            str_tee_platform="SGX_DCAP",
            json_report='{"quote": "test_quote_data"}',
            json_nested_reports='{"nested": "reports"}',
        )

        dataclass_json = dataclass_report.to_json()

        # Compare JSON structures
        self.assertTrue(
            self._compare_json_structures(proto_json, dataclass_json),
            "AttestationReport JSON should align with protobuf structure",
        )

        # Test round-trip conversion
        reconstructed = AttestationReport.from_json(dataclass_json)
        self.assertEqual(
            reconstructed.str_report_version, dataclass_report.str_report_version
        )
        self.assertEqual(
            reconstructed.str_report_type, dataclass_report.str_report_type
        )
        self.assertEqual(
            reconstructed.str_tee_platform, dataclass_report.str_tee_platform
        )
        self.assertEqual(reconstructed.json_report, dataclass_report.json_report)
        self.assertEqual(
            reconstructed.json_nested_reports, dataclass_report.json_nested_reports
        )

    def test_attestation_attribute_alignment(self):
        """Test AttestationAttribute JSON conversion aligns with protobuf."""
        # Create protobuf model
        proto_attr = ual_pb2.UnifiedAttestationAttributes()
        proto_attr.str_tee_platform = "SGX"
        proto_attr.hex_platform_hw_version = "hw_version_123"
        proto_attr.hex_platform_sw_version = "sw_version_456"
        proto_attr.hex_secure_flags = "secure_flags_789"
        proto_attr.hex_platform_measurement = "platform_measurement_abc"
        proto_attr.hex_boot_measurement = "boot_measurement_def"
        proto_attr.str_tee_name = "Intel_SGX"
        proto_attr.str_tee_identity = "tee_identity_123"
        proto_attr.hex_ta_measurement = "ta_measurement_xyz"
        proto_attr.hex_ta_dyn_measurement = "ta_dyn_measurement_abc"
        proto_attr.hex_signer = "signer_123456"
        proto_attr.hex_prod_id = "prod_id_789"
        proto_attr.str_min_isvsvn = "isvsvn_001"
        proto_attr.bool_debug_disabled = "true"
        proto_attr.hex_user_data = "user_data_123456"
        proto_attr.hex_hash_or_pem_pubkey = "hash_or_pubkey_789"
        proto_attr.hex_nonce = "nonce_abcdef123456"
        proto_attr.hex_spid = "spid_1234567890abcd"
        proto_attr.str_verified_time = "2023-12-01T10:30:00Z"

        # Convert to JSON
        proto_json = json_format.MessageToJson(proto_attr)

        # Create dataclass model with same data
        dataclass_attr = AttestationAttribute(
            str_tee_platform="SGX",
            hex_platform_hw_version="hw_version_123",
            hex_platform_sw_version="sw_version_456",
            hex_secure_flags="secure_flags_789",
            hex_platform_measurement="platform_measurement_abc",
            hex_boot_measurement="boot_measurement_def",
            str_tee_name="Intel_SGX",
            str_tee_identity="tee_identity_123",
            hex_ta_measurement="ta_measurement_xyz",
            hex_ta_dyn_measurement="ta_dyn_measurement_abc",
            hex_signer="signer_123456",
            hex_prod_id="prod_id_789",
            str_min_isvsvn="isvsvn_001",
            bool_debug_disabled="true",
            hex_user_data="user_data_123456",
            hex_hash_or_pem_pubkey="hash_or_pubkey_789",
            hex_nonce="nonce_abcdef123456",
            hex_spid="spid_1234567890abcd",
            str_verified_time="2023-12-01T10:30:00Z",
        )

        dataclass_json = dataclass_attr.to_json()

        # Compare JSON structures
        self.assertTrue(
            self._compare_json_structures(proto_json, dataclass_json),
            "AttestationAttribute JSON should align with protobuf structure",
        )

        # Test round-trip conversion
        reconstructed = AttestationAttribute.from_json(dataclass_json)
        self.assertEqual(
            reconstructed.str_tee_platform, dataclass_attr.str_tee_platform
        )
        self.assertEqual(
            reconstructed.hex_platform_hw_version,
            dataclass_attr.hex_platform_hw_version,
        )
        self.assertEqual(reconstructed.hex_nonce, dataclass_attr.hex_nonce)

    def test_attestation_policy_alignment(self):
        """Test AttestationPolicy JSON conversion aligns with protobuf."""
        # Create protobuf model
        proto_policy = ual_pb2.UnifiedAttestationPolicy()
        proto_policy.pem_public_key = (
            "-----BEGIN PUBLIC KEY-----\ntest_key\n-----END PUBLIC KEY-----"
        )

        # Add main attributes
        proto_attr1 = proto_policy.main_attributes.add()
        proto_attr1.str_tee_platform = "SGX"
        proto_attr1.hex_ta_measurement = "measurement_123"

        proto_attr2 = proto_policy.main_attributes.add()
        proto_attr2.str_tee_platform = "TDX"
        proto_attr2.hex_ta_measurement = "measurement_456"

        # Add nested policies
        proto_nested = proto_policy.nested_policies
        proto_nested.str_group_name = "test_group"
        proto_nested.str_group_id = "group_123"

        nested_policy1 = proto_nested.policies.add()
        sub_attr1 = nested_policy1.sub_attributes.add()
        sub_attr1.str_tee_platform = "SGX"
        sub_attr1.hex_signer = "signer_123"

        # Convert to JSON
        proto_json = json_format.MessageToJson(proto_policy)

        # Create dataclass model with same data
        dataclass_attr1 = AttestationAttribute(
            str_tee_platform="SGX", hex_ta_measurement="measurement_123"
        )

        dataclass_attr2 = AttestationAttribute(
            str_tee_platform="TDX", hex_ta_measurement="measurement_456"
        )

        dataclass_sub_attr = AttestationAttribute(
            str_tee_platform="SGX", hex_signer="signer_123"
        )

        dataclass_nested_policy = AttestationNestedPolicy(
            sub_attributes=[dataclass_sub_attr]
        )

        dataclass_nested_policies = AttestationNestedPolicies(
            str_group_name="test_group",
            str_group_id="group_123",
            policies=[dataclass_nested_policy],
        )

        dataclass_policy = AttestationPolicy(
            pem_public_key="-----BEGIN PUBLIC KEY-----\ntest_key\n-----END PUBLIC KEY-----",
            main_attributes=[dataclass_attr1, dataclass_attr2],
            nested_policies=dataclass_nested_policies,
        )

        dataclass_json = dataclass_policy.to_json()

        # Compare JSON structures
        self.assertTrue(
            self._compare_json_structures(proto_json, dataclass_json),
            "AttestationPolicy JSON should align with protobuf structure",
        )

        # Test round-trip conversion
        reconstructed = AttestationPolicy.from_json(dataclass_json)
        self.assertEqual(reconstructed.pem_public_key, dataclass_policy.pem_public_key)
        self.assertEqual(len(reconstructed.main_attributes), 2)
        self.assertEqual(reconstructed.main_attributes[0].str_tee_platform, "SGX")
        self.assertEqual(reconstructed.nested_policies.str_group_name, "test_group")

    def test_empty_values_handling(self):
        """Test handling of empty/None values aligns with protobuf behavior."""
        # Test with mostly empty values
        proto_params = ual_pb2.UnifiedAttestationGenerationParams()
        proto_params.tee_identity = "test_identity"
        # Leave other fields empty/default

        proto_json = json_format.MessageToJson(proto_params)

        # Create dataclass with None values
        dataclass_params = AttestationGenerationParams(
            tee_identity="test_identity",
            report_type=None,
            report_hex_nonce=None,
            report_params=None,
        )

        dataclass_json = dataclass_params.to_json()

        # Both should handle empty values consistently
        proto_obj = json.loads(proto_json)
        dataclass_obj = json.loads(dataclass_json)

        # Check that empty/None values are handled consistently
        self.assertEqual(
            proto_obj.get("tee_identity"), dataclass_obj.get("tee_identity")
        )

    def test_nested_structure_alignment(self):
        """Test complex nested structures align between dataclass and protobuf."""
        # Create a complex nested structure
        proto_policy = ual_pb2.UnifiedAttestationPolicy()

        # Main attributes
        for i in range(3):
            attr = proto_policy.main_attributes.add()
            attr.str_tee_platform = f"Platform_{i}"
            attr.hex_ta_measurement = f"measurement_{i}"

        # Nested policies
        nested = proto_policy.nested_policies
        nested.str_group_name = "ComplexGroup"
        nested.str_group_id = "complex_123"

        for i in range(2):
            policy = nested.policies.add()
            for j in range(2):
                sub_attr = policy.sub_attributes.add()
                sub_attr.str_tee_platform = f"NestedPlatform_{i}_{j}"
                sub_attr.hex_signer = f"signer_{i}_{j}"

        proto_json = json_format.MessageToJson(proto_policy)

        # Create equivalent dataclass structure
        main_attrs = []
        for i in range(3):
            main_attrs.append(
                AttestationAttribute(
                    str_tee_platform=f"Platform_{i}",
                    hex_ta_measurement=f"measurement_{i}",
                )
            )

        nested_policies = []
        for i in range(2):
            sub_attrs = []
            for j in range(2):
                sub_attrs.append(
                    AttestationAttribute(
                        str_tee_platform=f"NestedPlatform_{i}_{j}",
                        hex_signer=f"signer_{i}_{j}",
                    )
                )
            nested_policies.append(AttestationNestedPolicy(sub_attributes=sub_attrs))

        dataclass_nested = AttestationNestedPolicies(
            str_group_name="ComplexGroup",
            str_group_id="complex_123",
            policies=nested_policies,
        )

        dataclass_policy = AttestationPolicy(
            main_attributes=main_attrs, nested_policies=dataclass_nested
        )

        dataclass_json = dataclass_policy.to_json()

        # Compare the complex structures
        self.assertTrue(
            self._compare_json_structures(proto_json, dataclass_json),
            "Complex nested structures should align between dataclass and protobuf",
        )


if __name__ == "__main__":
    unittest.main()
