#ifndef CODA_KEYS
#define CODA_KEYS

#include "group.h"

void generate_keypair(group *pub_key, scalar priv_key);
void generate_public_key(group *pub_key);

#endif // CODA_KEYS
