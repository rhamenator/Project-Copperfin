- 2026-10-04: Matched installed VFP9 `STR()` width and decimals bounds, including
  the exact 237-character width ceiling, error 1908, and explicit VFP9-mode
  negative low-32-bit conversion, while rejecting unsafe raw arguments by
  default before precision formatting or padding allocation (#5611, #6776).
