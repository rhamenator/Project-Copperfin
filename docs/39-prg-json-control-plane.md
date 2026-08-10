# PRG JSON Control-Plane Facade

## Purpose

Copperfin keeps FP/VFP source in charge of orchestration. External-language
source remains in separately built artifacts, and a foreign worker must return
immutable completion data instead of entering mutable runtime state. PRG code
therefore needs a small, deterministic way to inspect structured result
payloads without embedding another language or depending on a managed runtime.

This first native JSON parity surface provides that control-plane primitive. It
does not admit, select, launch, or authorize an external artifact.

## PRG Functions

`CFJSONVALID(cDocument)` returns a logical value. It is true only when the
entire UTF-8 character value is one valid JSON value within the fixed runtime
limits.

`CFJSONTYPE(cDocument [, cPointer])` returns one invariant lower-case token:

- `object`, `array`, `string`, `number`, `boolean`, or `null` for a selected
  value;
- `missing` when a valid document does not contain the requested selection;
- `invalid` when the document or JSON Pointer is invalid.

`CFJSONGET(cDocument [, cPointer [, uFallback]])` returns the selected value.
For a JSON string it returns decoded UTF-8 character data. For every other JSON
kind it returns the exact selected JSON bytes, excluding only surrounding
whitespace. This deliberately preserves large integers and decimal/exponent
spelling instead of coercing them through binary floating point. Invalid or
missing selections return `uFallback` unchanged when supplied, otherwise an
empty character value.

The optional selector follows RFC 6901 JSON Pointer syntax. The empty pointer
selects the root, `/items/0` selects the first array item, `~1` represents `/`
inside a member name, and `~0` represents `~`. Array indexes are zero-based and
canonical: leading-zero forms such as `01` are not accepted.

Example:

```foxpro
cPayload = '{"status":"ready","rows":[{"id":9007199254740993}]}'

IF CFJSONVALID(cPayload) AND CFJSONTYPE(cPayload, '/status') == 'string'
    cStatus = CFJSONGET(cPayload, '/status', 'unknown')
    cExactId = CFJSONGET(cPayload, '/rows/0/id', '')
ENDIF
```

`cExactId` remains the character value `9007199254740993`; Copperfin does not
silently round it.

## Bounds And Failure Policy

The reusable native parser defaults to 1 MiB per document, 64 nested container
levels, and 65,536 JSON values. Callers may lower any bound, but cannot raise
the hard ceilings above 16 MiB, 64 levels, or 65,536 values. The value-count
ceiling bounds memory amplification from dense arrays and objects. The PRG
facade uses the defaults.

The parser accepts one complete JSON value and rejects:

- malformed or non-shortest UTF-8, surrogate errors, and invalid escapes;
- duplicate object member names after escape decoding;
- trailing commas or trailing non-whitespace bytes;
- malformed numbers;
- malformed JSON Pointer escapes and non-canonical array indexes;
- byte, nesting, or value-count limit violations.

No filesystem, environment, child-process, network, sample, locale, or secret
access occurs. Machine type tokens are intentionally not localized. Document
contents are not written to runtime audit events.

## Concurrency And Interop Boundary

The parser owns only call-local state. It does not mutate a data session and is
safe to use while PRG code polls a supervised task. The portable artifact
invocation adapter now admits a JSON result, but it is not yet submitted through
the PRG task registry. A future PRG-facing route must publish that result into
the existing immutable task-completion record; PRG can then use
`CFTASKSTATUS()`, `CFTASKRESULT()`, `CFTASKOUTPUT()`, and these JSON helpers to
decide what happens next. The external worker still cannot call back into
mutable Copperfin runtime state.

In-process Windows interop, if later approved, must preserve this same PRG
contract. Out-of-process adapters remain the preferred isolation boundary.

## Verification

`test_platform_models` covers strict parsing, decoded and raw forms, escaped
pointer tokens, array traversal, exact large-number bytes, duplicate-key
rejection, malformed grammar and UTF-8, and byte/depth bounds.

`test_prg_engine_json_facade` executes all three functions through a real PRG
runtime session and covers valid, missing, fallback, malformed, nested, decoded
string, and exact-number behavior. Its isolation audit records a portable,
parallel-safe test with a unique test-owned temporary directory and no external
I/O beyond that fixture.

Both focused targets pass with GCC and with Clang 21 ASan/UBSan. The broader
runtime-surface regression, native-isolation contract, and document-install
contract pass under GCC, and focused Clang analyzer checks report no project
diagnostics. At exact candidate head `b507572d1`, Linux Native run
`31312190207` and macOS Native run `31312191221` pass `327/327`, macOS also
passes the four-locale SET POINT matrix at `8/8`, and Windows Native run
`31312192368` passes `326/326`. Both focused targets pass on all three hosted
platforms, and all eight protected checks are green. Product and documentation
changes end at `0176d0531`; the final candidate delta is test-only coverage
proving that a numeric fallback remains numeric.

This is the first native JSON-helper slice of the broader parity-facade work.
Regex, safe HTTP, key lifecycle, external artifact admission, dispatch,
and automatic route promotion remain separate acceptance criteria.
