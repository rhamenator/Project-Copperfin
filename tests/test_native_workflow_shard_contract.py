"""VR-CI-7181-002: registered whole-workflow contract, including rejection checks."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

REPO = Path(__file__).resolve().parents[1]
CHECKER = REPO / "tests/run_native_platform_workflow_contract_check.cmake"


class WorkflowContractTests(unittest.TestCase):
    def setUp(self):
        base = REPO / "build/native-contract-unit"
        base.mkdir(parents=True, exist_ok=True)
        self.temp = tempfile.TemporaryDirectory(prefix="contract-", dir=base)
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        # Only .github copies are mutable. Other referenced inputs are read-only
        # links; cleanup removes links, never their original targets.
        for item in REPO.iterdir():
            if item.name in {".git", "build"}:
                continue
            if item.name == ".github":
                shutil.copytree(item, self.root / item.name)
            else:
                (self.root / item.name).symlink_to(item, target_is_directory=item.is_dir())

    def run_contract(self):
        return subprocess.run(["cmake", f"-DSOURCE_DIR={self.root}", "-P", str(CHECKER)],
                              text=True, capture_output=True, check=False)

    def mutate(self, relative, old, new):
        target = self.root / relative
        self.assertFalse(target.is_symlink())
        original = target.read_text()
        self.assertIn(old, original)
        target.write_text(original.replace(old, new))
        result = self.run_contract()
        self.assertNotEqual(result.returncode, 0, result.stdout + result.stderr)

    def test_complete_contract_passes(self):
        result = self.run_contract()
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)

    def test_missing_shard(self):
        self.mutate(".github/workflows/native-validation-windows.yml", "shard: [1, 2, 3]", "shard: [1, 2]")

    def test_missing_aggregate(self):
        target = self.root / ".github/workflows/native-validation-windows.yml"
        target.write_text(target.read_text().split("  windows-native-coverage:")[0])
        self.assertNotEqual(self.run_contract().returncode, 0)

    def test_timeout_increase(self):
        self.mutate(".github/workflows/native-validation-windows.yml", "timeout-minutes: 120", "timeout-minutes: 240")

    def test_build_tree_upload(self):
        self.mutate(".github/workflows/native-validation-windows.yml", "path: artifacts/windows-native-validation-metrics/", "path: build/")

    def test_masked_failure(self):
        self.mutate(".github/workflows/native-validation-windows.yml", "      - name: Run shared native validation",
                    "      - name: Run shared native validation\n        continue-on-error: true")

    def test_wrong_default(self):
        self.mutate(".github/actions/native-validation/action.yml", "default: '1'", "default: '3'")


if __name__ == "__main__":
    unittest.main()
