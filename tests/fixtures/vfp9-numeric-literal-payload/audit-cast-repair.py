# Copyright © 2026 Richard M. Hamilton.
# SPDX-License-Identifier: GPL-3.0-only
# Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
"""Check actual old/fixed CAST receipts, never derive Numeric expected tables."""
import gzip
import json
import math
from pathlib import Path
import struct
import sys
import xml.etree.ElementTree as ET

root = Path(sys.argv[1])
def read(name):
    path = root / name
    if path.is_file():
        return path.read_text()
    with gzip.open(root / (name + ".gz"), "rt") as stream:
        return stream.read()

inner = struct.unpack(">d", bytes.fromhex("BFEFFFFFFFFFFFFF"))[0]
assert inner == math.nextafter(-1.0, 0.0) and math.trunc(inner) == 0
assert math.trunc(struct.unpack(">d", bytes.fromhex("BFF0000000000000"))[0]) == -1
failures = 0
for family, direct, public, per_context in (("byte",140,126,8),("uint64",62,159,9)):
    assert read(f"original-{family}-build.status") == "0\n"
    assert read(f"original-{family}-direct.status") == "0\n"
    assert f"direct={direct} public=0" in read(f"original-{family}-direct.out")
    assert not read(f"original-{family}-direct.stderr")
    for mode in ("COPPERFIN", "VFP9"):
        for session in (1,2):
            stem = f"original-{family}-{mode}-{session}"
            assert read(stem+".status") == "1\n"
            assert f"direct=0 public={public}" in read(stem+".out")
            lines = read(stem+".stderr").splitlines()
            assert len(lines) == per_context
            assert all(line.startswith("FAIL:") and "-0.9999999999999999" in line for line in lines)
            failures += len(lines)
assert failures == 68
assert read("fixed-build.status") == read("fixed-ctest.status") == "0\n"
cases = list(ET.fromstring(read("fixed-junit.xml")).iter("testcase"))
families = ("test_prg_engine_cast_byte_numeric", "test_prg_engine_cast_uint64_numeric")
expected = {family+suffix for family in families for suffix in
            ("", "_copperfin_session1", "_copperfin_session2", "_vfp9_session1", "_vfp9_session2")}
expected.update(("test_prg_engine_numeric_literal_payload", "test_prg_numeric_literal_payload_helper",
                 "test_cloud_validation_sanitizer_selection", "test_native_test_isolation_contract"))
assert len(cases)==14 and {case.attrib["name"] for case in cases}==expected
for case in cases:
    assert case.attrib.get("status")=="run"
    assert not any(list(case.iter(tag)) for tag in ("failure","error","skipped"))
    name, output = case.attrib["name"], case.findtext("system-out") or ""
    for family,direct,public in ((families[0],140,128),(families[1],62,162)):
        if name.startswith(family):
            assert (f"direct={direct} public=0" if name==family else f"direct=0 public={public}") in output
    if name=="test_prg_engine_numeric_literal_payload":
        assert "descriptor=0 bits=0 evaluation=0 controls=0 total=0" in output
    if name=="test_prg_numeric_literal_payload_helper":
        assert "selected=458 failures=0" in output
print(json.dumps({"originalSourceHead":"b8c1325c7b466237ed58eeac5ac0c8d2daaa7f37",
                  "originalAssertions":failures,"fixedExecuted":14,"fixedFailuresErrorsSkips":0,
                  "scope":"GNU replay/fixed tests only; new-head hosted/review gates remain"},indent=2))
