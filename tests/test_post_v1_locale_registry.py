"""Planning registry failures that could silently imply unsupported packs."""

import copy
import json
import sys
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "scripts"))
from validate_post_v1_locale_registry import REGISTRY, validate  # noqa: E402

EVIDENCE_SCHEMA = ROOT / "docs" / "contracts" / "locale-entry-evidence-v1.schema.json"


class RegistryTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.registry = json.loads(REGISTRY.read_text(encoding="utf-8"))

    def test_committed_registry(self):
        self.assertEqual([], validate(self.registry))

    def test_cross_script_fallback_rejected(self):
        data = copy.deepcopy(self.registry)
        next(x for x in data["locales"] if x["tag"] == "zh-Hant-HK")["preferredParent"] = "zh-Hans-CN"
        self.assertTrue(any("cross-script" in e for e in validate(data)))

    def test_fallback_cycle_rejected(self):
        data = copy.deepcopy(self.registry)
        next(x for x in data["locales"] if x["tag"] == "fr-FR")["preferredParent"] = "fr-CA"
        self.assertTrue(any("cycle" in e for e in validate(data)))

    def test_candidate_cannot_claim_qualification(self):
        data = copy.deepcopy(self.registry)
        next(x for x in data["locales"] if x["tag"] == "ar")["releaseStatus"] = "production"
        self.assertTrue(any("unearned" in e for e in validate(data)))

    def test_both_human_review_statuses_require_reviewer_evidence(self):
        schema = json.loads(EVIDENCE_SCHEMA.read_text(encoding="utf-8"))
        conditions = schema["properties"]["entries"]["items"]["allOf"]
        human_statuses = {"human-linguistic", "human-cultural-and-terminology"}
        matching = [condition for condition in conditions
                    if set(condition.get("if", {}).get("properties", {})
                           .get("reviewStatus", {}).get("enum", [])) == human_statuses]
        self.assertEqual(1, len(matching))
        self.assertIn("reviewerEvidence", matching[0]["then"]["required"])


if __name__ == "__main__":
    unittest.main()
