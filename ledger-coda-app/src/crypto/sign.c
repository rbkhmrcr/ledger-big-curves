#include "group.h"
#include "rescue.h"
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

bool is_even(field y) {
  // FIXME get smallest bit
  return false;
}

int schnorr_sign(
    signature *sig,
    scalar *private_key,
    group *public_key,
    scalar *hash,
    unsigned int sig_len) {

  group *r = 0;
  scalar k_prime;
  rescue(k_prime, hash, private_key, NULL);
  group_scalar_mul(r, k_prime, &group_one);

  field k;
  // if ry is even, k = k'
  if (is_even(r->y)) {
    os_memcpy(k, k_prime, scalar_BYTES);
  }
  // else k = - k'
  else {
    field_negate(k, k_prime);
  }

  scalar s, e;
  rescue(e, hash, r->x, public_key->x);
  scalar_mul(s, e, *private_key); // e*sk
  scalar_add(s, k, s);      // k + e*sk

  os_memcpy(sig, r->x, field_BYTES);
  os_memcpy(sig + field_BYTES, s, scalar_BYTES);

  return (field_BYTES + scalar_BYTES);
};
