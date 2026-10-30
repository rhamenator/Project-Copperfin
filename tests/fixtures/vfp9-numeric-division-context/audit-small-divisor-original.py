# Copyright © 2026 Richard M. Hamilton.
# SPDX-License-Identifier: GPL-3.0-only
# Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
"""Read complete actual ORIGINAL evidence; never import product/candidate expectations."""
import gzip
import hashlib
import json
from pathlib import Path
import re
import sys
import xml.etree.ElementTree as ET

root = Path(sys.argv[1])

def raw(name):
    file = root / name
    return file.read_bytes() if file.exists() else gzip.decompress((root / (name + ".gz")).read_bytes())

for phase in ("configure", "build", "worker"):
    assert raw(phase + ".status") == b"0\n", phase
assert raw("ctest.status") == b"8\n", "actual CTest failure must be retained"
assert raw("production-before.diff") == raw("production-after.diff") == b""
source = raw("source-before.log")
assert source == raw("source-after-build.log") == raw("source-after-test.log")
assert source.count(b": OK\n") == 18
suite = ET.fromstring(raw("results.xml"))
cases = suite.findall("testcase")
expected = {
    "test_prg_engine_numeric_small_divisor",
    "test_prg_engine_numeric_constant_division", "test_prg_engine_numeric_nonwhole_division",
    "test_prg_numeric_constant_division_intrinsic_helper", "test_prg_numeric_nonwhole_division_helper",
    "test_native_test_isolation_contract", "test_cloud_validation_sanitizer_selection",
}
assert len(cases) == len(expected) and {c.attrib["name"] for c in cases} == expected
assert int(suite.attrib["tests"]) == 7 and int(suite.attrib["failures"]) == 1
assert all(int(suite.attrib.get(k, "0")) == 0 for k in ("skipped", "errors"))
failed = None
for case in cases:
    assert case.find("skipped") is None and case.find("error") is None
    if case.attrib["name"] == "test_prg_engine_numeric_small_divisor":
        assert case.attrib["status"] == "fail" and case.find("failure") is not None
        failed = case.findtext("system-out")
    else:
        assert case.attrib["status"] == "run" and case.find("failure") is None
assert failed
# CTest's complete --output-on-failure console log retains this entire failed
# case; JUnit may clip system-out at its default307200-byte threshold. Keep
# that clipped artifact/first reader failure, but do not infer missing counters.
console = raw("ctest.log").decode("utf8")
block = re.search(r"^\d+/7 Test #\d+: test_prg_engine_numeric_small_divisor[^\n]*\*\*\*Failed[^\n]*\n(.*?^small-divisor division:[^\n]*\n)", console, re.M | re.S)
assert block, "complete independently retained failure block is required"
complete = block.group(1)
assert failed.startswith(complete[:1024]), "bind clipped JUnit to the complete console case"
junit_clipped = "test output was removed since it exceeds the threshold" in failed
if not junit_clipped:
    assert failed.strip() == complete.strip()
failed = complete
summary = re.findall(r"^small-divisor division: descriptor=(\d+) bits=(\d+) literalBits=(\d+) evaluation=(\d+) display=(\d+) controls=(\d+) total=(\d+)$", failed, re.M)
assert len(summary) == 1
descriptor, bits, literal_bits, evaluation, display, controls, total = map(int, summary[0])
assert descriptor > 0 and literal_bits == evaluation == controls == 0
assert total == descriptor + bits + evaluation + display + controls
failures = re.findall(r"^FAIL: (.*)$", failed, re.M)
assert len(failures) == total
assert len(re.findall(r"^FAIL: ", console, re.M)) == total
assert sum(x.endswith("native descriptor") or x.endswith("native precision survives pause and scalar assignment") for x in failures) == descriptor
assert sum(" bits actual=" in x or x.endswith("native quotient bits") for x in failures) == bits
assert sum(x.endswith("native display") for x in failures) == display
bit_pairs = {}
for actual, native in re.findall(r" bits actual=([0-9A-F]{16}) native=([0-9A-F]{16})$", failed, re.M):
    key = actual + ":" + native
    bit_pairs[key] = bit_pairs.get(key, 0) + 1
display_kinds = {}
for line in failures:
    if line.endswith("native display"):
        kind = line.split()[1].split(":")[0]
        display_kinds[kind] = display_kinds.get(kind, 0) + 1
print(json.dumps({"requirement": "RQ-CF-PRG-NUMERIC-CONSTANT-DIVISION-SMALL-DIVISOR-001",
    "sourceFreeze": "3b7a5163618ea92420f63b70c5cac265cdd08807",
    "actualExecuted": 7, "actualFailed": 1, "actualSkipped": 0,
    "descriptor": descriptor, "bits": bits, "literalBits": literal_bits,
    "evaluation": evaluation, "display": display, "controls": controls, "total": total,
    "payloadBitPairs": bit_pairs, "displayFailureKinds": display_kinds,
    "junitFailedSystemOutClipped": junit_clipped,
    "completeFailedOutputSource": "Lossless full CTest console block, exact summary/failure-category counts and JUnit prefix binding; no rerun or changed test expectation.",
    "frozenSourceAndHeaderHashes": 18,
    "rawJUnitSha256": hashlib.sha256(raw("results.xml")).hexdigest(),
    "rawCTestSha256": hashlib.sha256(raw("ctest.log")).hexdigest(),
    "conclusion": "Actual unchanged-production ORIGINAL: only the new public regression fails. Exact failures retained; this is not support, SAME or compatibility completion."}, indent=2))
