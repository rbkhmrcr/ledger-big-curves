#include "os.h"
#include "cx.h"
#include "crypto.h"
#include "poseidon.h"

// E1/Fp : y^2 = x^3 + 7
// BN382_p = 5543634365110765627805495722742127385843376434033820803590214255538854698464778703795540858859767700241957783601153
// BN382_q = 5543634365110765627805495722742127385843376434033820803592568747918351978899288491582778380528407187068941959692289
// 382 bits = 48 bytes
// these differ only in the 25th - 32nd bytes (the start of the third row)
static const field field_modulus = {
    0x24, 0x04, 0x89, 0x3f, 0xda, 0xd8, 0x87, 0x8e, 0x71, 0x50, 0x3c, 0x69,
    0xb0, 0x9d, 0xbf, 0x88, 0xb4, 0x8a, 0x36, 0x14, 0x28, 0x9b, 0x09, 0x01,
    0x20, 0x12, 0x24, 0x6d, 0x22, 0x42, 0x41, 0x20, 0x00, 0x00, 0x00, 0x01,
    0x80, 0x0c, 0x18, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01};

static const scalar group_order = {
    0x24, 0x04, 0x89, 0x3f, 0xda, 0xd8, 0x87, 0x8e, 0x71, 0x50, 0x3c, 0x69,
    0xb0, 0x9d, 0xbf, 0x88, 0xb4, 0x8a, 0x36, 0x14, 0x28, 0x9b, 0x09, 0x01,
    0x80, 0x18, 0x30, 0x91, 0x83, 0x03, 0x01, 0x80, 0x00, 0x00, 0x00, 0x01,
    0x80, 0x0c, 0x18, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01};

// a = 0, b = 7
static const field group_coeff_b = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07};

static const field field_one = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01};

static const field field_two = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01};

static const field field_mimus_one = {
    0x24, 0x04, 0x89, 0x3f, 0xda, 0xd8, 0x87, 0x8e, 0x71, 0x50, 0x3c, 0x69,
    0xb0, 0x9d, 0xbf, 0x88, 0xb4, 0x8a, 0x36, 0x14, 0x28, 0x9b, 0x09, 0x01,
    0x20, 0x12, 0x24, 0x6d, 0x22, 0x42, 0x41, 0x20, 0x00, 0x00, 0x00, 0x01,
    0x80, 0x0c, 0x18, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

static const field field_zero = {0};
static const scalar scalar_zero = {0};
static const group group_zero = {{0}, field_one, {0}};

// g_generator = (1, 1587713460471950740217388326193312024737041813752165827005856534245539019723616944862168333942330219466268138558982)
// make projective by setting Z = 1
// g1_generator (g1 has a = 0, b = 14 though?) = (1, 93360544046129830094757569027791679210844519762232758194920967606984287664392872848607365449491441272860487554919)
static const field gen_y = {
    0x0a, 0x50, 0xca, 0x03, 0xe4, 0xff, 0xad, 0x6e, 0x34, 0xfe, 0x4c, 0x72,
    0xf1, 0x3f, 0x2f, 0xbe, 0x5b, 0x32, 0xd0, 0x95, 0x41, 0xfc, 0x19, 0x5a,
    0x61, 0x91, 0x61, 0x76, 0x5f, 0x55, 0xc5, 0xce, 0x98, 0x43, 0xbe, 0x34,
    0x35, 0x3b, 0x8a, 0x3e, 0xfd, 0xc4, 0x03, 0xcd, 0x9d, 0x3c, 0x56, 0x06};

static const group group_one = {field_one, gen_y, field_one};

void field_add(field c, const field a, const field b) {
  cx_math_addm(c, a, b, field_modulus, field_bytes);
}

void field_sub(field c, const field a, const field b) {
  cx_math_subm(c, a, b, field_modulus, field_bytes);
}

void field_mul(field c, const field a, const field b) {
  cx_math_multm(c, a, b, field_modulus, field_bytes);
}

void field_sq(field c, const field a) {
  cx_math_multm(c, a, a, field_modulus, field_bytes);
}

void field_inv(field c, const field a) {
  cx_math_invprimem(c, a, field_modulus, field_bytes);
}

void field_negate(field c, const field a) {
  cx_math_subm(c, field_modulus, a, field_modulus, field_bytes);
}

// c = a^e mod m
// cx_math_powm(result_pointer, a, e, len_e, m, len(result)  (which is also
// len(a) and len(m)) )
void field_pow(field c, const field a, const field e) {
  cx_math_powm(c, a, e, 1, field_modulus, field_bytes);
}

unsigned int field_eq(const field a, const field b) {
  return (os_memcmp(a, b, field_bytes) == 0);
}

void scalar_add(scalar c, const scalar a, const scalar b) {
  cx_math_addm(c, a, b, group_order, scalar_bytes);
}

void scalar_sub(scalar c, const scalar a, const scalar b) {
  cx_math_subm(c, a, b, group_order, scalar_bytes);
}

void scalar_mul(scalar c, const scalar a, const scalar b) {
  cx_math_multm(c, a, b, group_order, scalar_bytes);
}

void scalar_sq(scalar c, const scalar a) {
  cx_math_multm(c, a, a, group_order, scalar_bytes);
}

// c = a^e mod m
// cx_math_powm(result_pointer, a, e, len_e, m, len(result)  (which is also
// len(a) and len(m)) )
void scalar_pow(scalar c, const scalar a, const scalar e) {
  cx_math_powm(c, a, e, 1, group_order, scalar_bytes);
}

unsigned int scalar_eq(const scalar a, const scalar b) {
  return (os_memcmp(a, b, scalar_bytes) == 0);
}

unsigned int is_zero(const group *p) {
  return (os_memcmp(p->x, field_zero, field_bytes) == 0 &&
          os_memcmp(p->y, field_one, field_bytes) == 0 &&
          os_memcmp(p->z, field_zero, field_bytes) == 0);
}

unsigned int is_on_curve(const group *p) {
  if (is_zero(p)) {
    return 1;
  }

  field t0, t1, t2;
  field_mul(t0, p->y, p->y);        // y^2
  field_mul(t1, p->x, p->x);        // x^2
  field_add(t2, t1, group_coeff_a); // x^2 + a
  field_mul(t1, t2, p->x);          // x^3 + ax
  field_add(t2, t1, group_coeff_b); // x^3 + ax + b

  return (os_memcmp(t0, t2, field_bytes) == 0);
}

void group_double(group *r, const group *p) {

  if (is_zero(p)) {
    *r = group_zero;
    return;
  }

  field t1, t2;
  field_mul(r->y, p->x, p->x);        // xp^2
  field_add(t1, r->y, r->y);          // 2xp^2
  field_add(t2, r->y, t1);            // 3xp^2
  field_add(r->y, t2, group_coeff_a); // 3xp^2 + a
  field_add(t1, p->y, p->y);          // 2yp
  field_inv(t2, t1);                  // 1/2yp
  field_mul(t1, r->y, t2);            // (3xp^2 + a)/2yp

  field_mul(r->y, t1, t1);   // lambda^2
  field_sub(t2, r->y, p->x); // lambda^2 - xp
  field_sub(r->x, t2, p->x); // lambda^2 - xp - xp

  field_sub(r->y, p->x, r->x); // xp - xr
  field_mul(t2, t1, r->y);     // lambda(xp - xr)
  field_sub(r->y, t2, p->y);   // lambda(xp - xr) - yp
  return;
}

void group_add(group *r, const group *p, const group *q) {

  if (is_zero(p)) {
    *r = *q;
    return;
  }
  if (is_zero(q)) {
    *r = *p;
    return;
  }

  field t1;
  field_mul(t1, p->x, q->x);
  if (field_eq(t1, field_zero)) {
    // if pxqx == 0, either p = q -> p + q = 2p
    if (field_eq(p->y, q->y)) {
      group_double(r, p);
      return;
    } else {
      // or p = -q -> p + q = 0
      *r = group_zero;
      return;
    }
  }

  field_sub(r->y, q->x, p->x); // xq - xp
  field_sub(t1, q->y, p->y);   // yq - yp
  field_inv(r->x, r->y);       // 1 / (xq - xp)
  field_mul(r->y, t1, r->x);   // (yq - yp)/(xq - xp)

  field_mul(t1, r->y, r->y);   // lambda^2
  field_sub(r->x, t1, p->x);   // lambda^2 - xp
  field_sub(r->x, r->x, q->x); // lambda^2 - xp - xq

  field_sub(t1, p->x, r->x);   // xp - xr
  field_mul(r->y, r->y, t1);   // lambda(xp - xr)
  field_sub(r->y, r->y, p->y); // lambda(xp - xr) - yp
  return;
}


void group_scalar_mul(group *r, const scalar k, const group *p) {

  *r = group_zero;
  if (is_zero(p)) {
    return;
  }
  if (scalar_eq(k, scalar_zero)) {
    return;
  }

  for (unsigned int i = scalar_offset; i < scalar_bits; i++) {
    unsigned int di = k[i / 8] & (1 << (7 - (i % 8)));
    group q0;
    group_double(&q0, r);
    *r = q0;
    if (di != 0) {
      group_add(&q0, r, p);
      *r = q0;
    }
  }
  return;
}

// Ledger uses:
// - BIP 39 to generate and interpret the master seed, which
//   produces the 24 words shown on the device at startup.
// - BIP 32 for HD key derivation (using the child key derivation function)
// - BIP 44 for HD account derivation (so e.g. btc and coda keys don't clash)

void generate_keypair(unsigned int index, group *pub_key, scalar priv_key) {

  unsigned int bip32_path[5];
  unsigned char chain[32];

  bip32_path[0] = 44 | 0x80000000;
  bip32_path[1] = 49370 | 0x80000000;
  bip32_path[2] = index | 0x80000000;
  bip32_path[3] = 0;
  bip32_path[4] = 0;

  os_perso_derive_node_bip32(CX_CURVE_256K1, bip32_path,
                             sizeof(bip32_path) / sizeof(bip32_path[0]),
                             priv_key, chain);
  os_memcpy(priv_key + 32, chain, 32);
  os_memcpy(priv_key + 64, chain, 32);

  group_scalar_mul(pub_key, priv_key, &group_one);
  // os_memset(priv_key, 0, sizeof(priv_key));
  return;
}

void generate_pubkey(group *pub_key, const scalar priv_key) {
  group_scalar_mul(pub_key, priv_key, &group_one);
  return;
}

inline unsigned int is_odd(const field y) { return (y[field_bytes - 1] & 1); }

void poseidon_4in(scalar out, const scalar in1, const scalar in2,
                  const scalar in3, const scalar in4) {
  state pos = {{0}, {0}, {0}};
  scalar tmp[sponge_size - 1];

  os_memcpy(tmp[0], in1, scalar_bytes);
  os_memcpy(tmp[1], in2, scalar_bytes);
  poseidon(pos, tmp);
  os_memcpy(tmp[0], in3, scalar_bytes);
  os_memcpy(tmp[1], in4, scalar_bytes);
  poseidon(pos, tmp);
  poseidon_digest(pos, out);
  return;
}

void sign(field rx, scalar s, const group *public_key, const scalar private_key,
          const scalar msg) {
  scalar k_prime;
  /* rx is G_io_apdu_buffer so we can take group_bytes bytes from it */
  {
    group *r;
    r = (group *)rx;
    poseidon_4in(k_prime, msg, public_key->x, public_key->y,
                 private_key);                // k = hash(m || pk || sk)
    group_scalar_mul(r, k_prime, &group_one); // r = k*g

    /* store so we don't need group *r anymore */
    os_memcpy(rx, r->x, field_bytes);
    if (is_odd(r->y)) {
      field_negate(k_prime, k_prime); // if ry is odd, k = - k'
    }
  }
  poseidon_4in(s, msg, rx, public_key->x,
               public_key->y);   // e = hash(xr || pk || m)
  scalar_mul(s, s, private_key); // e*sk
  scalar_add(s, k_prime, s);     // k + e*sk
  return;
}
