#ifndef POSEIDON
#define POSEIDON

#include "crypto.h"

// alpha = smallest prime st gcd(p, alpha) = 1
// m = number of field elements in the state
// N = number of rounds
// For m = rq + cq, sponge absorbs (via field addition) and squeezes rq field
// elements per iteration, and offers log2(cq) bits of security.
// here alpha = 11, m = 3, r = 1, s = 2 ?
// we split the full rounds into two and put half before the parital ro
// and half after. we have full rounds = 8 and partial = 33, totalling 41

#define ROUNDS 41
#define SPONGE_SIZE 3

void poseidon(scalar state[3], const scalar input);
void poseidon_digest(scalar state[3], scalar out);

#endif /* POSEIDON */
