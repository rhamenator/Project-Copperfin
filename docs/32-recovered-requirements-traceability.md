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
| `LLR-VFP-NUM-001` | PRG numeric source and calculation text shall use a period as the decimal point independently of the host C/C++ locale or the VFP display-point setting. Comma-decimal text shall not be silently accepted as the same numeric literal. Machine numeric text routed back into parser/runtime contracts shall remain period-decimal and ungrouped. | Shipped VFP9 SP2 Help: [Numeric Data Type](https://vfphelp.com/vfp9/html/f0945c58-08e4-46fc-a15b-e1714a064d91.htm) defines numeric values using digits, an optional sign, and a decimal point. [SET POINT Command](https://www.vfphelp.com/help/html/ab6ea03e-d7f8-4ddb-b2e2-56755efd8857.htm) states that `SET POINT` changes display but calculations must use a period. | `include/copperfin/platform/invariant_numeric.h`; `src/platform/invariant_numeric.cpp`; `src/runtime/prg_engine_expression.inl` (`parse_number`) | `tests/test_prg_engine_parser_classes.cpp` (`test_invariant_numeric_parser_preserves_vfp_decimal_contract`, comma-decimal preprocessor locale test); `tests/test_prg_engine_data_io_save_restore.cpp` (`test_restore_from_parses_numeric_values_invariantly`) | macOS: both CTest targets under `C`, `en_US.UTF-8`, `pt_BR.UTF-8`, and `de_DE.UTF-8` (8/8). Linux seq1419: source/evidence mapping pass plus both targets under default, `C`, and `en_US.utf8` (6/6); `pt_BR`/`de_DE` were not installed and macOS supplies those executions. | `recovered` | closed `#4896` |
| `LLR-VFP-NUM-002` | `SET POINT` shall control the decimal-point character used to display numeric and Currency expressions. Its default shall be period; argument-free `SET POINT TO` shall reset period; and the setting shall be scoped to the current data session. Display punctuation shall not change calculation syntax or invariant machine-readable Currency text. | Mounted shipped VFP9 `dv_foxhelp.chm` (10,870,662 bytes; SHA-256 `abaa86e7623bb00e8bd9323cf2d8e162013598e35d9492557a3ddd1c2cf13e79`), topic `html/ab6ea03e-d7f8-4ddb-b2e2-56755efd8857.htm`, title `SET POINT Command`. The topic states the numeric/Currency display scope, period default and reset, calculation-period boundary, and current-data-session scope. | `src/runtime/prg_engine_dispatch.inl` (`SET POINT`, current-session state, runtime print); `src/runtime/prg_engine_string_functions.cpp` (`format_value_for_display`); `src/runtime/prg_engine_string_function_helpers.inl` (`apply_numeric_picture_symbols`); `src/runtime/prg_engine_helpers.cpp` (`value_as_string` exact Currency text) | `tests/test_prg_engine_string_math_functions.cpp` (direct Currency display, invariant stringification, no-picture `TRANSFORM()`, default/explicit/reset/session state); `tests/test_prg_engine_control_flow_control_flow_basics.cpp` (numeric and Currency runtime print) | Linux: `test_prg_engine_control_flow` and `test_prg_engine_string_math_functions` pass under the default locale and explicit `C` (4/4 executions). Exact implementation-head Windows Native Validation run `31275114419` passed 322/322, including both mapped targets. | `recovered` | open `#4913`; implementation child `#4914` |

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

Independent Linux review at channel sequence 1419 fetched and checked both
shipped-help pages, verified every mapped code and test location, and ran the
actual `test_prg_engine_parser_classes` and `test_prg_engine_data_io` targets.
The latter is the CMake target containing
`test_prg_engine_data_io_save_restore.cpp`; the source shard is not a standalone
test target. The Linux host's missing `pt_BR` and `de_DE` locales are disclosed
rather than inferred as executions. Linux source review confirmed the mapped
comma-decimal parser test constructs its locale facet in process, while the
macOS matrix supplies both installed locale runs.

## LLR-VFP-NUM-002 Evidence Boundary

The requirement text is recovered from the mounted shipped CHM, not from
Copperfin behavior. No binary was decompiled or disassembled. A temporary
current-head probe was used only to compare the recovered requirement with the
implementation: it produced `1.234,5000` for a numeric value but invariant
`1234.5000` for Currency under comma `POINT` and period `SEPARATOR`, proving a
display-path gap before #4914 changed product code.

The correction formats `value_as_string()` output for Currency through the
existing display-symbol seam. That source text is produced directly from the
signed scaled `int64` magnitude and four decimal digits, so display formatting
does not convert through `double`. The same seam serves runtime print and
no-picture `TRANSFORM()`. It deliberately does not add the `SET CURRENCY`
symbol: symbol presence and position remain picture/output-surface behavior,
outside this recovered `SET POINT` requirement.

`value_as_string(Currency)`, calculation parsing, SAVE/RESTORE, manifests,
JSON, and other machine-readable paths remain invariant period-decimal under
`LLR-VFP-NUM-001`. The focused regressions separately prove default period,
explicit punctuation, argument-free reset, per-data-session isolation, exact
four-place Currency digits, and unchanged invariant stringification.
