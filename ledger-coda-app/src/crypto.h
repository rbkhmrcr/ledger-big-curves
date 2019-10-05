#ifndef CODA_CRYPTO
#define CODA_CRYPTO

#define field_bytes 96
#define scalar_bytes 96
#define group_bytes 192
#define scalar_bits 768   // scalar_bytes * 8
#define scalar_offset 15  // scalars have 753 ( = 768 - 15 ) used bits

typedef unsigned char field[field_bytes];
typedef unsigned char scalar[scalar_bytes];

typedef struct group {
  field x;
  field y;
} group;

typedef struct signature {
  field rx;
  scalar s;
} signature;

void scalar_add(scalar c, const scalar a, const scalar b);
void scalar_mul(scalar c, const scalar a, const scalar b);
void scalar_pow(scalar c, const scalar a, const scalar e);

void group_scalar_mul(group *r, const scalar k, const group *p);
void generate_keypair(unsigned int index, group *pub_key, scalar priv_key);
void generate_public_key(group *pub_key);

unsigned int sign(
    signature *sig,
    group *public_key,
    scalar private_key,
    scalar hash,
    unsigned int sig_len);

#endif // CODA_CRYPTO
