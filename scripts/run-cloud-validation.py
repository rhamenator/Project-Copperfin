#!/usr/bin/env python3
"""Bounded, reproducible hosted native validation campaigns."""

import argparse
import json
import os
from pathlib import Path
import resource
import shutil
import subprocess
import sys
import time


ROOT = Path(__file__).resolve().parents[1]
TARGETS = {
    "stress": ["test_prg_engine_control_flow", "test_prg_engine_database_lifecycle",
               "test_prg_engine_runtime_surface_functions_buffering", "test_prg_engine_data_io"],
    "migration": ["test_cloud_migration_stress", "test_dbf_table", "test_staged_import_publish"],
    "sanitizer-pr": ["test_prg_engine_control_flow", "test_prg_engine_relations",
                     "test_prg_engine_database_lifecycle",
                     "test_prg_engine_runtime_surface_functions_buffering", "test_prg_engine_data_io",
                     "test_cloud_migration_stress", "test_dbf_table", "test_runtime_pipeline",
                     "test_package_launcher_inventory_trust"],
}


def run(command, log_path, env=None):
    start = time.monotonic()
    with log_path.open("w", encoding="utf-8", errors="replace") as output:
        result = subprocess.run(command, cwd=ROOT, env=env, stdout=output,
                                stderr=subprocess.STDOUT, check=False)
    elapsed = time.monotonic() - start
    print(f"{' '.join(map(str, command[:4]))}: exit={result.returncode} elapsed={elapsed:.1f}s")
    if result.returncode:
        print(f"Failure log: {log_path}", file=sys.stderr)
        print("".join(log_path.read_text(errors="replace").splitlines(keepends=True)[-100:]),
              file=sys.stderr)
        raise SystemExit(result.returncode)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("lane", choices=["sanitizer", "fuzz", "stress", "migration"])
    parser.add_argument("--profile", choices=["pr", "nightly"], default="pr")
    parser.add_argument("--seconds", type=int, default=0)
    parser.add_argument("--iterations", type=int, default=0)
    parser.add_argument("--rows", type=int, default=0)
    parser.add_argument("--seed", type=int, default=1)
    args = parser.parse_args()
    seconds = args.seconds or (600 if args.profile == "nightly" else 30)
    iterations = args.iterations or (12 if args.profile == "nightly" else 2)
    rows = args.rows or (100000 if args.profile == "nightly" else 1000)
    if not 1 <= seconds <= 1800 or not 1 <= iterations <= 30 or not 1 <= rows <= 300000 or not 1 <= args.seed <= 2147483647:
        parser.error("seconds=1..1800, iterations=1..30, rows=1..300000, seed=1..2147483647")

    evidence = ROOT / "artifacts" / "cloud-validation" / args.lane
    evidence.mkdir(parents=True, exist_ok=True)
    build = ROOT / "build-cloud" / args.lane
    configuration = ["cmake", "-S", str(ROOT), "-B", str(build),
                     "-DCMAKE_BUILD_TYPE=Debug", "-DCOPPERFIN_BUILD_TESTS=ON"]
    if args.lane in ("sanitizer", "fuzz"):
        configuration.extend(["-DCMAKE_C_COMPILER=clang", "-DCMAKE_CXX_COMPILER=clang++"])
    if args.lane == "sanitizer":
        flags = "-O1 -g -fno-omit-frame-pointer -fsanitize=address,undefined -fno-sanitize-recover=all"
        configuration.extend([f"-DCMAKE_C_FLAGS={flags}", f"-DCMAKE_CXX_FLAGS={flags}",
                              "-DCMAKE_EXE_LINKER_FLAGS=-fsanitize=address,undefined",
                              "-DCMAKE_SHARED_LINKER_FLAGS=-fsanitize=address,undefined"])
    if args.lane == "fuzz":
        configuration.append("-DCOPPERFIN_BUILD_FUZZ_TESTS=ON")
    run(configuration, evidence / "configure.log")
    targets = (["fuzz_dbf_header"] if args.lane == "fuzz" else
               [] if args.lane == "sanitizer" and args.profile == "nightly" else
               TARGETS["sanitizer-pr" if args.lane == "sanitizer" else args.lane])
    build_command = ["cmake", "--build", str(build), "--parallel", "2"]
    if targets:
        build_command.extend(["--target", *targets])
    run(build_command, evidence / "build.log")

    metadata = {"lane": args.lane, "profile": args.profile, "seed": args.seed,
                "seconds": seconds, "iterations": iterations, "rows": rows,
                "revision": os.getenv("GITHUB_SHA", "local")}
    (evidence / "metadata.json").write_text(json.dumps(metadata, indent=2) + "\n")
    print("Campaign: " + json.dumps(metadata, sort_keys=True))
    environment = os.environ.copy()
    environment["ASAN_OPTIONS"] = "detect_leaks=1:halt_on_error=1:abort_on_error=1"
    environment["UBSAN_OPTIONS"] = "halt_on_error=1:print_stacktrace=1"
    if args.lane == "fuzz":
        corpus = build / "fuzz-corpus"
        shutil.rmtree(corpus, ignore_errors=True)
        shutil.copytree(ROOT / "tests/fuzz/corpus/dbf_header", corpus)
        run([str(build / "tests/fuzz_dbf_header"), f"-max_total_time={seconds}",
             f"-seed={args.seed}", f"-artifact_prefix={evidence}/", str(corpus)],
            evidence / "campaign.log", environment)
        # Coverage additions are not useful evidence after a passing run.
        shutil.rmtree(corpus)
    elif args.lane == "migration":
        run(["/usr/bin/time", "-f", "max_rss_kib=%M elapsed_seconds=%e",
             "-o", str(evidence / "migration-metrics.txt"),
             str(build / "tests/test_cloud_migration_stress"), str(args.seed), str(rows)],
            evidence / "large-migration.log", environment)
        tests = "^(test_dbf_table|test_staged_import_publish)$"
        run(["ctest", "--test-dir", str(build), "--no-tests=error", "--output-on-failure", "--timeout", "180",
             "-R", tests], evidence / "ctest.log", environment)
    else:
        command = ["ctest", "--test-dir", str(build), "--no-tests=error", "--output-on-failure",
                   "--timeout", "180", "--parallel", "2"]
        if args.lane == "stress":
            command.extend(["--repeat", f"until-fail:{iterations}", "-R",
                            "^(test_prg_engine_control_flow|test_prg_engine_database_lifecycle|"
                            "test_prg_engine_runtime_surface_functions_buffering|test_prg_engine_data_io)$"])
        elif args.profile == "pr":
            command.extend(["-R", "^(test_prg_engine_control_flow|test_prg_engine_relations|"
                            "test_prg_engine_database_lifecycle|"
                            "test_prg_engine_runtime_surface_functions_buffering|test_prg_engine_data_io|"
                            "test_cloud_migration_stress|test_dbf_table|test_runtime_pipeline|"
                            "test_package_launcher_inventory_trust|test_native_test_isolation_contract)$"])
        run(command, evidence / "ctest.log", environment)
    metadata["max_rss_kib"] = resource.getrusage(resource.RUSAGE_CHILDREN).ru_maxrss
    (evidence / "metadata.json").write_text(json.dumps(metadata, indent=2) + "\n")
    print(f"{args.lane} passed; evidence={evidence} max_rss_kib={metadata['max_rss_kib']}")


if __name__ == "__main__":
    main()
