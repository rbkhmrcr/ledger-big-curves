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

void field_add(field c, const field a, const field b);
void field_mul(field c, const field a, const field b);
void field_pow(field c, const field a, const field e);

void group_scalar_mul(group *r, const scalar k, const group *p);
void generate_pubkey(group *pub_key, const scalar priv_key);
void generate_keypair(unsigned int index, group *pub_key, scalar priv_key);

void sign(field rx, scalar s, const group *public_key, const scalar private_key,
    const scalar msgx, const scalar msgm);

#endif // CODA_CRYPTO
