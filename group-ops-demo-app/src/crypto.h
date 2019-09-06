#ifndef CODA_CRYPTO_UTILS
#define CODA_CRYPTO_UTILS

#include <stdbool.h>

// length in uint64_t
#define fmnt6753_uint64 12
// length in bytes
#define fmnt6753_BYTES 96
// length in bits
#define fmnt6753_BITS 753
// scalar length in uint64_t
#define scalar6753_uint64 12
// scalar length in bytes
#define scalar6753_BYTES 96
// scalar length in bits
#define scalar6753_BITS 753
// (x, y) length in bits
#define gmnt6753_BYTES 192

typedef unsigned char fmnt6753[fmnt6753_BYTES];
typedef unsigned char scalar6753[scalar6753_BYTES];
typedef unsigned char group[gmnt6753_BYTES];

typedef struct gmnt6753 {
  fmnt6753 X;
  fmnt6753 Y;
  fmnt6753 Z;
} gmnt6753;

extern const fmnt6753 fmnt6753_modulus;
extern const fmnt6753 fmnt6753_zero;
extern const fmnt6753 fmnt6753_one;
extern const fmnt6753 gmnt6753_coeff_a;
extern const fmnt6753 gmnt6753_coeff_b;
extern const struct gmnt6753 gmnt6753_zero;
extern const struct gmnt6753 gmnt6753_one;

void fmnt6753_add(fmnt6753 c, const fmnt6753 a, const fmnt6753 b);
void fmnt6753_sub(fmnt6753 c, const fmnt6753 a, const fmnt6753 b);
void fmnt6753_mul(fmnt6753 c, const fmnt6753 a, const fmnt6753 b);
void fmnt6753_dbl(fmnt6753 c, const fmnt6753 a);

void scalar6753_add(scalar6753 c, const scalar6753 a, const scalar6753 b);
void scalar6753_mul(scalar6753 c, const scalar6753 a, const scalar6753 b);

// TODO : do we need these exposed?
bool gmnt6753_is_zero(gmnt6753 *p);
bool gmnt6753_is_on_curve(gmnt6753 *p);
void gmnt6753_copy(gmnt6753 *r, gmnt6753 *p);

void gmnt6753_affine_add(gmnt6753 *r, const gmnt6753 *p, const gmnt6753 *q);
void gmnt6753_affine_double(gmnt6753 *r, const gmnt6753 *p);
void gmnt6753_affine_scalar_mul(gmnt6753 *r, const scalar6753 k, const gmnt6753 *p);

#endif // CODA_CRYPTO_UTILS
