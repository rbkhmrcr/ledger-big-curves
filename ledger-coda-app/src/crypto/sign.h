#ifndef CODA_SIGN
#define CODA_SIGN

#include "group.h"

typedef struct signature {
  field rx;
  scalar s;
} signature;


int sign(
    signature *sig,
    scalar *private_key, 
    group *public_key,
    scalar *hash,
    unsigned int sig_len);

#endif // CODA_SIGN
