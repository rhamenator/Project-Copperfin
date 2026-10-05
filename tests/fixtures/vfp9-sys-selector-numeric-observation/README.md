# Installed VFP9 SYS Numeric-selector observations

Recovered 2026-10-05 from installed Visual FoxPro 09.00.0000.7423 under Wine
with `/home/rich/bin/vfp9-probe sys-selector.prg --result sys-selector.out`.
The retained 24-row table normalizes host-dependent identity/path data instead
of committing machine/user names. `matches-5` means the current-drive result;
`matches-7` means empty text (BASE7-LENGTH is zero), not an OS-name result.

Numeric fractions truncate toward zero. Oversized positive Numeric values do
not wrap to a recognized selector: 4294967301 does not select SYS(5).
Negative Numeric values convert through signed low-32 bits, then clamp negative
results to zero: -4294967291 selects SYS(5); negative huge/infinite values and
-2147483648 select SYS(0). NaN and extended int64/uint64 cannot be recovered
from this native source table; their checked conversion is derived safety
policy under #5611/#6776, not a native VFP9 claim.

This bounded slice governs Numeric and Copperfin exact-integer primary
selectors. Other coercions retain their existing rounding, but now validate
the rounded value before integral conversion. Optional SYS parameters and
individual operation/return behavior are unchanged. In particular, Copperfin
still returns `"0"` for unimplemented selectors instead of native empty text,
and native SYS(0)/negative-identity parity remains outside this slice (#6945).
The separate SYS(7)/unknown-selector return discrepancy is recorded as #6966.
No selector operation in the retained probe changes system configuration.

Exploratory Currency sub-unit/out-of-int32 calls did not complete under Wine,
even with an extended runner deadline, and are not compatibility evidence.
The final retained source deliberately excludes Currency/type exploration;
their SYS compatibility needs a separate bounded native investigation. No VM
was started or modified. The shared runner was not changed.

## Verification-model correction

Hosted macOS run `37315149442`, job `111779902353`, at `2988bccc0` passed
every SYS result/error/callback assertion but failed 13 added FE_INVALID
assertions. Those assertions assumed strict floating-point exception
semantics that the Release build does not enable. Clang's default exception
behavior is `ignore`, with FENV_ACCESS off; floating-point flags are not a
portable oracle for source-level checked-conversion safety in that model.
See [Clang's floating-point controls](https://clang.llvm.org/docs/UsersManual.html#controlling-floating-point-behavior).

A reduced checked-cast function compiled locally with Clang 21.1.8 using
`--target=aarch64-apple-darwin -O3 -x c++ -S` emits `fcvtzs` before `csel`
for the guarded result. Adding `-ffp-exception-behavior=strict` keeps the
conversion after the rejection branches. This is compiler-diagnostic
evidence, not a new VFP compatibility requirement. Portable regression checks
therefore assert results, catchable errors and operation callbacks; sanitizer
builds check source-level cast safety. With floating-status checks removed,
the old SYS dispatch still fails 26 semantic assertions.
