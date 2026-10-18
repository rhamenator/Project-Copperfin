# Copyright © 2026 Richard M. Hamilton.
# SPDX-License-Identifier: GPL-3.0-only
# Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
"""Audit actual local execution receipts; never generate Numeric expectations."""
import gzip
import json
from pathlib import Path
import sys
import xml.etree.ElementTree as ET

root = Path(sys.argv[1])
def receipt(name):
    plain = root / name
    if plain.is_file():
        return plain.read_text()
    with gzip.open(root / (name + ".gz"), "rt") as stream:
        return stream.read()

expected = {
    "test_prg_engine_numeric_literal_payload", "test_prg_numeric_literal_payload_helper",
    "test_prg_numeric_literal_metadata_helper", "test_prg_engine_numeric_literal_metadata",
    "test_prg_engine_numeric_integer_metadata", "test_prg_engine_numeric_unary_metadata",
    "test_prg_engine_numeric_runtime_division", "test_prg_engine_numeric_constant_division",
    "test_prg_engine_numeric_real_assignment", "test_prg_engine_numeric_integer_assignment",
    "test_prg_engine_int64_exact_expression", "test_prg_engine_currency_literals",
    "test_prg_engine_currency_functions", "test_prg_engine_currency_overflow",
    "test_prg_engine_currency_exact_muldiv", "test_prg_engine_macro_expressions",
    "test_prg_engine_control_flow", "test_prg_engine_numeric_behavior",
    "test_cloud_validation_sanitizer_selection", "test_native_test_isolation_contract",
}
for phase_receipt in ("build", "same-ctest", "broader-ctest", "staged-source-check"):
    assert receipt(phase_receipt + ".status") == "0\n", phase_receipt
assert not receipt("same-ctest.stderr")
assert not receipt("broader-ctest.stderr")

inventories = {}
for phase, wanted in (("same", {"test_prg_engine_numeric_literal_payload",
                               "test_prg_numeric_literal_payload_helper"}),
                      ("broader", expected)):
    suite = ET.parse(root / (phase + "-junit.xml")).getroot()
    cases = list(suite.iter("testcase"))
    assert len(cases) == len(wanted)
    assert {case.attrib["name"] for case in cases} == wanted
    for case in cases:
        assert case.attrib.get("status") == "run", case.attrib
        assert not any(list(case.iter(tag)) for tag in ("failure", "error", "skipped"))
        if case.attrib["name"] == "test_prg_engine_numeric_literal_payload":
            assert case.findtext("system-out") == (
                "literal payload: descriptor=0 bits=0 evaluation=0 controls=0 total=0\n")
        if case.attrib["name"] == "test_prg_numeric_literal_payload_helper":
            assert case.findtext("system-out") == "literal payload helper: selected=458 failures=0\n"
    inventories[phase] = {"executed": len(cases), "failures": 0, "errors": 0, "skips": 0,
                          "names": sorted(wanted)}
print(json.dumps({"requirement": "RQ-CF-PRG-NUMERIC-DECIMAL-LITERAL-PAYLOAD-001",
                  "sourceCheckpoint": "a6cfc9954580e45c6c8cd2b760c6a3945369f6e5",
                  "actualLocalExecution": inventories,
                  "scope": "GNU local execution only, not hosted/review/merge acceptance"}, indent=2))
