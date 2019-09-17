#ifndef CODA_CRYPTO_UTILS
#define CODA_CRYPTO_UTILS

#include <stdbool.h>

#define field_BYTES 96
#define field_BITS 753
#define scalar_BYTES 96
#define scalar_BITS 753

typedef unsigned char field[field_BYTES];
typedef unsigned char scalar[scalar_BYTES];

typedef struct group {
  field x;
  field y;
} group;

void field_add(field c, const field a, const field b);
void field_sub(field c, const field a, const field b);
void field_mul(field c, const field a, const field b);
void field_sq(field c, const field a);
void field_inv(field c, const field a);

void scalar_add(scalar c, const scalar a, const scalar b);
void scalar_sub(scalar c, const scalar a, const scalar b);
void scalar_mul(scalar c, const scalar a, const scalar b);
void scalar_sq(scalar c, const scalar a);

void group_add(group *r, const group *p, const group *q);
void group_double(group *r, const group *p);
group group_scalar_mul(const scalar k, const group *p);

#endif // CODA_CRYPTO_UTILS

