- 2026-10-07: JTOD/JTOT now check finite Numeric and exact signed/unsigned
  integer admission before narrowing Julian days, rejecting non-finite and
  outside-signed64 operands with localized catchable error 11. Both numeric
  modes retain the derived Gregorian civil-day/midnight extension policy;
  admitted out-of-calendar days return typed empty values without low32 aliases.
  Independent native calendar/absence observations, numeric boundary and state
  regressions, requirements traceability and sanitizer evidence are retained
  under #5611/#6776; Date/Julian storage, formatting and type/arity work stay separate.
