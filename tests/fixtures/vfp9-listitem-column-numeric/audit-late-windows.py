#!/usr/bin/env python3
# Copyright © 2026 Richard M. Hamilton.
# SPDX-License-Identifier: GPL-3.0-only
# Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
"""Capture-only report of the retained failed attempt, never a passing gate."""
import json
from pathlib import Path
import sys
import xml.etree.ElementTree as ET

root = Path(sys.argv[1])
run = json.loads((root / "completed-run.json").read_text())
assert run["headSha"] == "8968c625a84512394ca3604a9b77d7b7baca62b4"
assert run["conclusion"] == "failure" and run["status"] == "completed"
assert (root / "watch.status").read_text() == "1\n"
inventories, attempted, reports = [], [], []
for result in sorted((root / "artifacts").rglob("results.xml")):
    def read(name):
        return json.loads((result.parent / name).read_text(encoding="utf-8-sig"))
    full = read("full-inventory.json")["tests"]
    selected = read("selected-inventory.json")["tests"]
    plan = read("selection-plan.json")
    suite = ET.parse(result).getroot()
    cases = suite.findall("testcase")
    names = [case.attrib["name"] for case in cases]
    assert plan["selected_names"] == [test["name"] for test in selected]
    assert sorted(names) == sorted(plan["selected_names"])
    assert len(names) == int(suite.attrib["tests"])
    assert int(suite.attrib["skipped"]) == 0
    assert all(case.find("skipped") is None and case.find("error") is None for case in cases)
    inventories.append([test["name"] for test in full])
    attempted.extend(names)
    failures = [dict(case.attrib, failure=case.find("failure").attrib)
                for case in cases if case.find("failure") is not None]
    assert len(failures) == int(suite.attrib["failures"])
    assert all(case.attrib["status"] == "run" for case in cases if case.find("failure") is None)
    reports.append({"shard": plan["shard_index"], "tests": len(names),
                    "failures": failures,
                    "after_inventory_present": (result.parent / "selected-inventory-after.json").exists()})
assert len(inventories) == 3 and all(names == inventories[0] for names in inventories)
assert len(attempted) == len(set(attempted)) == len(inventories[0]) == 610
assert sorted(attempted) == sorted(inventories[0])
failures = [failure for report in reports for failure in report["failures"]]
assert len(failures) == 1
assert failures[0]["name"] == "test_prg_engine_numeric_behavior"
assert failures[0]["failure"] == {"message": "Timeout"}
assert failures[0]["time"] == "180.07" and failures[0]["status"] == "fail"
assert [r["shard"] for r in reports if not r["after_inventory_present"]] == [1]
print(json.dumps({"audit": "failed-attempt-capture-only", "attempted_exactly_once": 610,
                  "passed": 609, "failed": 1, "complete_passing_qualification": False,
                  "shards": reports}, sort_keys=True))
