// Trimmed from upstream github.com/orlp/ed25519 (public domain / zlib-style
// license, see license.txt in this directory) to expose only ed25519_verify.
// The sign/keypair/seed/add_scalar/key_exchange declarations that existed in
// the original ed25519.h have been deliberately removed: this vendored copy
// is verify-only, and none of sign.c/keypair.c/seed.c/add_scalar.c/
// key_exchange.c are compiled into cf_licensing.
#ifndef ED25519_H
#define ED25519_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

int ed25519_verify(const unsigned char *signature, const unsigned char *message, size_t message_len, const unsigned char *public_key);

#ifdef __cplusplus
}
#endif

#endif
