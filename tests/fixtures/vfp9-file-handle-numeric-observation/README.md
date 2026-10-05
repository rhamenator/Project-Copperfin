# Installed VFP9 file-handle numeric observations

Recovered on 2026-10-05 with installed VFP9 SP2 under Wine using
`/home/rich/bin/vfp9-probe file-handle-numeric.prg`. The retained 380 rows
cover 38 arguments across ten names. Every call gets a freshly opened unique
six-byte temporary file positioned at byte 3; writes use one byte and FCHSIZE
uses size 3, so no probe creates a large file. Each file is closed and erased.

FCLOSE, FREAD, FWRITE, FGETS, FPUTS, FSEEK, FEOF, FFLUSH and FCHSIZE share the
observed handle conversion: Numeric fractions truncate toward zero, signed
low-32-bit wrapping can select the live handle, and Numeric infinities and
values outside int64 convert to an invalid handle. Currency fractions truncate
but nonzero sub-unit values and whole units outside [-4294967296, 4294967295]
are invalid handles. The negative double-wrap alias distinguishes Currency's
range rejection from Numeric wrapping. Logical, numeric Character, NULL and
uninitialized values are invalid handles, not coerced numeric zero/one or
parsed live handles. All invalid-handle outcomes set FERROR 6 without a
catchable argument error.

FTELL is a Copperfin extension: installed VFP9 attempts to resolve it as an
unavailable program (error 1). Its shared conversion policy, NaN rejection,
and exact int64/uint64 behavior above double precision are derived from the
owner's #5611/#6776 safety policy, not VFP9 claims.

RQ-CF-PRG-FILE-HANDLE-NUMERIC-001 derives default COPPERFIN admission of exact
signed-32-bit numeric handles; fractions, non-finite and out-of-range numbers
raise localized catchable error 11 before lookup or stream mutation. Explicit
VFP9 enables only the recovered conversion quirks. Both modes retain the
consistent invalid-type FERROR 6 rule without string/Logical coercion. Native
and verified-byte callers share the validator.

This slice changes conversion only. Existing return-contract gaps remain:
FCLOSE/FFLUSH Numeric instead of Logical results (#5914/#5913), FCHSIZE's zero
instead of resulting size (#5912), FPUTS LF/count (#5887/#5911), invalid-handle
FWRITE/FPUTS/FSEEK -1 instead of zero (#6959), and failed-seek returns (#6956).
Portable tests pin these current results separately from admission and verify
that rejected handles cannot close, write, resize or reposition the target.
