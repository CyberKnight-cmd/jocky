/* JOCKY Runtime — Crypto / Part of libjocky. / Hash functions */
#ifndef JKY_CRYPTO_H
#define JKY_CRYPTO_H

#include "jky_value.h"
#include <stdint.h>
#include <stddef.h>

void       jky_sha256(const uint8_t *data, size_t len, uint8_t out[32]);
JkyString *jky_sha256_hex(const uint8_t *data, size_t len);
JkyString *jky_sha256_file(const char *path, JkyError **err);
void       jky_hmac_sha256(const uint8_t *key, size_t klen,
                           const uint8_t *data, size_t dlen, uint8_t out[32]);

#endif // JKY_CRYPTO_H
