#ifndef CODA_CRYPTO_UTILS
#define CODA_CRYPTO_UTILS

#include <stdbool.h>

#define field_uint64 12
#define field_BYTES 96
#define field_BITS 753

#define scalar_uint64 12
#define scalar_BYTES 96
#define scalar_BITS 753

#define group_BYTES 288
#define affine_BYTES 192

typedef unsigned char field[field_BYTES];
typedef unsigned char scalar[scalar_BYTES];

typedef struct group {
  field X;
  field Y;
  field Z;
} group;

typedef struct affine {
  field x;
  field y;
} affine;

extern const field field_modulus;
extern const field field_zero;
extern const field field_one;
extern const field group_coeff_a;
extern const field group_coeff_b;
extern const struct group group_zero;
extern const struct group group_one;

void field_add(field c, const field a, const field b);
void field_sub(field c, const field a, const field b);
void field_mul(field c, const field a, const field b);
void field_dbl(field c, const field a);
void field_inv(field c, const field a);

void scalar_add(scalar c, const scalar a, const scalar b);
void scalar_mul(scalar c, const scalar a, const scalar b);

void affine_to_projective(group *r, const affine *p);
void projective_to_affine(affine *r, const group *p);

void group_add(group *r, const group *p, const group *q);
void group_madd(group *r, const group *p, const group *q);
void group_double(group *r, const group *p);
void group_scalar_mul(group *r, const scalar k, const group *p);

void affine_add(affine *r, const affine *p, const affine *q);
void affine_double(affine *r, const affine *p);
void affine_scalar_mul(affine *r, const scalar k, const affine *p);

#endif /* CODA_CRYPTO_UTILS */
