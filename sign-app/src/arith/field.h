#ifndef CODA_FIELD
#define CODA_FIELD

#include "os.h"
#include <stdbool.h>

// length in uint64_t
#define fmnt6753_uint64 12
// length in bytes
#define fmnt6753_BYTES 96
// length in bits
#define fmnt6753_BITS 753

typedef uint64_t fmnt6753[fmnt6753_uint64];

static const fmnt6753 fmnt6753_zero = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

static const fmnt6753 fmnt6753_one = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1};

void fmnt6753_add(fmnt6753 c, fmnt6753 a, fmnt6753 b);
void fmnt6753_sub(fmnt6753 c, fmnt6753 a, fmnt6753 b);
void fmnt6753_mul(fmnt6753 c, fmnt6753 a, fmnt6753 b);
void fmnt6753_div(fmnt6753 c, fmnt6753 a, fmnt6753 b);
void fmnt6753_sq(fmnt6753 c, fmnt6753 a);
void fmnt6753_inv(fmnt6753 c, fmnt6753 a);
void fmnt6753_int_mul(fmnt6753 c, uint64_t b, fmnt6753 a);
bool fmnt6753_eq(const fmnt6753 a, const fmnt6753 b);
#endif // CODA_FIELD
