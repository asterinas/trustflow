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
    AttestationNestedPolicies,
    AttestationNestedPolicy,
)


class TestJsonNoneFiltering(unittest.TestCase):
    """Test that None values are properly filtered from JSON output."""

    def test_attestation_generation_params_none_filtering(self):
        """Test that None values are filtered out from AttestationGenerationParams."""
        # Create object with some None values
        params = AttestationGenerationParams(
            tee_identity="test_identity",
            report_type="test_type",
            report_hex_nonce=None,  # This should be filtered out
            report_params=None,  # This should be filtered out
        )

        json_str = params.to_json()
        json_obj = json.loads(json_str)

        # Only non-None values should be present
        self.assertIn("tee_identity", json_obj)
        self.assertIn("report_type", json_obj)
        self.assertNotIn("report_hex_nonce", json_obj)
        self.assertNotIn("report_params", json_obj)
        self.assertEqual(json_obj["tee_identity"], "test_identity")
        self.assertEqual(json_obj["report_type"], "test_type")

    def test_attestation_report_params_none_filtering(self):
        """Test that None values are filtered out from AttestationReportParams."""
        # Create object with only one field set
        params = AttestationReportParams(
            str_report_identity="test_identity",
            hex_user_data=None,
            json_nested_reports=None,
            pem_public_key=None,
            hex_spid=None,
        )

        json_str = params.to_json()
        json_obj = json.loads(json_str)

        # Only the set field should be present
        self.assertIn("str_report_identity", json_obj)
        self.assertNotIn("hex_user_data", json_obj)
        self.assertNotIn("json_nested_reports", json_obj)
        self.assertNotIn("pem_public_key", json_obj)
        self.assertNotIn("hex_spid", json_obj)
        self.assertEqual(json_obj["str_report_identity"], "test_identity")

    def test_attestation_attribute_none_filtering(self):
        """Test that None values are filtered out from AttestationAttribute."""
        # Create object with only a few fields set
        attr = AttestationAttribute(
            str_tee_platform="SGX",
            hex_ta_measurement="measurement_123",
            # All other fields are None by default
        )

        json_str = attr.to_json()
        json_obj = json.loads(json_str)

        # Only the set fields should be present
        self.assertIn("str_tee_platform", json_obj)
        self.assertIn("hex_ta_measurement", json_obj)

        # None fields should not be present
        none_fields = [
            "hex_platform_hw_version",
            "hex_platform_sw_version",
            "hex_secure_flags",
            "hex_platform_measurement",
            "hex_boot_measurement",
            "str_tee_name",
            "str_tee_identity",
            "hex_ta_dyn_measurement",
            "hex_signer",
            "hex_prod_id",
            "str_min_isvsvn",
            "bool_debug_disabled",
            "hex_user_data",
            "hex_hash_or_pem_pubkey",
            "hex_nonce",
            "hex_spid",
            "str_verified_time",
        ]

        for field in none_fields:
            self.assertNotIn(field, json_obj)

    def test_attestation_policy_none_filtering(self):
        """Test that None values are filtered out from AttestationPolicy."""
        # Create object with nested None values
        policy = AttestationPolicy(
            pem_public_key="test_key",
            main_attributes=None,  # Should be filtered out
            nested_policies=None,  # Should be filtered out
        )

        json_str = policy.to_json()
        json_obj = json.loads(json_str)

        # Only the set field should be present
        self.assertIn("pem_public_key", json_obj)
        self.assertNotIn("main_attributes", json_obj)
        self.assertNotIn("nested_policies", json_obj)
        self.assertEqual(json_obj["pem_public_key"], "test_key")

    def test_nested_structures_none_filtering(self):
        """Test that None values are filtered from nested structures."""
        # Create nested structure with mixed None values
        sub_attr = AttestationAttribute(
            str_tee_platform="SGX",
            hex_signer="signer_123",
            # Other fields are None
        )

        nested_policy = AttestationNestedPolicy(sub_attributes=[sub_attr])

        nested_policies = AttestationNestedPolicies(
            str_group_name="test_group",
            str_group_id=None,  # Should be filtered out
            policies=[nested_policy],
        )

        policy = AttestationPolicy(
            pem_public_key="test_key",
            main_attributes=None,  # Should be filtered out
            nested_policies=nested_policies,
        )

        json_str = policy.to_json()
        json_obj = json.loads(json_str)

        # Check main structure
        self.assertIn("pem_public_key", json_obj)
        self.assertIn("nested_policies", json_obj)
        self.assertNotIn("main_attributes", json_obj)

        # Check nested structure
        nested_obj = json_obj["nested_policies"]
        self.assertIn("str_group_name", nested_obj)
        self.assertIn("policies", nested_obj)
        self.assertNotIn("str_group_id", nested_obj)  # Should be filtered out

        # Check innermost structure
        policy_obj = nested_obj["policies"][0]
        self.assertIn("sub_attributes", policy_obj)

        sub_attr_obj = policy_obj["sub_attributes"][0]
        self.assertIn("str_tee_platform", sub_attr_obj)
        self.assertIn("hex_signer", sub_attr_obj)
        # None fields should be filtered out
        self.assertNotIn("hex_ta_measurement", sub_attr_obj)
        self.assertNotIn("str_tee_name", sub_attr_obj)

    def test_empty_list_handling(self):
        """Test that empty lists are preserved (not filtered out)."""
        policy = AttestationPolicy(
            pem_public_key="test_key",
            main_attributes=[],  # Empty list should be preserved
            nested_policies=None,
        )

        json_str = policy.to_json()
        json_obj = json.loads(json_str)

        self.assertIn("pem_public_key", json_obj)
        self.assertIn("main_attributes", json_obj)
        self.assertNotIn("nested_policies", json_obj)
        self.assertEqual(json_obj["main_attributes"], [])

    def test_round_trip_with_none_values(self):
        """Test that JSON round-trip conversion works correctly with None filtering."""
        original = AttestationGenerationParams(
            tee_identity="test_identity",
            report_type="test_type",
            report_hex_nonce="abc123",
            report_params=None,  # This will be filtered out
        )

        # Convert to JSON
        json_str = original.to_json()

        # Convert back from JSON
        reconstructed = AttestationGenerationParams.from_json(json_str)

        # Check that the reconstruction is correct
        self.assertEqual(reconstructed.tee_identity, original.tee_identity)
        self.assertEqual(reconstructed.report_type, original.report_type)
        self.assertEqual(reconstructed.report_hex_nonce, original.report_hex_nonce)
        self.assertIsNone(
            reconstructed.report_params
        )  # Should be None since it was filtered out


if __name__ == "__main__":
    unittest.main()
