# Copyright © 2026 Richard M. Hamilton.
# SPDX-License-Identifier: GPL-3.0-only
# Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
"""Read the corrected expanded ORIGINAL independently of product/candidate code."""
import gzip
import json
from pathlib import Path
import re
import sys
import xml.etree.ElementTree as ET

root = Path(sys.argv[1])
def raw(name):
    f = root / name
    return f.read_bytes() if f.exists() else gzip.decompress((root / (name + ".gz")).read_bytes())
assert raw("build.status") == b"0\n" and raw("ctest.status") == b"8\n"
suite = ET.fromstring(raw("results.xml"))
cases = suite.findall("testcase")
failed = {"test_prg_engine_numeric_constant_division", "test_prg_engine_numeric_nonwhole_division"}
assert len(cases) == 4 and {c.attrib["name"] for c in cases} == failed | {
    "test_native_test_isolation_contract", "test_cloud_validation_sanitizer_selection"}
assert int(suite.attrib["tests"]) == 4 and int(suite.attrib["failures"]) == 2
assert int(suite.attrib["skipped"]) == int(suite.attrib.get("errors", "0")) == 0
for case in cases:
    expected = case.attrib["name"] in failed
    assert case.attrib["status"] == ("fail" if expected else "run")
    assert (case.find("failure") is not None) == expected
    assert case.find("error") is None and case.find("skipped") is None
log = re.sub(r"^\d+: ", "", raw("ctest.log").decode("ascii"), flags=re.M)
constant = re.findall(r"^constant division: descriptor=(\d+) bits=(\d+) evaluation=(\d+) controls=(\d+) total=(\d+)$", log, re.M)
nonwhole = re.findall(r"^non-whole division: descriptor=(\d+) bits=(\d+) literalBits=(\d+) evaluation=(\d+) controls=(\d+) total=(\d+)$", log, re.M)
assert constant == [("60", "0", "0", "0", "60")]
assert nonwhole == [("12480", "0", "0", "0", "0", "12480")]
failures = [line for line in log.splitlines() if line.startswith("FAIL: ")]
assert len(failures) == 12540
assert sum(line.endswith(" native descriptor") for line in failures) == 11100
assert sum(line.endswith(" native precision survives pause and scalar assignment") for line in failures) == 1440
inv = json.loads((Path(__file__).parent / "nonwhole-timing-cases.json").read_text())
callbacks = {r["expression"] for r in inv["rows"] if r.get("core") and "0.00" not in r["core"] and not r["observesArgument"]}
assert len(callbacks) == 72
observed = set()
for line in failures:
    if line.endswith(" native precision survives pause and scalar assignment"):
        match = re.fullmatch(r"FAIL: (COPPERFIN|VFP9)/D(0|2|6|14|18)/FIXED(OFF|ON) callback (.+) native precision survives pause and scalar assignment", line)
        assert match and match[4] in callbacks
        key = match.groups()
        assert key not in observed
        observed.add(key)
assert len(observed) == 72 * 20
print(json.dumps({"baseline": "4b6cc734fb1b70cf7cbcdd1f083fafa1317fa4f9",
    "expandedFreeze": "569b36e84236a7f1c7f97492a8a92d136bbecce5",
    "queryAdapterRefreeze": "0967ad091475ffdb4600bba60536df5b6ef77e00",
    "actualExecuted": 4, "expectedOriginalFailures": sorted(failed),
    "constantDescriptor": 60, "nonwholeDescriptor": 12480,
    "nonwholePriorDescriptor": 11040, "callbackDescriptor": 1440,
    "bits": 0, "literalBits": 0, "evaluation": 0, "controls": 0,
    "callbackForms": 72, "contextsPerForm": 20,
    "conclusion": "Expanded source unchanged after adapter refreeze; actual metadata-only ORIGINAL failures retained before any non-whole support/hook. SET query type issue7208 remains unfixed."}, indent=2))
