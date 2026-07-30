# Recovered Requirements Traceability

## Purpose

This document is the durable low-level requirement (LLR) to code to test matrix
for requirements recovered after implementation. It implements the Requirements
Recovery Principle in `docs/01-product-charter.md` without treating Copperfin's
existing behavior as its own requirement source.

Allowed requirement evidence is limited to observed behavior from a real,
installed VFP9 environment, shipped Microsoft/VFP documentation, and registered
known-bug or crash exceptions in `docs/27-known-vfp9-bug-exceptions.md`.
Decompiled or disassembled VFP binaries are prohibited inputs under
`docs/07-clean-room-rules.md`.

## Status Values

- `recovered`: the requirement has allowed source evidence and mapped code and
  test evidence.
- `gap`: allowed source evidence exists, but code or test evidence is missing or
  contradicts the requirement. Track implementation in a separate prompt-sized
  issue.
- `exception`: the behavior is intentionally different and has an applied
  `KBX-*` entry in `docs/27-known-vfp9-bug-exceptions.md`.

## Traceability Matrix

| LLR ID | Recovered low-level requirement | Allowed source evidence | Code | Tests | Verification | Status | Issue |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `LLR-VFP-NUM-001` | PRG numeric source and calculation text shall use a period as the decimal point independently of the host C/C++ locale or the VFP display-point setting. Comma-decimal text shall not be silently accepted as the same numeric literal. Machine numeric text routed back into parser/runtime contracts shall remain period-decimal and ungrouped. | Shipped VFP9 SP2 Help: [Numeric Data Type](https://vfphelp.com/vfp9/html/f0945c58-08e4-46fc-a15b-e1714a064d91.htm) defines numeric values using digits, an optional sign, and a decimal point. [SET POINT Command](https://www.vfphelp.com/help/html/ab6ea03e-d7f8-4ddb-b2e2-56755efd8857.htm) states that `SET POINT` changes display but calculations must use a period. | `include/copperfin/platform/invariant_numeric.h`; `src/platform/invariant_numeric.cpp`; `src/runtime/prg_engine_expression.inl` (`parse_number`) | `tests/test_prg_engine_parser_classes.cpp` (`test_invariant_numeric_parser_preserves_vfp_decimal_contract`, comma-decimal preprocessor locale test); `tests/test_prg_engine_data_io_save_restore.cpp` (`test_restore_from_parses_numeric_values_invariantly`) | macOS current-head `test_prg_engine_parser_classes` and `test_prg_engine_data_io` under default, `C`, `pt_BR.UTF-8`, and `de_DE.UTF-8`; Linux read-only mapping review required before closure | `recovered` | `#4896` |

## LLR-VFP-NUM-001 Evidence Boundary

The shipped help directly establishes two facts: VFP numeric/calculation syntax
uses a decimal point, and changing the displayed point does not change the
period required in calculations. The host-locale isolation clause is the
portable implementation consequence of those facts: Copperfin must not let the
embedding C/C++ locale reinterpret the VFP token grammar.

This row does **not** require displayed numeric text to ignore `SET POINT`,
`SET SYSFORMATS`, or other VFP formatting state. It also does not claim that all
string-to-number functions share source-literal syntax. Those surfaces require
separate recovered requirements and evidence.

The shared double parser requires full input consumption, rejects leading
whitespace, malformed signs, comma decimals, trailing text, overflow, and
nonfinite values unless a consuming binary-field contract explicitly opts in.
On Apple libc++, where floating-point `std::from_chars` may be unavailable, the
fallback stream is explicitly imbued with `std::locale::classic()` and uses
`std::noskipws`, preserving the same invariant contract.
