# Copyright © 2026 Richard M. Hamilton.
# SPDX-License-Identifier: GPL-3.0-only
# Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
"""Inspect actual SAME execution without product or candidate-model imports."""
import gzip
import json
from pathlib import Path
import re
import sys
import xml.etree.ElementTree as ET

root = Path(sys.argv[1])
def raw(name):
    source = root / name
    return source.read_bytes() if source.exists() else gzip.decompress((root / (name + ".gz")).read_bytes())

for phase in ("build", "ctest", "frozen-source-before", "frozen-source-after"):
    assert raw(phase + ".status") == b"0\n", phase
assert raw("frozen-source-before.log") == raw("frozen-source-after.log")
assert raw("frozen-source-before.log").count(b": OK\n") == 7
suite = ET.fromstring(raw("results.xml"))
cases = suite.findall("testcase")
expected = {
    "test_prg_engine_numeric_constant_division", "test_prg_engine_numeric_nonwhole_division",
    "test_prg_numeric_constant_division_intrinsic_helper", "test_prg_numeric_nonwhole_division_helper",
    "test_native_test_isolation_contract", "test_cloud_validation_sanitizer_selection",
}
assert len(cases) == len(expected) and {c.attrib["name"] for c in cases} == expected
assert int(suite.attrib["tests"]) == 6
for attr in ("failures", "skipped", "errors"):
    assert int(suite.attrib.get(attr, "0")) == 0, attr
for case in cases:
    assert case.attrib["status"] == "run"
    assert all(case.find(tag) is None for tag in ("failure", "error", "skipped"))
log = re.sub(r"^\d+: ", "", raw("ctest.log").decode("utf8"), flags=re.M)
assert not re.search(r"^FAIL: ", log, re.M)
assert re.findall(r"^constant division: descriptor=(\d+) bits=(\d+) evaluation=(\d+) controls=(\d+) total=(\d+)$", log, re.M) == [("0",) * 5]
assert re.findall(r"^non-whole division: descriptor=(\d+) bits=(\d+) literalBits=(\d+) evaluation=(\d+) controls=(\d+) total=(\d+)$", log, re.M) == [("0",) * 6]
assert log.count("68 native nonwhole roots/314 syntax nodes, 640 older native rows and bounded-domain controls passed") == 1
assert log.count("640 frozen native intrinsic rows, materialization and unsupported-domain controls passed") == 1
print(json.dumps({"requirement": "RQ-CF-PRG-NUMERIC-CONSTANT-DIVISION-NONWHOLE-001",
    "hook": "146d4c6ddcc834feb5b259d46a7950bf461bf435",
    "actualExecuted": 6, "actualFailedOrSkipped": 0,
    "constantDescriptor": 0, "nonwholeDescriptor": 0,
    "bits": 0, "literalBits": 0, "evaluation": 0, "controls": 0,
    "frozenSourceAndHeaderHashes": 7,
    "conclusion": "Unchanged expanded public SAME and native direct/whole helpers executed and passed. Hosted/full-suite/review/strict merge gates remain separate."}, indent=2))
