# Vendored: orlp/ed25519 (verify-only)

Source: https://github.com/orlp/ed25519 (public domain / zlib-style license,
see `license.txt` in this directory — reproduced verbatim, unmodified).

Only the verify-path files were vendored: `fe.c/h`, `ge.c/h`, `sc.c/h`,
`sha512.c/h`, `fixedint.h`, `precomp_data.h`, `ed25519.h`, `verify.c`.
`sign.c`, `keypair.c`, `seed.c`, `add_scalar.c`, and `key_exchange.c` from
the upstream repository are **not** included — this build has no signing,
key-generation, or key-exchange capability, deliberately.

`ed25519.h` has been trimmed (comment explains why, at the top of that
file) to declare only `ed25519_verify`; every other file in this directory
is byte-for-byte as fetched from upstream. Do not "clean up" or reformat
these files — keeping them identical to upstream makes it possible to diff
against a future upstream release if one is ever needed.

This code is called only from `src/licensing/ed25519_verify.h`'s thin
wrapper, which exposes no signing entry point.
