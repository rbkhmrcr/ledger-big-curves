// blake2b_init initializes a 256-bit unkeyed BLAKE2B hash.
void blake2b_init(cx_blake2b_t *S);
// blake2b_update adds data to a BLAKE2B hash.
void blake2b_update(cx_blake2b_t *S, const uint8_t *in, uint64_t inlen);
// blake2b_final outputs a finalized BLAKE2B hash.
void blake2b_final(cx_blake2b_t *S, uint8_t *out, uint64_t outlen);

// blake2b is a helper function that outputs the BLAKE2B hash of in.
void blake2b(uint8_t *out, uint64_t outlen, const uint8_t *in, uint64_t inlen);
