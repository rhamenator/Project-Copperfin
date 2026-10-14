#!/usr/bin/env python3
"""RQ-CF-CI-WINDOWS-NATIVE-BUDGET-001: fail-closed aggregate execution proof."""
import argparse
import json
from pathlib import Path
import xml.etree.ElementTree as ET


def require(condition, message):
    if not condition:
        raise ValueError(message)


def read_json(path):
    return json.loads(path.read_text(encoding="utf-8-sig"))


def audit(root):
    plans = sorted(root.rglob("selection-plan.json"))
    require(len(plans) == 3, "Exactly three current-attempt shard artifacts required")
    inventories = []
    seen_shards = set()
    executed = []
    reports = []
    for plan_path in plans:
        folder = plan_path.parent
        plan = read_json(plan_path)
        shard = plan["shard_index"]
        require(type(shard) is int and shard in (1, 2, 3) and shard not in seen_shards,
                "Unique bounded shard indices required")
        seen_shards.add(shard)
        require(plan["shard_count"] == 3, "All jobs must use the same three-shard policy")
        full = read_json(folder / "full-inventory.json")["tests"]
        names = [test["name"] for test in full]
        require(names and all(isinstance(name, str) and name.strip() for name in names),
                "Nonempty complete inventory names required")
        require(len(set(names)) == len(names), "Duplicate full inventory names rejected")
        require(plan["full_count"] == len(names) and plan["full_names"] == names,
                "Plan must match the actual full inventory")
        partitions = [names[index::3] for index in range(3)]
        require(plan["partitions"] == partitions and plan["selected_names"] == partitions[shard - 1],
                "Stored partition must match independent complete-inventory coverage")
        require(plan["ctest_selector"] == f"{shard},,3", "Exact selector required")
        inventories.append(names)
        selected = read_json(folder / "selected-inventory.json")["tests"]
        after = read_json(folder / "selected-inventory-after.json")["tests"]
        expected = partitions[shard - 1]
        require([test["name"] for test in selected] == expected, "Actual selection omitted/duplicated tests")
        full_by_name = {test["name"]: test for test in full}
        require([test["name"] for test in after] == expected, "Post-execution selection changed")
        for test in selected + after:
            original = full_by_name[test["name"]]
            require(test.get("command") and test["command"] == original.get("command"),
                    "Built command must remain unchanged")
            require(test["properties"] == original["properties"], "Test/isolation properties changed")
        for test in full:
            for prop in test["properties"]:
                require(not (prop["name"] == "DISABLED" and prop["value"] is True), "Disabled test rejected")
                require(not (prop["name"] in {"FIXTURES_SETUP", "FIXTURES_REQUIRED", "FIXTURES_CLEANUP", "DEPENDS"}
                             and prop["value"]), "Cross-test fixture/dependency rejected")
        suite = ET.parse(folder / "results.xml").getroot()
        require(suite.tag == "testsuite", "CTest JUnit root required")
        require(int(suite.attrib["failures"]) == 0 and int(suite.attrib["skipped"]) == 0
                and int(suite.attrib.get("errors", "0")) == 0, "No skipped/failed/error execution allowed")
        cases = suite.findall("testcase")
        actual = [case.attrib["name"] for case in cases]
        require(len(actual) == len(expected) and sorted(actual) == sorted(expected),
                "Exact selected execution multiset required")
        require(int(suite.attrib["tests"]) == len(actual), "Declared execution count inconsistent")
        for case in cases:
            require(case.attrib.get("status") == "run" and
                    all(case.find(tag) is None for tag in ("failure", "error", "skipped")),
                    "Every test must actually run successfully")
        executed.extend(actual)
        reports.append({"shard": shard, "selected": len(expected), "executed": len(actual)})
    require(all(names == inventories[0] for names in inventories),
            "Different full inventories across jobs cannot qualify one complete suite")
    require(len(executed) == len(set(executed)) and sorted(executed) == sorted(inventories[0]),
            "Aggregate execution must cover every full-inventory test exactly once")
    return {"complete": True, "full_inventory": len(inventories[0]), "unique_executions": len(executed),
            "shards": sorted(reports, key=lambda report: report["shard"]),
            "note": "Full native execution coverage only; no independent-human/release certification claim."}


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("root", type=Path)
    args = parser.parse_args()
    print(json.dumps(audit(args.root), sort_keys=True))
