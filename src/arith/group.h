#ifndef CODA_GROUP
#define CODA_GROUP

#include "field.h"
#include "group-utils.h"
#include <stdbool.h>
#include <stdint.h>

void gmnt6753_add(gmnt6753 *r, gmnt6753 *p, gmnt6753 *q);
void gmnt6753_sub(gmnt6753 *r, gmnt6753 *p, gmnt6753 *q);
void gmnt6753_double(gmnt6753 *r, gmnt6753 *p);
void gmnt6753_scalar_mul(gmnt6753 *r, scalar6753 k, const gmnt6753 *p);

#endif // CODA_GROUP
