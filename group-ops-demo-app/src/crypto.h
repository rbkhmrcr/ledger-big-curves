#ifndef CODA_CRYPTO_UTILS
#define CODA_CRYPTO_UTILS

#include <stdbool.h>

#define field_BYTES 96
#define field_BITS 753
#define scalar_uint64 12
#define scalar_BYTES 96
#define scalar_BITS 753

typedef unsigned char field[field_BYTES];
typedef unsigned char scalar[scalar_BYTES];

typedef struct group {
  field x;
  field y;
} group;

void group_add(group *r, const group *p, const group *q);
void group_double(group *r, const group *p);
group group_scalar_mul(const scalar k, const group *p);

#endif // CODA_CRYPTO_UTILS
