# PRG Payload Integrity And Base64 Facade

## Purpose

PRG-controlled interop needs a portable way to identify immutable result bytes
and carry arbitrary bytes through text-only envelopes. Copperfin provides that
small native surface without embedding foreign-language source or granting an
external runtime access to mutable FP/VFP state.

These functions are Copperfin extensions, not VFP9 functions:

| Function | Result |
| --- | --- |
| `CFSHA256(cData [, uFallback])` | Lowercase 64-character SHA-256 digest of the exact input bytes. |
| `CFBASE64ENCODE(cData [, uFallback])` | Canonical padded RFC 4648 Base64 text using the standard `+` and `/` alphabet. |
| `CFBASE64DECODE(cText [, uFallback])` | Exact decoded bytes after strict canonical validation. |

On missing or non-character input, a bound violation, an internal hash failure,
or invalid Base64, the function returns `uFallback` unchanged when supplied,
including its PRG type. Otherwise it returns an empty character value. Numeric,
logical, currency, and other values are not silently stringified; callers must
choose their own explicit byte representation first.

```foxpro
cPayload = CHR(0) + 'Copperfin'
cDigest = CFSHA256(cPayload)
cTransport = CFBASE64ENCODE(cPayload)
cRestored = CFBASE64DECODE(cTransport, '')

IF cRestored == cPayload
    * PRG decides what happens next.
ENDIF
```

## Byte And Validation Contract

The functions operate on PRG character values as byte strings. They do not
perform Unicode normalization, code-page conversion, line-ending conversion,
or numeric coercion within a string. SHA-256 output and Base64 syntax are
locale-invariant machine text.

The decoder accepts only one canonical complete Base64 value. It rejects:

- whitespace and line wrapping;
- URL-safe `-` and `_` alphabet substitutions;
- incomplete quartets;
- padding outside the final quartet;
- misplaced or excessive `=` padding; and
- nonzero unused bits hidden below final padding.

Empty input is valid and maps to empty output. Decode failure is distinct from
successfully decoding an empty value through the optional typed fallback.

## Fixed Bounds

Hashing and encoding accept at most 1 MiB of payload bytes. Decoding accepts at
most 1,398,104 Base64 bytes—the canonical encoded size of a 1 MiB payload—and
also rejects any decoded result above 1 MiB. These ceilings are compiled into
the facade and cannot be raised by PRG code.

All work and storage are linear in the bounded input size. The implementation
uses the existing portable `cf_security` SHA-256 primitive and a call-local
Base64 codec. It performs no file, environment, network, child-process, sample,
secret-store, audit-stream, locale, data-session, or callback access.

## Security Boundaries And Nonclaims

SHA-256 identifies bytes; it does not authenticate who supplied them. Base64
is an encoding, not encryption. These helpers do not provide signatures,
message authentication, password hashing, key derivation, confidentiality,
secret management, file hashing, artifact admission, policy authorization, or
external execution. Release authenticity continues to require the separate
signed-artifact trust boundary.

A foreign worker still cannot call into Copperfin runtime state. PRG may apply
these helpers only to bytes already available through its controlled task or
payload boundary.

## Verification

`test_payload_crypto` covers standard SHA-256 and RFC 4648 vectors, binary
round trips, strict malformed and noncanonical rejection, exact-ceiling
round trips, and above-ceiling rejection. `test_prg_engine_payload_crypto_facade`
executes all three functions through a real PRG runtime session and proves
invariant digest spelling, canonical transport text, zero/high-byte retention,
typed fallbacks, and missing/non-character input behavior.

Both tests are portable, parallel-safe, network-free, and child-process-free.
The platform test touches no filesystem; the PRG test owns one unique temporary
directory for its source fixture. Both pass under GCC and Clang 21 ASan/UBSan.
The final-source GCC package, isolation, audit-stream, security-control,
focused, and broad runtime-surface set passes `7/7`; focused Clang analyzer
checks report no project diagnostic. At exact candidate head `08fe8dd98`,
Linux Native run `31321075915` and macOS Native run `31321075953` pass
`331/331`, the macOS four-locale SET POINT matrix passes `8/8`, and Windows
Native run `31321075911` passes `330/330`. Both focused targets pass in all
three native runs, and all eight candidate-head protected checks are green.
