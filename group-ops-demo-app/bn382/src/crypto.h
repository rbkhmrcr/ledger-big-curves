#ifndef CODA_CRYPTO
#define CODA_CRYPTO

#define field_bytes   48
#define scalar_bytes  48
#define group_bytes   144
#define affine_bytes  96
#define scalar_bits   384   // scalar_bytes * 8
#define scalar_offset 2     // scalars have 382 ( = 384 - 2 ) used bits

typedef unsigned char field[field_bytes];
typedef unsigned char scalar[scalar_bytes];

typedef struct group {
  field X;
  field Y;
  field Z;
} group;

typedef struct affine {
  field x;
  field y;
} affine;

typedef struct signature {
  field rx;
  scalar s;
} signature;

void field_add(field c, const field a, const field b);
void field_mul(field c, const field a, const field b);
void field_pow(field c, const field a, const field e);

void affine_scalar_mul(affine *r, const scalar k, const affine *p);
void generate_pubkey(affine *pub_key, const scalar priv_key);
void generate_keypair(unsigned int index, affine *pub_key, scalar priv_key);

void sign(field rx, scalar s, const affine *public_key, const scalar private_key,
    const scalar msg);

#endif // CODA_CRYPTO
