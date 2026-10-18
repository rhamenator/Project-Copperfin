# Copyright © 2026 Richard M. Hamilton.
# SPDX-License-Identifier: GPL-3.0-only
# Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
"""Diagnose the first expanded ORIGINAL and its separate query readback gap."""
import gzip
import json
from pathlib import Path
import re
import sys
import xml.etree.ElementTree as ET

p = Path(sys.argv[1])
def raw(name):
    f = p / name
    return f.read_bytes() if f.exists() else gzip.decompress((p / (name + ".gz")).read_bytes())
def text(name):
    return raw(name).decode("ascii")

assert text("build.status") == "0\n" and text("ctest.status") == "8\n"
suite = ET.fromstring(raw("results.xml"))
cases = suite.findall("testcase")
expected_failures = {"test_prg_engine_numeric_constant_division", "test_prg_engine_numeric_nonwhole_division"}
assert len(cases) == 4 and len({c.attrib["name"] for c in cases}) == 4
assert int(suite.attrib["failures"]) == 2 and int(suite.attrib["skipped"]) == int(suite.attrib.get("errors", "0")) == 0
for c in cases:
    failed = c.attrib["name"] in expected_failures
    assert c.attrib["status"] == ("fail" if failed else "run")
    assert (c.find("failure") is not None) == failed
    assert c.find("error") is None and c.find("skipped") is None
assert {c.attrib["name"] for c in cases} == expected_failures | {
    "test_native_test_isolation_contract", "test_cloud_validation_sanitizer_selection"}
log = re.sub(r"^\d+: ", "", text("ctest.log"), flags=re.M)
constant = re.findall(r"^constant division: descriptor=(\d+) bits=(\d+) evaluation=(\d+) controls=(\d+) total=(\d+)$", log, re.M)
nonwhole = re.findall(r"^non-whole division: descriptor=(\d+) bits=(\d+) literalBits=(\d+) evaluation=(\d+) controls=(\d+) total=(\d+)$", log, re.M)
assert constant == [("60", "0", "0", "0", "60")]
assert nonwhole == [("12480", "0", "0", "0", "720", "13200")]
failures = [s for s in log.splitlines() if s.startswith("FAIL: ")]
assert len(failures) == 13260
counts = {}
for suffix in (" native descriptor", " native precision survives pause and scalar assignment", " native final setting"):
    counts[suffix.strip()] = sum(s.endswith(suffix) for s in failures)
assert counts == {"native descriptor": 11100, "native precision survives pause and scalar assignment": 1440, "native final setting": 720}
inv = json.loads((Path(__file__).parent / "nonwhole-timing-cases.json").read_text())
target14 = {r["expression"] for r in inv["rows"] if r.get("core") and "0.00" not in r["core"] and not r["observesArgument"] and r["changesTo"] == 14}
assert len(target14) == 36
for s in failures:
    if s.endswith(" native final setting"):
        m = re.fullmatch(r"FAIL: (COPPERFIN|VFP9)/D(0|2|6|14|18)/FIXED(OFF|ON) callback (.+) native final setting", s)
        assert m and m[4] in target14

q = p / "query-diagnostic"
for stem in ("build", "public", "first", "repeat"):
    assert (q / (stem + ".status")).read_text() == "0\n"
    assert (q / (stem + ".stderr")).stat().st_size == 0
expected = []
for mode in ("COPPERFIN", "VFP9"):
    for setting in (0, 14):
        expected.extend([f'{mode}|{setting}|SET(\'DECIMALS\')|kind=3|number=0|string="{setting}"',
            f'{mode}|{setting}|VARTYPE(SET(\'DECIMALS\'))|kind=3|number=0|string="C"',
            f'{mode}|{setting}|VAL(TRANSFORM(SET(\'DECIMALS\')))|kind=2|number={setting}|string=""'])
assert (q / "public.out").read_text().splitlines() == expected
typed = []
for stem in ("first", "repeat"):
    b = (q / (stem + "-results.txt")).read_bytes()
    assert b == (q / (stem + ".out")).read_bytes()
    lines = b.decode("ascii").split("\r\n")
    assert lines.pop() == "" and lines.pop() == "DONE|2"
    assert lines.pop(0) == "VERSION|Visual FoxPro 09.00.0000.7423 for Windows" and len(lines) == 2
    run = []
    for line, setting in zip(lines, (0, 14)):
        f = line.split("|")
        assert f[:3] == ["QUERY", str(setting), "N"] and f[4] == str(setting)
        m = dict(s.split("=") for s in f[3].split(";"))
        assert m["type"] == "49" and int(m["width"], 16) == 10 and int(m["integer"], 16) == setting and m["cw"] == "027F"
        run.append({"setting": setting, "type": "Integer", "width": 10, "value": setting, "normalizedReadback": setting})
    typed.append(run)
assert typed[0] == typed[1]
print(json.dumps({"expandedOriginal": {"executed": 4, "constantDescriptor": 60, "nonwholeDescriptor": 12480,
    "bits": 0, "literalBits": 0, "evaluation": 0, "finalSettingControl": 720},
    "queryDiagnosis": {"native": typed[0], "copperfinBothModes": "Character with correct decimal text; inactive number field0",
        "normalizedReadbackBothHosts": [0,14], "issue": 7208},
    "conclusion": "Preserve the failing run; normalize only the observation adapter and refreeze before support. Query type parity remains a separate unfixed issue."}, indent=2))
