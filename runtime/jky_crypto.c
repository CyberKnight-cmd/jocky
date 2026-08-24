/* JOCKY Runtime — Crypto / Part of libjocky. / SHA256 Implementation */
#include "jky_crypto.h"
#include <string.h>
#include <stdio.h>

#define ROTR(x, n) (((x) >> (n)) | ((x) << (32 - (n))))
#define CH(x, y, z) (((x) & (y)) ^ (~(x) & (z)))
#define MAJ(x, y, z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define EP0(x) (ROTR(x, 2) ^ ROTR(x, 13) ^ ROTR(x, 22))
#define EP1(x) (ROTR(x, 6) ^ ROTR(x, 11) ^ ROTR(x, 25))
#define SIG0(x) (ROTR(x, 7) ^ ROTR(x, 18) ^ ((x) >> 3))
#define SIG1(x) (ROTR(x, 17) ^ ROTR(x, 19) ^ ((x) >> 10))

static const uint32_t K[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

static void sha256_transform(uint32_t state[8], const uint8_t data[64]) {
    uint32_t a, b, c, d, e, f, g, h, i, j, t1, t2, m[64];

    for (i = 0, j = 0; i < 16; ++i, j += 4)
        m[i] = (data[j] << 24) | (data[j + 1] << 16) | (data[j + 2] << 8) | (data[j + 3]);
    for ( ; i < 64; ++i)
        m[i] = SIG1(m[i - 2]) + m[i - 7] + SIG0(m[i - 15]) + m[i - 16];

    a = state[0]; b = state[1]; c = state[2]; d = state[3];
    e = state[4]; f = state[5]; g = state[6]; h = state[7];

    for (i = 0; i < 64; ++i) {
        t1 = h + EP1(e) + CH(e, f, g) + K[i] + m[i];
        t2 = EP0(a) + MAJ(a, b, c);
        h = g; g = f; f = e; e = d + t1;
        d = c; c = b; b = a; a = t1 + t2;
    }

    state[0] += a; state[1] += b; state[2] += c; state[3] += d;
    state[4] += e; state[5] += f; state[6] += g; state[7] += h;
}

void jky_sha256(const uint8_t *data, size_t len, uint8_t out[32]) {
    uint32_t state[8] = {
        0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
        0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
    };
    uint8_t block[64];
    size_t bitlen = len * 8;
    size_t offset = 0;

    while (len >= 64) {
        sha256_transform(state, data + offset);
        offset += 64;
        len -= 64;
    }

    memcpy(block, data + offset, len);
    block[len++] = 0x80;
    if (len > 56) {
        memset(block + len, 0, 64 - len);
        sha256_transform(state, block);
        len = 0;
    }
    memset(block + len, 0, 56 - len);
    
    // Append length (big-endian 64-bit)
    block[56] = (bitlen >> 56) & 0xff;
    block[57] = (bitlen >> 48) & 0xff;
    block[58] = (bitlen >> 40) & 0xff;
    block[59] = (bitlen >> 32) & 0xff;
    block[60] = (bitlen >> 24) & 0xff;
    block[61] = (bitlen >> 16) & 0xff;
    block[62] = (bitlen >> 8) & 0xff;
    block[63] = (bitlen) & 0xff;
    sha256_transform(state, block);

    for (int i = 0; i < 8; i++) {
        out[i*4]     = (state[i] >> 24) & 0xff;
        out[i*4 + 1] = (state[i] >> 16) & 0xff;
        out[i*4 + 2] = (state[i] >> 8) & 0xff;
        out[i*4 + 3] = (state[i]) & 0xff;
    }
}

JkyString *jky_sha256_hex(const uint8_t *data, size_t len) {
    uint8_t hash[32];
    jky_sha256(data, len, hash);
    char hex[65];
    for (int i = 0; i < 32; i++) {
        sprintf(hex + i * 2, "%02x", hash[i]);
    }
    return jky_string_from_cstr(hex);
}

JkyString *jky_sha256_file(const char *path, JkyError **err) {
    FILE *f = fopen(path, "rb");
    if (!f) {
        if (err) *err = jky_error_from("Cannot open file");
        return NULL;
    }
    uint32_t state[8] = {
        0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
        0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
    };
    uint8_t buffer[1024];
    size_t bytes;
    size_t total_len = 0;
    uint8_t block[64];
    size_t block_len = 0;

    while ((bytes = fread(buffer, 1, sizeof(buffer), f)) > 0) {
        total_len += bytes;
        for (size_t i = 0; i < bytes; i++) {
            block[block_len++] = buffer[i];
            if (block_len == 64) {
                sha256_transform(state, block);
                block_len = 0;
            }
        }
    }
    fclose(f);

    size_t bitlen = total_len * 8;
    block[block_len++] = 0x80;
    if (block_len > 56) {
        memset(block + block_len, 0, 64 - block_len);
        sha256_transform(state, block);
        block_len = 0;
    }
    memset(block + block_len, 0, 56 - block_len);
    block[56] = (bitlen >> 56) & 0xff;
    block[57] = (bitlen >> 48) & 0xff;
    block[58] = (bitlen >> 40) & 0xff;
    block[59] = (bitlen >> 32) & 0xff;
    block[60] = (bitlen >> 24) & 0xff;
    block[61] = (bitlen >> 16) & 0xff;
    block[62] = (bitlen >> 8) & 0xff;
    block[63] = (bitlen) & 0xff;
    sha256_transform(state, block);

    uint8_t hash[32];
    for (int i = 0; i < 8; i++) {
        hash[i*4]     = (state[i] >> 24) & 0xff;
        hash[i*4 + 1] = (state[i] >> 16) & 0xff;
        hash[i*4 + 2] = (state[i] >> 8) & 0xff;
        hash[i*4 + 3] = (state[i]) & 0xff;
    }

    char hex[65];
    for (int i = 0; i < 32; i++) {
        sprintf(hex + i * 2, "%02x", hash[i]);
    }
    return jky_string_from_cstr(hex);
}

void jky_hmac_sha256(const uint8_t *key, size_t klen,
                     const uint8_t *data, size_t dlen, uint8_t out[32]) {
    uint8_t k_ipad[64];
    uint8_t k_opad[64];
    uint8_t tk[32];
    
    if (klen > 64) {
        jky_sha256(key, klen, tk);
        key = tk;
        klen = 32;
    }
    
    memset(k_ipad, 0, sizeof(k_ipad));
    memset(k_opad, 0, sizeof(k_opad));
    memcpy(k_ipad, key, klen);
    memcpy(k_opad, key, klen);
    
    for (int i = 0; i < 64; i++) {
        k_ipad[i] ^= 0x36;
        k_opad[i] ^= 0x5c;
    }
    
    uint8_t *inner = malloc(64 + dlen);
    memcpy(inner, k_ipad, 64);
    memcpy(inner + 64, data, dlen);
    
    uint8_t inner_hash[32];
    jky_sha256(inner, 64 + dlen, inner_hash);
    free(inner);
    
    uint8_t outer[64 + 32];
    memcpy(outer, k_opad, 64);
    memcpy(outer + 64, inner_hash, 32);
    
    jky_sha256(outer, 64 + 32, out);
}
