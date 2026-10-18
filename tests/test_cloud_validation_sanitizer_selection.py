# Copyright © 2026 Richard M. Hamilton.
# SPDX-License-Identifier: GPL-3.0-only
# Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
"""RQ-CF-PRG-NUMERIC-LITERAL-METADATA-VERIFY-001: inspect, never run, driver."""
import ast
from pathlib import Path
import re
import unittest

SOURCE = Path(__file__).resolve().parents[1] / "scripts/run-cloud-validation.py"
TREE = ast.parse(SOURCE.read_text(encoding="utf-8"), filename=str(SOURCE))
TARGETS = ast.literal_eval(next(node.value for node in TREE.body
                              if isinstance(node, ast.Assign) and
                              any(isinstance(t, ast.Name) and t.id == "TARGETS" for t in node.targets)))
MAIN = next(node for node in TREE.body if isinstance(node, ast.FunctionDef) and node.name == "main")
# The existing driver supplies its PR selector as one constant argument to
# command.extend. Inspect that actual AST argument, not matching comments or
# merely counting test-name strings anywhere in the source.
SELECTORS = [node.args[0].elts[1].value for node in ast.walk(MAIN)
             if isinstance(node, ast.Call) and isinstance(node.func, ast.Attribute)
             and isinstance(node.func.value, ast.Name) and node.func.value.id == "command"
             and node.func.attr == "extend" and len(node.args) == 1
             and isinstance(node.args[0], ast.List) and len(node.args[0].elts) == 2
             and isinstance(node.args[0].elts[0], ast.Constant) and node.args[0].elts[0].value == "-R"
             and isinstance(node.args[0].elts[1], ast.Constant)]


class SanitizerSelection(unittest.TestCase):
    def test_one_pr_selector(self):
        self.assertEqual(len(SELECTORS), 1, "recover the new driver shape before changing this contract")

    def test_every_built_sanitizer_test_is_executed(self):
        self.assertEqual(len(SELECTORS), 1)
        selector = re.compile(SELECTORS[0])
        for name in TARGETS["sanitizer-pr"]:
            with self.subTest(name=name):
                self.assertIsNotNone(selector.search(name), "built target omitted from actual CTest filter")

    def test_numeric_metadata_tests_are_built_and_run_with_exact_names(self):
        self.assertEqual(len(SELECTORS), 1)
        selector = re.compile(SELECTORS[0])
        for name in ("test_prg_numeric_literal_metadata_helper", "test_prg_engine_numeric_literal_metadata",
                     "test_prg_numeric_unary_metadata_helper", "test_prg_engine_numeric_unary_metadata",
                     "test_prg_numeric_expression_context_helper", "test_prg_engine_numeric_zero_context",
                     "test_prg_numeric_integer_metadata_helper", "test_prg_engine_numeric_integer_metadata",
                     "test_prg_numeric_integer_assignment_helper", "test_prg_engine_numeric_integer_assignment",
                     "test_prg_numeric_real_assignment_helper", "test_prg_engine_numeric_real_assignment",
                     "test_prg_numeric_runtime_division_helper", "test_prg_engine_numeric_runtime_division",
                     "test_prg_numeric_constant_division_intrinsic_helper", "test_prg_engine_numeric_constant_division",
                     "test_prg_engine_numeric_nonwhole_division"):
            with self.subTest(name=name):
                self.assertIn(name, TARGETS["sanitizer-pr"])
                self.assertIsNotNone(selector.search(name))
                self.assertIsNone(selector.search("prefix_" + name))
                self.assertIsNone(selector.search(name + "_suffix"))


if __name__ == "__main__":
    unittest.main()
