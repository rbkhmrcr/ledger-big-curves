#include "arith/field.h"
#include "arith/group-utils.h"
#include "arith/group.h"
#include <string.h>

/* we are using this to replace `cx_ecdsa_sign` in the boilerplate code,
 * which has the signature:
 * SYSCALL int cx_ecdsa_sign(const cx_ecfp_private_key_t WIDE *private_key PLENGTH(
                              scc__cx_scc_struct_size_ecfp_privkey__private_key),
                          int mode, cx_md_t hashID,
                          const unsigned char WIDE *hash PLENGTH(hash_len),
                          unsigned int hash_len,
                          unsigned char *sig PLENGTH(sig_len),
                          unsigned int sig_len,
                          unsigned int *info PLENGTH(sizeof(unsigned int)));

  private_key = &N_privateKey,
  mode = CX_RND_RFC6979 | CX_LAST,
  hashID = CX_SHA256,
  hash = result,
  hash_len = sizeof(result),
  sig = G_io_apdu_buffer,
  sig_len ?
  info = NULL
 */

void get_x(fmnt6753 x, gmnt6753 *p) {
  fmnt6753 invz;
  fmnt6753_inv(invz, p->Z);    // 1/Z
  fmnt6753_sq(invz, invz);     // 1/Z^2
  fmnt6753_mul(x, invz, p->X); // X/Z^2
}

void get_y(fmnt6753 y, gmnt6753 *p) {
  fmnt6753 invz, temp;
  fmnt6753_inv(invz, p->Z);       // 1/Z
  fmnt6753_sq(temp, invz);        // 1/Z^2
  fmnt6753_mul(temp, temp, invz); // 1/Z^3
  fmnt6753_mul(y, temp, p->Y);    // Y/Z^3
}

bool is_even(fmnt6753 y) {
  // FIXME get smallest bit
  return false;
}

int sign(scalar6753 *private_key, gmnt6753 *public_key,
         const unsigned char WIDE *hash PLENGTH(hash_len),
         unsigned int hash_len, unsigned char *sig PLENGTH(sig_len),
         unsigned int sig_len) {

  gmnt6753 *r = 0;
  scalar6753 k_prime;
  random_oracle(k_prime, hash, private_key, NULL);
  gmnt6753_scalar_mul(r, k_prime, &gmnt6753_one);

  fmnt6753 k, rx, ry, pkx;
  get_x(rx, r);
  get_y(ry, r);
  get_x(pkx, public_key);
  // if ry is even, k = k'
  if (is_even(ry)) {
    os_memcpy(k, k_prime, scalar6753_BYTES);
  }
  // else k = - k'
  else {
    fmnt6753_negate(k, k_prime);
  }

  scalar6753 s, e;
  random_oracle(e, hash, rx, pkx);
  scalar6753_mul(s, e, *private_key); // e*sk
  scalar6753_add(s, k, s);      // k + e*sk

  os_memcpy(sig, rx, fmnt6753_BYTES);
  os_memcpy(sig + fmnt6753_BYTES, s, scalar6753_BYTES);

  return (fmnt6753_BYTES + scalar6753_BYTES);
};
