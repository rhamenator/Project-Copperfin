# License Signer Tooling

Copyright © 2026 Richard M. Hamilton. All rights reserved.

This directory contains back-office tooling for generating and using the
Ed25519 signing key that issues Project Copperfin commercial license files
(`*.cflicense`). It is intentionally **not** part of the CMake build graph
and must never be linked into, bundled with, or shipped as part of any
Copperfin product binary.

- `generate_signing_key.sh` — generates an Ed25519 keypair under
  `keys/` (git-ignored) and emits a ready-to-embed C++ public-key header.

**The private key must never be committed to this repository, or to any
other shared or public location.** Only the public key header derived from
it is safe to commit, and only that header should ever be copied into
`include/copperfin/licensing/ed25519_public_key.h`.

If this script is ever copied into another machine or repository purely as
a file-transport mechanism, delete it from git history there once it has
served that purpose — it should not persist as tracked, reachable source
on more machines than necessary.
