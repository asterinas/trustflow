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
import sys

sys.path.append("/home/ruidong.qrd/code/github/trustflow/pylib")

from trustflow.attestation.common import (
    AttestationGenerationParams,
    AttestationReportParams,
    AttestationReport,
    AttestationAttribute,
    AttestationPolicy,
    AttestationNestedPolicy,
)


class TestAttestationBasicFunctionality(unittest.TestCase):
    """Test basic JSON functionality without protobuf alignment."""

    def test_basic_json_operations(self):
        """Test basic JSON serialization and deserialization."""
        # Test AttestationGenerationParams
        original_params = AttestationGenerationParams(
            tee_identity="test_identity",
            report_type="test_type",
            report_hex_nonce="test_nonce",
        )

        json_str = original_params.to_json()
        reconstructed_params = AttestationGenerationParams.from_json(json_str)

        self.assertEqual(
            reconstructed_params.tee_identity, original_params.tee_identity
        )
        self.assertEqual(reconstructed_params.report_type, original_params.report_type)
        self.assertEqual(
            reconstructed_params.report_hex_nonce, original_params.report_hex_nonce
        )
        self.assertIsNone(reconstructed_params.report_params)

    def test_none_value_filtering_basic(self):
        """Test that None values are filtered from JSON output."""
        # Test with None values
        params = AttestationGenerationParams(
            tee_identity="test_identity",
            report_type="test_type",
            report_hex_nonce=None,
            report_params=None,
        )

        json_str = params.to_json()
        json_obj = json.loads(json_str)

        # Only non-None values should be present
        self.assertIn("tee_identity", json_obj)
        self.assertIn("report_type", json_obj)
        self.assertNotIn("report_hex_nonce", json_obj)
        self.assertNotIn("report_params", json_obj)

    def test_nested_structures_basic(self):
        """Test basic nested structure functionality."""
        # Create nested structure
        report_params = AttestationReportParams(
            str_report_identity="test_identity", hex_user_data="test_data"
        )

        generation_params = AttestationGenerationParams(
            tee_identity="test_tee",
            report_type="test_type",
            report_params=report_params,
        )

        # Test JSON serialization
        json_str = generation_params.to_json()
        json_obj = json.loads(json_str)

        self.assertIn("tee_identity", json_obj)
        self.assertIn("report_type", json_obj)
        self.assertIn("report_params", json_obj)
        self.assertEqual(
            json_obj["report_params"]["str_report_identity"], "test_identity"
        )
        self.assertEqual(json_obj["report_params"]["hex_user_data"], "test_data")

        # Test deserialization
        reconstructed = AttestationGenerationParams.from_json(json_str)
        self.assertEqual(reconstructed.tee_identity, "test_tee")
        self.assertIsNotNone(reconstructed.report_params)
        self.assertEqual(
            reconstructed.report_params.str_report_identity, "test_identity"
        )

    def test_complex_attributes(self):
        """Test complex AttestationAttribute functionality."""
        attr = AttestationAttribute(
            str_tee_platform="SGX",
            hex_ta_measurement="measurement_123",
            hex_signer="signer_456",
        )

        json_str = attr.to_json()
        json_obj = json.loads(json_str)

        # Check that only set fields are present
        self.assertIn("str_tee_platform", json_obj)
        self.assertIn("hex_ta_measurement", json_obj)
        self.assertIn("hex_signer", json_obj)

        # Check that None fields are not present
        none_fields = ["str_tee_name", "hex_platform_hw_version", "hex_nonce"]
        for field in none_fields:
            self.assertNotIn(field, json_obj)

        # Test round trip
        reconstructed = AttestationAttribute.from_json(json_str)
        self.assertEqual(reconstructed.str_tee_platform, "SGX")
        self.assertEqual(reconstructed.hex_ta_measurement, "measurement_123")
        self.assertEqual(reconstructed.hex_signer, "signer_456")

    def test_policy_with_attributes(self):
        """Test AttestationPolicy with attributes."""
        # Create attributes
        attr1 = AttestationAttribute(
            str_tee_platform="SGX", hex_ta_measurement="measurement1"
        )
        attr2 = AttestationAttribute(
            str_tee_platform="TDX", hex_ta_measurement="measurement2"
        )

        # Create policy
        policy = AttestationPolicy(
            pem_public_key="test_key", main_attributes=[attr1, attr2]
        )

        # Test JSON serialization
        json_str = policy.to_json()
        json_obj = json.loads(json_str)

        self.assertIn("pem_public_key", json_obj)
        self.assertIn("main_attributes", json_obj)
        self.assertNotIn("nested_policies", json_obj)  # Should be filtered out

        self.assertEqual(len(json_obj["main_attributes"]), 2)
        self.assertEqual(json_obj["main_attributes"][0]["str_tee_platform"], "SGX")
        self.assertEqual(json_obj["main_attributes"][1]["str_tee_platform"], "TDX")

    def test_empty_json_handling(self):
        """Test handling of minimal JSON input."""
        # Test with empty JSON for AttestationReportParams
        empty_json = "{}"
        result = AttestationReportParams.from_json(empty_json)

        # All fields should be None
        self.assertIsNone(result.str_report_identity)
        self.assertIsNone(result.hex_user_data)
        self.assertIsNone(result.json_nested_reports)
        self.assertIsNone(result.pem_public_key)
        self.assertIsNone(result.hex_spid)

    def test_json_structure_consistency(self):
        """Test that JSON structure is consistent across multiple serializations."""
        original = AttestationGenerationParams(
            tee_identity="test_identity",
            report_type="test_type",
            report_hex_nonce="test_nonce",
        )

        # Serialize multiple times
        json1 = original.to_json()
        json2 = original.to_json()

        # Should be identical
        self.assertEqual(json1, json2)

        # Deserialize and serialize again
        reconstructed = AttestationGenerationParams.from_json(json1)
        json3 = reconstructed.to_json()

        # Should still be identical
        self.assertEqual(json1, json3)

    def test_all_dataclass_types(self):
        """Test that all dataclass types can be serialized and deserialized."""
        # Test AttestationReport
        report = AttestationReport(
            str_report_version="1.0",
            str_report_type="SGX_QUOTE",
            str_tee_platform="SGX_DCAP",
            json_report='{"quote": "data"}',
        )

        report_json = report.to_json()
        reconstructed_report = AttestationReport.from_json(report_json)
        self.assertEqual(reconstructed_report.str_report_version, "1.0")

        # Test AttestationNestedPolicy
        nested_policy = AttestationNestedPolicy(
            sub_attributes=[AttestationAttribute(str_tee_platform="SGX")]
        )

        nested_json = nested_policy.to_json()
        reconstructed_nested = AttestationNestedPolicy.from_json(nested_json)
        self.assertEqual(len(reconstructed_nested.sub_attributes), 1)
        self.assertEqual(reconstructed_nested.sub_attributes[0].str_tee_platform, "SGX")


if __name__ == "__main__":
    unittest.main()
