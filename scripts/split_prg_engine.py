#!/usr/bin/env python3
"""
split_prg_engine.py
Extract logical groups of Impl methods from prg_engine.cpp into .inl files.

Each group is either:
  'anon'  - included inside the anonymous namespace (free functions)
  'impl'  - included inside the Impl struct body (method definitions)

The script:
1. Reads prg_engine.cpp
2. Writes each group's lines to src/runtime/<name>.inl
3. Replaces those lines in prg_engine.cpp with a single #include directive
4. Writes the modified prg_engine.cpp

Run from project root: python3 scripts/split_prg_engine.py
"""

import os
import shutil

SRC_DIR = "src/runtime"
TARGET = os.path.join(SRC_DIR, "prg_engine.cpp")

# (start_1based, end_1based_inclusive, filename, location)
GROUPS = [
    (252,  847,  "prg_engine_free_functions.inl", "anon"),
    (909,  1166, "prg_engine_session.inl",         "impl"),
    (1167, 1959, "prg_engine_cursor.inl",           "impl"),
    (1960, 2790, "prg_engine_records.inl",          "impl"),
    (2791, 3663, "prg_engine_aggregate.inl",        "impl"),
    (3665, 4271, "prg_engine_dll.inl",              "impl"),
    (4272, 4665, "prg_engine_sql.inl",              "impl"),
    (4670, 5112, "prg_engine_variables.inl",        "impl"),
    (5113, 5645, "prg_engine_arrays.inl",           "impl"),
    (5646, 6288, "prg_engine_flow.inl",             "impl"),
]

def main():
    # Backup original
    backup = TARGET + ".bak"
    shutil.copy2(TARGET, backup)
    print(f"Backed up to {backup}")

    with open(TARGET, "r", encoding="utf-8") as f:
        lines = f.readlines()

    total_lines = len(lines)
    print(f"Source file: {total_lines} lines")

    # Verify line count matches expectations
    # Groups reference 1-based line numbers; convert to 0-based indices
    for start, end, name, loc in GROUPS:
        if end > total_lines:
            print(f"ERROR: group {name} end={end} exceeds file length {total_lines}")
            return

    # Process groups in REVERSE order to preserve line numbers
    # (each replacement changes line count, so work from bottom up)
    groups_reversed = list(reversed(GROUPS))
    for start, end, name, loc in groups_reversed:
        s = start - 1  # 0-based inclusive
        e = end         # 0-based exclusive (line at index e is line end+1, 1-based)

        group_lines = lines[s:e]

        # Write the .inl file
        inl_path = os.path.join(SRC_DIR, name)
        header_comment = f"// {name}\n"
        if loc == "anon":
            header_comment += "// Free helper functions. Included inside anonymous namespace in prg_engine.cpp.\n"
        else:
            header_comment += "// PrgRuntimeSession::Impl method group. Included inside Impl struct in prg_engine.cpp.\n"
        header_comment += "// This file must not be compiled separately.\n\n"

        with open(inl_path, "w", encoding="utf-8") as f:
            f.write(header_comment)
            f.writelines(group_lines)
        inl_lines = len(group_lines)
        print(f"  Wrote {inl_lines:5d} lines -> {name}")

        # Replace the extracted lines with a single #include directive
        include_line = f'#include "{name}"\n'
        lines[s:e] = [include_line]

    with open(TARGET, "w", encoding="utf-8") as f:
        f.writelines(lines)

    new_total = len(lines)
    print(f"\nModified prg_engine.cpp: {new_total} lines (was {total_lines})")

    # Print summary
    print("\nNew .inl file sizes:")
    for _, _, name, _ in GROUPS:
        path = os.path.join(SRC_DIR, name)
        with open(path) as f:
            n = sum(1 for _ in f)
        print(f"  {name:45s}  {n:5d} lines")

if __name__ == "__main__":
    main()
