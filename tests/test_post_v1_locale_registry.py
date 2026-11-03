"""Planning registry failures that could silently imply unsupported packs."""

import copy
import json
import sys
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "scripts"))
from validate_post_v1_locale_registry import REGISTRY, validate  # noqa: E402


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


if __name__ == "__main__":
    unittest.main()
