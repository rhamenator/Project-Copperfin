- 2026-10-08: FV/PV now reject unsupported fourth/fifth/later arguments with
  localized catchable error 1230 before numeric coercion, eliminating both
  unsafe optional payment-timing llround-to-int paths under #5611/#6776 and
  the admitted #5878 arity contract. Native VFP9 probes, numeric/floating-
  exception/state regressions and recovered requirements distinguish this
  bounded fix from the still-unfinished three-argument order/sign/domain
  corrections in #5878/#6944 and future non-conflicting financial extensions.
