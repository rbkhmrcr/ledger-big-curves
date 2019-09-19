#ifndef CODA_KEYS
#define CODA_KEYS

#include "group.h"

void generate_keypair(scalar priv_key, group *pub_key);
void generate_public_key(group *pub_key);

#endif // CODA_KEYS
