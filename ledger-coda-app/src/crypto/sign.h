#ifndef CODA_SIGN
#define CODA_SIGN

#include "group.h"

typedef struct signature {
  field rx;
  scalar s;
} signature;


unsigned int sign(
    signature *sig,
    group *public_key,
    scalar private_key, 
    scalar hash,
    unsigned int sig_len);

#endif // CODA_SIGN
