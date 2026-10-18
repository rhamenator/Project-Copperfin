# Copyright © 2026 Richard M. Hamilton.
# SPDX-License-Identifier: GPL-3.0-only
# Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
"""RQ-CF-PRG-NUMERIC-CONSTANT-DIVISION-NONWHOLE-001: actual re-entry diagnosis.

Read results only; import no product helper or post-capture arithmetic model.
The earlier ORIGINAL auditor remains unchanged for its distinct baseline.
"""
import json
from pathlib import Path
import re
import sys
import xml.etree.ElementTree as ET

root = Path(sys.argv[1])
assert (root / "refreshed-original-build.status").read_text() == "0\n"
assert (root / "refreshed-original-ctest.status").read_text() == "8\n"
suite = ET.parse(root / "refreshed-original-junit.xml").getroot()
cases = suite.findall("testcase")
public = "test_prg_engine_numeric_nonwhole_division"
assert len(cases) == 3 and {c.attrib["name"] for c in cases} == {
    public, "test_native_test_isolation_contract", "test_cloud_validation_sanitizer_selection"}
assert int(suite.attrib["failures"]) == 1 and int(suite.attrib["skipped"]) == 0
assert int(suite.attrib.get("errors", "0")) == 0
for case in cases:
    assert case.attrib["status"] == "run"
    assert case.find("error") is None and case.find("skipped") is None
    assert (case.find("failure") is not None) == (case.attrib["name"] == public)

log = re.sub(r"^\d+: ", "", (root / "refreshed-original-ctest.log").read_text(), flags=re.M)
pattern = (r"^non-whole division: descriptor=(\d+) bits=(\d+) literalBits=(\d+) "
           r"evaluation=(\d+) controls=(\d+) total=(\d+)$")
matches = re.findall(pattern, log, flags=re.M)
assert len(matches) == 1
descriptor, bits, literal_bits, evaluation, controls, total = map(int, matches[0])
assert descriptor > 0 and bits == literal_bits == evaluation == controls == 0
assert total == descriptor
failures = [line for line in log.splitlines() if line.startswith("FAIL: ")]
assert len(failures) == descriptor
inventory = json.loads((Path(__file__).parent / "nonwhole-grid-cases.json").read_text())
assert len(inventory["rows"]) == 578
admitted = {f'{row["kind"]}:{row["index"]} {row["expression"]}' for row in inventory["rows"]
            if row["kind"] != "retained-outside-scope"}
assert len(admitted) == 574
contexts = {f"{mode}/D{setting}/FIXED{fixed}": []
            for mode in ("COPPERFIN", "VFP9") for setting in (0, 2, 6, 14, 18) for fixed in ("OFF", "ON")}
for line in failures:
    match = re.fullmatch(r"FAIL: (\S+) (.+) native descriptor", line)
    assert match and match[1] in contexts and match[2] in admitted
    contexts[match[1]].append(match[2])
for labels in contexts.values():
    assert labels and len(labels) == len(set(labels))
assert sum(map(len, contexts.values())) == descriptor
print(json.dumps({"baseline": "4b6cc734fb1b70cf7cbcdd1f083fafa1317fa4f9",
    "public_freeze": "274a38635a463db154b4b9e0c78c502d6722c53c",
    "executed_tests": 3, "expected_original_failed_tests": [public],
    "native_rows": 578, "admitted_exact_rows": 574, "descriptor": descriptor,
    "bits": bits, "literalBits": literal_bits, "evaluation": evaluation,
    "controls": controls, "total": total,
    "descriptor_failures_by_context": {k: len(v) for k, v in contexts.items()},
    "conclusion": "Integrated literal producer removes observed payload failures; division metadata remains an actual ORIGINAL failure. No division hook or timing/parity acceptance claim."}, indent=2))
