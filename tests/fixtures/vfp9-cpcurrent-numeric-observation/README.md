# Installed VFP9 CPCURRENT observations

Recovered 2026-10-05 from installed Visual FoxPro 09.00.0000.7423 under Wine
with `/home/rich/bin/vfp9-probe cpcurrent.prg --result cpcurrent.out`.
The complete 32-row table retains Numeric, Currency, Logical, Character and
NULL cases, plus omitted/host/OEM controls. Code-page numbers are host-dependent;
this installation returned 1252, 1252 and 437 for those three controls.

Only explicit Numeric selectors 1 and 2 succeeded in this table. All tested
fractions, out-of-int32 values, huge finite values and both infinities raised
error 11, without wrapping to an accepted query. NaN and Copperfin's extended
int64/uint64 values are derived checked-admission cases, not native VFP9 claims.

`RQ-CF-PRG-CPCURRENT-NUMERIC-001` under #5611/#6776 changes conversion safety
only: both settings require finite, integral signed-32-bit Numeric selectors;
exact integer operands are checked without a floating round trip. Rejection
precedes configured-code-page reads. Other coercions retain their previous
rounding behind a representability check; no VFP9 quirk was recovered that
requires a mode difference for this conversion boundary.

The same probe found independent query-domain/type discrepancies, recorded
against main `8101c2838e2ad2adc55a833ef5c2f032b947d52a` as #6968. Native
explicit zero, unknown integral selectors, Currency fractions, Logical,
Character and NULL all failed here. The bounded conversion slice deliberately
does not recover those contracts: existing explicit-zero configured lookup,
unknown-integral host fallback and other coercions remain. Their preservation
tests are regression guards, not a native-parity claim. Omitted queries and
startup CODEPAGE behavior are unchanged. No full CPCURRENT parity is claimed.

The old conversion reports 86 semantic assertions failing in the focused
numeric suite. Verification covers 80 direct boundary calls and six valid
query controls across both settings, plus 16 catchable PRG rows. Adjacent
signed-32-bit bounds, exact integers, NaN, infinities and configured-state-read
counts are included. Numeric, locale/code-page and string/math suites pass
normally and under Clang ASan/UBSan/float-cast-overflow. No sanitizer report is
claimed for the old uninstrumented library `llround` call, and floating-status
flags are not a portable oracle under non-strict FP compilation.

No VM was started or modified; the shared probe runner was not changed.
