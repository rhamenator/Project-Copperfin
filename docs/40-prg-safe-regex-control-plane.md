# PRG Safe-Regex Control-Plane Facade

## Purpose

PRG remains responsible for orchestration when Copperfin eventually connects
approved external-language adapters. Immutable completion text sometimes needs
lightweight validation or extraction before PRG decides what happens next.
This native facade provides that primitive without embedding foreign source,
loading a managed runtime, or using a potentially unbounded backtracking regex
engine.

## PRG Functions

`CFREGEXVALID(cPattern)` returns true only when the pattern uses the supported
bounded subset.

`CFREGEXTEST(cText, cPattern [, lIgnoreAsciiCase])` returns true when the pattern
has a match. Invalid syntax or a bound violation returns false.

`CFREGEXFIND(cText, cPattern [, nStart [, lIgnoreAsciiCase]])` returns the
one-based byte position of the leftmost-longest match, or zero when there is no
match or the request is invalid. `nStart` is also a one-based byte position. An
empty match at the end is therefore `LEN(cText) + 1`.

`CFREGEXGET(cText, cPattern [, nStart [, lIgnoreAsciiCase [, uFallback]]])`
returns the exact matched bytes. A missing or invalid match returns `uFallback`
unchanged when supplied, including its PRG type; otherwise it returns an empty
character value.

Example:

```foxpro
cResult = 'state=READY; rows=9007199254740993'

IF CFREGEXTEST(cResult, 'state=ready', .T.)
    cRows = CFREGEXGET(cResult, 'rows=\d+', 1, .F., '')
ENDIF
```

`cRows` is `rows=9007199254740993`; it is never coerced through floating point.

## Supported Subset

Matching is byte-oriented, consistent with VFP-style character positions:

- literal bytes and backslash-escaped literal bytes;
- `.` for any one byte, including a line-ending byte;
- byte classes, ranges, and negated classes such as `[A-Z]` and `[^,]`;
- ASCII shorthands `\d`, `\w`, `\s` and their negated forms outside classes;
- `^` and `$` only as whole-input start/end anchors;
- greedy `?`, `*`, and `+` quantifiers.

Search is deterministic leftmost-longest. Optional case folding is ASCII-only,
locale-invariant, and applies correctly to negated classes. Bytes above ASCII
remain exact. UTF-8 text can be matched by exact UTF-8 literal bytes, but `.`
and returned positions operate on bytes rather than Unicode graphemes.

Grouping, alternation, captures, backreferences, lookaround, counted
repetition, lazy/possessive modifiers, inline options, and replacement APIs are
intentionally unsupported. Such patterns fail `CFREGEXVALID()` and the other
facades return their documented failure value. Applications needing those
features can later use a separately approved external capability without
changing this bounded native contract.

## Bounds And Execution Model

The hard ceilings are 64 KiB of input, 256 pattern bytes, and 512 compiled
states. Callers cannot raise them. The compiler creates a small Thompson-style
state machine, and matching propagates bounded state vectors once per input
byte. It performs no recursive or exponential backtracking; work is bounded by
input bytes times compiled states, and matching state lives on call-local heap
storage rather than growing the host stack.

No filesystem, environment, child-process, network, locale, sample, secret,
audit-stream, or mutable data-session access occurs. A foreign thread still
cannot call into Copperfin runtime state. PRG may apply these helpers only after
an immutable result has crossed the controlled supervision boundary.

## Verification

`test_platform_safe_regex` covers supported literals, anchors, classes,
shorthands, quantifiers, leftmost-longest selection, starting offsets,
ASCII-case behavior, malformed/unsupported syntax, all hard ceilings, and a
full-bound overlapping-repetition input that would be hazardous for a
backtracking engine.

`test_prg_engine_regex_facade` executes all four functions through a real PRG
runtime session and covers exact large-number text, one-based positions,
case-insensitive testing, typed fallbacks, invalid starts, unsupported syntax,
and terminal empty matches. Both tests are portable, parallel-safe, and have no
network or child-process access. They pass under GCC and under Clang 21 with
ASan/UBSan; focused Clang analyzer checks report no project diagnostic. The
broader runtime-surface, document-install, and native-isolation contracts pass
under GCC.

At exact candidate head `fca07640c`, Linux Native run `31316773955` and macOS
Native run `31316774878` pass `329/329`; macOS also passes both SET POINT
display targets under four locales (`8/8`). Windows Native run `31316775761`
passes `328/328`. Both focused regex targets pass on every native platform,
and all eight candidate-head protected checks are green.

This is one native facade under the broader parity pack. Safe HTTP, key
lifecycle, collection utilities, external artifact admission,
dispatch, and automatic route promotion remain separate acceptance criteria.
