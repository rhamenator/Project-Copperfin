# Plain decimal literal producer: pre-code requirement map

RQ-CF-PRG-NUMERIC-LITERAL-METADATA-001 derives from the direct owner Numeric
compatibility instruction, parent RQ-CF-PRG-NUMERIC-METADATA-001 and #5611/#6776.
The first bounded producer is a digit-leading, unsigned plain decimal lexeme
with a point and at least one digit on each side, at most 255 digits total
(256 source bytes including the point). For native type N in this recovered
domain, direct metadata is display width min(40,lexeme length), decimal places
min(18,fractional digits). Leading/trailing zero spelling is retained in those
counts. Preserve the existing invariant-parser double without rounding it to
the metadata's scale. Apply the same ordinary Numeric semantics in both modes;
this is not the #6776 out-of-range-quirk conversion switch.

Allowed evidence: the repeated native144 grid and additional31-case direct/
assigned boundary pair (plain width40/41/42/43/102/255/256, scale17/18/19 and
long leading-zero/integer portions). Type-I trailing-point literals1./0./0001.
are controls, not type-N decimal metadata. Parentheses/whitespace do not add
width because they are not part of the lexeme. Unary plus returns the operand;
unary-minus metadata is a required follow-on, not inferred here.

The initial boundary probe aborted at the302-byte EVALUATE input with native36;
retain source/partial/status as unqualified for complete-grid acceptance. The
corrected probe records each outcome and completes31 cases; its257-byte case
returns native36 while256-byte controls are admitted. This does not prove a
universal literal/compiler or EVALUATE limit. Therefore this first metadata
producer does not claim native equivalence for longer source: leave metadata
unspecified there without changing Copperfin's existing parse/acceptance rule.
That domain is a mandatory later recovery/extension boundary, not silently
declared native-compatible. No new syntax/error behavior is introduced.

Architecture: add a nonallocating, bounded pure lexical classifier returning
optional PrgNumericMetadata. Validate ASCII digit/point shape, reject unsupported
shapes from metadata inference (not from runtime syntax), bound lengths before
casts, and cap attributes before conversion. Initially test it UNUSED. Only
after frozen baseline public tests fail for absent metadata wire it at
parse_number's successful invariant-parse return, retaining the original number
value. Do not alter tokenizer advancement, errors, suppress-evaluation behavior,
generic constructors, arithmetic/functions, assignment widths, fields, formatting,
ListItem or other list/combo consumers. No guessed metadata for integers,
exponents, signs or host values; other producer migrations stay open.

VR-5611-NUMERIC-LITERAL-METADATA-001: independent literal expected widths/scales
from native observations, pure helper positive/boundary/unsupported/long/NUL
vectors, and actual public paused watch evaluation under both Numeric modes
and DECIMALS2/14. Assert exact number bits where native-backed vectors exist,
NULL/kind stability, unchanged explicit formatting/arithmetic results and
state preservation. Establish original public failure before wiring without
weakening assertions; rerun fixed GNU and sanitizer evidence plus relevant
existing Numeric/display/engine tests. Pure helper tests are not public-runtime
parity. Keep whole-suite/hosted evidence separate and require exact-head review,
strict checks and conversation resolution before merge.

DQ-DV-5611-NUMERIC-LITERAL-METADATA-001: procedural delta is native recovery ->
UNUSED classifier -> independent original-fail regression -> single producer
wiring -> focused/broader verification. Documentation misuse severity MEDIUM
because incorrectly treating metadata as rounded value or using it as a buffer
size could affect data integrity. Walkthrough must show 19 fractional digits
retain a nonzero double while metadata reports18, width caps40, and unknown
domains remain explicitly unfinished. Native raw provenance, original-fail and
fixed results are implementing-agent development evidence, not independent-human
or release qualification. Rollback removes producer hookup/helper/test registration
and coherently rebuilds; preserve the owner objective and native evidence.
HZ-data-corruption-01/HZ-runtime-crash-01/HZ-doc-command-01 status/severity stay
unchanged; no allocations/casts derived from unchecked metadata, general
Numeric/list family, #7189 or release closure.
