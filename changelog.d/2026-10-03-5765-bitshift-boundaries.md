Fixed `BITLSHIFT()` and `BITRSHIFT()` numeric conversions so shift counts and operands cannot trigger undefined C++ behavior, with safe 32-bit semantics and explicit VFP9 compatibility-mode handling.
