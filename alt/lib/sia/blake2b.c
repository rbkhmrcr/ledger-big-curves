#include <stdint.h>
#include <os.h>
#include <cx.h>

void blake2b_init(cx_blake2b_t *S) {
	cx_blake2b_init(S, 256);
}

void blake2b_update(cx_blake2b_t *S, const uint8_t *in, uint64_t inlen) {
	cx_hash((cx_hash_t *)S, 0, in, inlen, NULL, 0);
}

void blake2b_final(cx_blake2b_t *S, uint8_t *out, uint64_t outlen) {
	uint8_t buf[32];
	cx_hash((cx_hash_t *)S, CX_LAST, NULL, 0, buf, sizeof(buf));
	os_memmove(out, buf, outlen);
}

void blake2b(uint8_t *out, uint64_t outlen, const uint8_t *in, uint64_t inlen) {
	cx_blake2b_t S;
	blake2b_init(&S);
	blake2b_update(&S, in, inlen);
	blake2b_final(&S, out, outlen);
}
