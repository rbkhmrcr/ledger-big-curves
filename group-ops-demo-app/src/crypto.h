#ifndef CODA_CRYPTO
#define CODA_CRYPTO

#include <stdbool.h>

#define field_BYTES 96
#define scalar_BYTES 96
#define group_BYTES 192
#define scalar_bits 768   // scalar_BYTES * 8
#define scalar_offset 15  // scalars have 753 ( = 768 - 15 ) used bits

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
