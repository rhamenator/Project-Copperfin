# VFP9 BIT fold arity evidence

`probe.ps1` ran through the installed Visual FoxPro 9 SP2
`VisualFoxPro.Application` COM server on `copperfin-vfp9-win11` on 2026-11-07.
It records the one-argument boundary for BITAND, BITOR and BITXOR, plus the
three retained multi-operand fractional folds selected for the portable
two-mode table. `run1.out` is the complete output.

- `probe.ps1` SHA-256:
  `f89d6f0260f505c6d95f2ab7d7e19718a578ae3924fb618f29fe50b6fce3f26b`
- `run1.out` SHA-256:
  `99f9a1daa192d9d712d8b11eaa34e80b54c7acc1d9e67cbc5bb7188d4d910f6b`

This evidence establishes only the listed arity errors and fractional fold
results. It does not establish mixed-type parity, the unresolved COPPERFIN
operand policy in #6873, or full #6874/#6875 acceptance.
