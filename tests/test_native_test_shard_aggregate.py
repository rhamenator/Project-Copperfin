"""Synthetic artifact mutation checks; not actual hosted native acceptance."""
import copy
import importlib.util
import json
from pathlib import Path
import tempfile
import unittest
import xml.etree.ElementTree as ET

SOURCE = Path(__file__).resolve().parents[1] / "scripts/audit-native-test-shards.py"
SPEC = importlib.util.spec_from_file_location("shard_audit", SOURCE)
MODULE = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(MODULE)


class AggregateTests(unittest.TestCase):
    def setUp(self):
        temp_base = SOURCE.parents[1] / "build/native-shard-unit"
        temp_base.mkdir(parents=True, exist_ok=True)
        self.temp = tempfile.TemporaryDirectory(prefix="aggregate-", dir=temp_base)
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        names = [f"test_{index}" for index in range(10)]
        full = [{"name": name, "command": ["cmake", "-E", "true"],
                 "properties": [{"name": "RUN_SERIAL", "value": True}]} for name in names]
        for index in range(3):
            folder = self.root / str(index + 1)
            folder.mkdir()
            selected = copy.deepcopy(full[index::3])
            plan = {"shard_index": index + 1, "shard_count": 3, "full_count": len(names),
                    "full_names": names, "selected_names": names[index::3],
                    "partitions": [names[offset::3] for offset in range(3)],
                    "ctest_selector": f"{index + 1},,3"}
            for file, data in {"selection-plan.json": plan, "full-inventory.json": {"tests": full},
                               "selected-inventory.json": {"tests": selected},
                               "selected-inventory-after.json": {"tests": selected}}.items():
                (folder / file).write_text(json.dumps(data), encoding="utf-8")
            suite = ET.Element("testsuite", tests=str(len(selected)), failures="0", skipped="0")
            for test in selected:
                ET.SubElement(suite, "testcase", name=test["name"], status="run")
            ET.ElementTree(suite).write(folder / "results.xml")

    def mutate_json(self, file, mutation):
        target = self.root / "1" / file
        value = json.loads(target.read_text())
        mutation(value)
        target.write_text(json.dumps(value))

    def test_complete(self):
        self.assertEqual(MODULE.audit(self.root)["unique_executions"], 10)

    def test_missing_artifact(self):
        (self.root / "1/selection-plan.json").unlink()
        with self.assertRaises(ValueError):
            MODULE.audit(self.root)

    def test_duplicate_index(self):
        self.mutate_json("selection-plan.json", lambda value: value.update(shard_index=2))
        with self.assertRaises(ValueError):
            MODULE.audit(self.root)

    def test_omitted_case(self):
        self.mutate_json("selected-inventory.json", lambda value: value["tests"].pop())
        with self.assertRaises(ValueError):
            MODULE.audit(self.root)

    def test_changed_serial(self):
        self.mutate_json("selected-inventory-after.json", lambda value: value["tests"][0]["properties"][0].update(value=False))
        with self.assertRaises(ValueError):
            MODULE.audit(self.root)

    def test_changed_command(self):
        self.mutate_json("selected-inventory.json", lambda value: value["tests"][0].update(command=["changed"]))
        with self.assertRaises(ValueError):
            MODULE.audit(self.root)

    def test_no_execution(self):
        suite = ET.parse(self.root / "1/results.xml")
        suite.getroot().remove(suite.getroot().find("testcase"))
        suite.write(self.root / "1/results.xml")
        with self.assertRaises(ValueError):
            MODULE.audit(self.root)

    def test_skipped_execution(self):
        suite = ET.parse(self.root / "1/results.xml")
        suite.getroot().set("skipped", "1")
        suite.write(self.root / "1/results.xml")
        with self.assertRaises(ValueError):
            MODULE.audit(self.root)

    def test_actual_failure_node(self):
        suite = ET.parse(self.root / "1/results.xml")
        ET.SubElement(suite.getroot().find("testcase"), "failure")
        suite.write(self.root / "1/results.xml")
        with self.assertRaises(ValueError):
            MODULE.audit(self.root)

    def test_inconsistent_count(self):
        suite = ET.parse(self.root / "1/results.xml")
        suite.getroot().set("tests", "600")
        suite.write(self.root / "1/results.xml")
        with self.assertRaises(ValueError):
            MODULE.audit(self.root)

    def test_different_individually_consistent_inventories(self):
        # Job2 could individually pass while its inventory differs from job1/3.
        folder = self.root / "2"
        for file in ("full-inventory.json", "selected-inventory.json", "selected-inventory-after.json"):
            value = json.loads((folder / file).read_text())
            for test in value["tests"]:
                if test["name"] == "test_1":
                    test["name"] = "foreign_test"
            (folder / file).write_text(json.dumps(value))
        value = json.loads((folder / "selection-plan.json").read_text())
        value["full_names"][1] = "foreign_test"
        value["selected_names"][0] = "foreign_test"
        value["partitions"][1][0] = "foreign_test"
        (folder / "selection-plan.json").write_text(json.dumps(value))
        suite = ET.parse(folder / "results.xml")
        suite.getroot().find("testcase").set("name", "foreign_test")
        suite.write(folder / "results.xml")
        with self.assertRaisesRegex(ValueError, "Different full inventories"):
            MODULE.audit(self.root)


if __name__ == "__main__":
    unittest.main()
