- 2026-10-02: Arithmetic and ordering on the 64-bit integers a DECLAREd external function
  returns are now exact (#6035). `nExact + 1` with `nExact = 9007199254740993` gave a
  rounded double result, int64 `+ - * /` converted both sides through double before
  narrowing, `-INT64_MIN` and `INT64_MIN / -1` were C++ signed overflow, and `<`/`>` on
  integers above 2^53 compared rounded values. The result is int64 when it fits, uint64
  only when an operand is uint64 and it exceeds INT64_MAX, and otherwise catchable numeric
  overflow (error 39). A Numeric operand that is a whole number joins the exact path; a
  fractional Numeric, two Numerics, and a Numeric divisor keep their double arithmetic.
  Equality between an int64 and a whole Numeric is now exact too, so 2^53+1 no longer equals 2^53.
