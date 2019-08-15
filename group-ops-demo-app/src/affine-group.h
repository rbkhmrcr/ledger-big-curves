#ifndef CODA_AFFINE_GROUP
#define CODA_AFFINE_GROUP

#include "field.h"
#include "group-utils.h"
#include <stdbool.h>
#include <stdint.h>

void gmnt6753_affine_add(gmnt6753 r, gmnt6753 p, gmnt6753 q);
gmnt6753_affine_double(gmnt6753 r, gmnt6753 p);
void gmnt6753_affine_scalar_mul(gmnt6753 r, scalar6753 k, const gmnt6753 p);

#endif // CODA_AFFINE_GROUP
