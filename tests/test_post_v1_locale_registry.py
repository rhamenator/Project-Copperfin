"""Planning registry failures that could silently imply unsupported packs."""

import copy
import json
import re
import sys
import tempfile
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "scripts"))
from validate_post_v1_locale_registry import REGISTRY, load_registry, validate  # noqa: E402

EVIDENCE_SCHEMA = ROOT / "docs" / "contracts" / "locale-entry-evidence-v1.schema.json"


def schema_accepts(value, schema):
    type_checks = {
        "object": lambda item: isinstance(item, dict),
        "array": lambda item: isinstance(item, list),
        "string": lambda item: isinstance(item, str),
    }
    expected_type = schema.get("type")
    if expected_type in type_checks and not type_checks[expected_type](value):
        return False
    if "const" in schema and value != schema["const"]:
        return False
    if "enum" in schema and value not in schema["enum"]:
        return False
    if isinstance(value, str):
        if len(value) < schema.get("minLength", 0):
            return False
        if "pattern" in schema and re.fullmatch(schema["pattern"], value) is None:
            return False
    if isinstance(value, dict):
        if any(key not in value for key in schema.get("required", [])):
            return False
        properties = schema.get("properties", {})
        if schema.get("additionalProperties") is False and any(key not in properties for key in value):
            return False
        if any(not schema_accepts(value[key], child)
               for key, child in properties.items() if key in value):
            return False
    if isinstance(value, list):
        if schema.get("uniqueItems") and len({json.dumps(item, sort_keys=True) for item in value}) != len(value):
            return False
        if "items" in schema and any(not schema_accepts(item, schema["items"]) for item in value):
            return False
    for condition in schema.get("allOf", []):
        if "if" in condition and schema_accepts(value, condition["if"]):
            if not schema_accepts(value, condition.get("then", {})):
                return False
        elif "if" not in condition and not schema_accepts(value, condition):
            return False
    return True


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

    def test_source_and_pseudo_rows_are_pinned(self):
        data = copy.deepcopy(self.registry)
        next(x for x in data["locales"] if x["tag"] == "en-US")["releaseStatus"] = "production"
        next(x for x in data["locales"] if x["tag"] == "qps-ploc")["phase"] = "western"
        errors = validate(data)
        self.assertTrue(any("en-US source baseline" in e for e in errors))
        self.assertTrue(any("qps-ploc test-only" in e for e in errors))

    def test_catalog_status_must_match_installed_inventory(self):
        data = copy.deepcopy(self.registry)
        next(x for x in data["locales"] if x["tag"] == "ja-JP")["catalogStatus"] = "present-unqualified"
        self.assertTrue(any("installed inventory" in e for e in validate(data)))

    def test_direction_and_script_subtag_must_match(self):
        data = copy.deepcopy(self.registry)
        next(x for x in data["locales"] if x["tag"] == "ar")["direction"] = "ltr"
        next(x for x in data["locales"] if x["tag"] == "zh-Hans-CN")["script"] = "Hant"
        errors = validate(data)
        self.assertTrue(any("direction does not match" in e for e in errors))
        self.assertTrue(any("script does not match" in e for e in errors))

    def test_non_latin_locale_cannot_inherit_from_en_us(self):
        data = copy.deepcopy(self.registry)
        next(x for x in data["locales"] if x["tag"] == "ja-JP")["preferredParent"] = "en-US"
        self.assertTrue(any("cross-script" in e for e in validate(data)))

    def test_required_arabic_variant_and_no_es_es_inheritance(self):
        data = copy.deepcopy(self.registry)
        data["locales"] = [item for item in data["locales"] if item["tag"] != "ar-EG"]
        next(x for x in data["locales"] if x["tag"] == "es-ES")["preferredParent"] = "es-419"
        errors = validate(data)
        self.assertTrue(any("missing issue #7075 tag ar-EG" in e for e in errors))
        self.assertTrue(any("disallowed automatic regional inheritance es-ES" in e for e in errors))

    def test_malformed_entries_and_unknown_fields_fail_closed(self):
        data = copy.deepcopy(self.registry)
        data["locales"].append("not-an-object")
        data["locales"][0]["installed"] = True
        data["locales"][1]["prerequisites"] = [""]
        errors = validate(data)
        self.assertTrue(any("must be an object" in e for e in errors))
        self.assertTrue(any("unknown locale fields" in e for e in errors))
        self.assertTrue(any("missing prerequisites" in e for e in errors))

    def test_duplicate_json_keys_are_rejected(self):
        with tempfile.TemporaryDirectory() as temporary:
            path = Path(temporary) / "registry.json"
            path.write_text('{"schemaVersion":1,"schemaVersion":1}\n', encoding="utf-8")
            with self.assertRaisesRegex(ValueError, "duplicate JSON key"):
                load_registry(path)

    def test_both_human_review_statuses_require_reviewer_evidence(self):
        schema = json.loads(EVIDENCE_SCHEMA.read_text(encoding="utf-8"))
        conditions = schema["properties"]["entries"]["items"]["allOf"]
        human_statuses = {"human-linguistic", "human-cultural-and-terminology"}
        matching = [condition for condition in conditions
                    if set(condition.get("if", {}).get("properties", {})
                           .get("reviewStatus", {}).get("enum", [])) == human_statuses]
        self.assertEqual(1, len(matching))
        self.assertIn("reviewerEvidence", matching[0]["then"]["required"])

    def test_evidence_schema_binds_human_review_to_exact_translation(self):
        schema = json.loads(EVIDENCE_SCHEMA.read_text(encoding="utf-8"))
        base = {
            "locale": "fr-FR",
            "sourceRevision": "1" * 40,
            "catalogRevision": "2" * 40,
            "entries": [{
                "key": "Example",
                "sourceSha256": "3" * 64,
                "translationStatus": "missing",
                "reviewStatus": "none",
            }],
        }
        self.assertTrue(schema_accepts(base, schema))
        human = copy.deepcopy(base)
        human["entries"][0]["reviewStatus"] = "human-linguistic"
        self.assertFalse(schema_accepts(human, schema))
        human["entries"][0].update({
            "translationStatus": "draft-human",
            "translationSha256": "4" * 64,
            "translatorEvidence": "translation://example",
            "reviewerEvidence": {
                "reviewerId": "reviewer-1",
                "qualification": "Qualified French reviewer",
                "reviewedAt": "2026-11-13",
                "evidenceLink": "review://example",
            },
            "unresolvedRisk": "None identified in the reviewed entry",
        })
        self.assertTrue(schema_accepts(human, schema))


if __name__ == "__main__":
    unittest.main()
