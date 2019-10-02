#ifndef CODA_CRYPTO
#define CODA_CRYPTO

#include <stdbool.h>

#define field_BYTES 96
#define field_BITS 753
#define scalar_BYTES 96
#define scalar_BITS 753
#define group_BYTES 192

typedef unsigned char field[field_BYTES];
typedef unsigned char scalar[scalar_BYTES];

typedef struct group {
  field x;
  field y;
} group;

group group_scalar_mul(const scalar k, const group *p);
void generate_keypair(group *pub_key, scalar priv_key);
void generate_public_key(group *pub_key);

#endif // CODA_CRYPTO
