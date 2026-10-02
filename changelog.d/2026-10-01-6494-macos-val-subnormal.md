- 2026-10-01: On macOS, `VAL("-1E-308")` and any other representable subnormal
  number text was rejected, because the Apple libc++ stream fallback in
  `try_parse_invariant_double` treated the `failbit` libc++ sets for a
  subnormal (`strtod` ERANGE) as a parse failure; this also kept the macOS
  native validation lane red (#6494). Subnormals are now accepted as
  `std::from_chars` accepts them on Linux and Windows, while overflow and
  total underflow still fail.
