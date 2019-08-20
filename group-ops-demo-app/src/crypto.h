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
// (x, y) length in bytes
#define gmnt6753_affine_BYTES 192
// (x, y, z) length in bytes
#define gmnt6753_BYTES 288

typedef unsigned char fmnt6753[fmnt6753_BYTES];
typedef unsigned char scalar6753[scalar6753_BYTES];
//typedef unsigned char group[gmnt6753_BYTES];

typedef struct gmnt6753_affine {
  fmnt6753 X;
  fmnt6753 Y;
} gmnt6753_affine;

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
extern const gmnt6753_affine gmnt6753_affine_zero;
extern const gmnt6753_affine gmnt6753_affine_one;
extern const gmnt6753 gmnt6753_zero;
extern const gmnt6753 gmnt6753_one;

bool is_zero(const gmnt6753 *p);
bool is_on_curve(const gmnt6753 *p);

void scalar6753_add(scalar6753 c, const scalar6753 a, const scalar6753 b);
void scalar6753_mul(scalar6753 c, const scalar6753 a, const scalar6753 b);

void gmnt6753_affine_add(gmnt6753_affine *r, const gmnt6753_affine *p, const gmnt6753_affine *q);
void gmnt6753_affine_double(gmnt6753_affine *r, const gmnt6753_affine *p);
gmnt6753_affine gmnt6753_affine_scalar_mul(const scalar6753 k, const gmnt6753_affine *p);

void gmnt6753_add(gmnt6753 *r, const gmnt6753 *p, const gmnt6753 *q);
void gmnt6753_madd(gmnt6753 *r, const gmnt6753 *p, const gmnt6753 *q);
void gmnt6753_dbl(gmnt6753 *r, const gmnt6753 *p);
gmnt6753 gmnt6753_scalar_mul(const scalar6753 k, const gmnt6753 *p);

//void group_add(group xy, const gmnt6753 *p, const gmnt6753 *q);
//void group_double(group xy, const gmnt6753 *p);
//void group_scalar_mul(group r, const scalar6753 k, const gmnt6753 *p);

#endif // CODA_CRYPTO_UTILS
