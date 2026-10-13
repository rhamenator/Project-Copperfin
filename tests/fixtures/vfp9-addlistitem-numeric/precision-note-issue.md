## Problem at exact current main

At `06d7b54107b5b22b20300108e1e19ef01357fb59`, the precision paragraph in
`tests/fixtures/vfp9-removelistitem-numeric/README.md:60` says native expressions
`1-2^(-52)` and `1-2^(-53)` evaluate as exactly1, based on native equality/
less-than comparison flags. Fresh installed-VFP9 binary serialization shows
their Double payloads are below1 instead. Comparison flags are not proof of
binary identity.

This emerged while calibrating independent measurement for the active
#5611/#6776 AddListItem numeric-argument recovery, before helpers/assertions/
configuration/production edits. It is a focused evidence-attribution/doc gap,
not authority to change earlier production conversions or expand the slice.
No agent-approved label or implementation admission is requested/applied.

## Independent primary-source measurement

Installed Visual FoxPro09.00.0000.7423; shipped BINTOC help completely read
564lines, SHA3708551c6edd118a2c10fa2afc3392ac2dfd19c325ba5d064e25639ecc17fc80,
documents the B Double representation and additive R/S flags. Fresh paired
six-expression probe serializes with BINTOC(v,'BRS'), emits each byte in hex,
then records the native comparison flags. Both runs finish0/cmp0/DONE6,
full8-line rawSHA9b82d2391924146e6e8f053a2eb16c849e6de3a31b9d1d271f1145bb08380859.

- Decimal source0.9999999999999999 serializes `3FF0000000000000` (exact1).
- Source1-2^(-52) serializes `3FEFFFFFFFFFFFFE` (below1).
- Source1-2^(-53) serializes `3FEFFFFFFFFFFFFF` (below1).
- All three nevertheless report native `(v=1)` true and `(v<1)` false.
- Source32768-2^(-38) serializes `40DFFFFFFFFFFFFF` (below32768) while
  `(v=32768)` is true and `(v<32768)` false.

For an additional consumer control, fresh paired70-call AddListItem probes
with the same binary-neighbor sources place BINTOC capture before versus
after direct invocation. Both variants produce identical full rawSHA
b03c7183da3a863ea5a8bcc71178c17d283af03a3075ac42b2995f0545061298,
and direct1-2^(-52)/1-2^(-53) invocation rejects11 on both controls. Separate
earlier instrumented boundary calls with comparisons immediately before the
consumer behave differently near1. The mechanism for that call-context
difference is NOT diagnosed; do not call it a parser, FPU or production fix.

Full source/raw/status/hash retained under the dedicated AddListItem worktree
build/addlistitem-native, to be retained in its recovery fixture. No Copperfin
implementation/helper generated these observations; no VM/credential/setup/
decompilation action occurred.

## Focused future correction

Correct the named precision attribution without rewriting historical raw
evidence or claiming a previously failed test passed. Separate native
comparison semantics, binary serialization, call-context effects and actual
consumer acceptance. Retain true decimal-source rounding and named-parent
derived binary-boundary tests; do not infer earlier helper/code changes from
this measurement. Follow requirements/doc traceability and appropriate review
for any admitted correction. This issue remains separate from the active
numeric-admission production slice.
