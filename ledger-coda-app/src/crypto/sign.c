#include "os.h"
#include "group.h"
#include "sign.h"
#include "poseidon.h"
#include <string.h>

bool is_even(field y) {
  // FIXME get smallest bit
  return false;
}

unsigned int sign(signature *sig, group *public_key, scalar private_key,
                  scalar hash, unsigned int sig_len) {

  group *r = 0;
  scalar k_prime;
  poseidon(k_prime);
  poseidon(hash);
  poseidon(private_key);
  poseidon_digest(k_prime);                   // k' = hash(sk || m)
  *r = group_scalar_mul(k_prime, &group_one); // r = k*g

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
  poseidon(hash);
  poseidon(r->x);
  poseidon(public_key->x);
  poseidon(hash);
  poseidon_digest(e);             // e = hash(xr || pk || m)
  scalar_mul(s, e, private_key); // e*sk
  scalar_add(s, k, s);            // k + e*sk

  os_memcpy(sig, r->x, field_BYTES);
  os_memcpy(sig + field_BYTES, s, scalar_BYTES);

  return (field_BYTES + scalar_BYTES);
};
