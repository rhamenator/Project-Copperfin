- 2026-10-15: Propagate known width/decimal attributes through Numeric unary
  minus under #5611/#6776 with checked width overflow, native nonzero
  original-fail/fixed coverage and explicit named sanitizer selection. Retain
  repeated native context/signed-zero recovery and PR7196 verification evidence;
  payload, assignment/display, signed-zero and full Numeric/ListItem parity
  remain unfinished rather than being declared compatible.
