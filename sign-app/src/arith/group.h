#ifndef CODA_GROUP
#define CODA_GROUP

#include <stdint.h>
#include <stdbool.h>
#include "field.h"
#include "group-utils.h"

void gmnt6753_add(gmnt6753 *r, gmnt6753 *p, gmnt6753 *q);
void gmnt6753_sub(gmnt6753 *r, gmnt6753 *p, gmnt6753 *q);
void gmnt6753_double(gmnt6753 *r, gmnt6753 *p);
void gmnt6753_scalar_mul(gmnt6753 *r, fmnt6753 k, gmnt6753 *p);

#endif // CODA_GROUP
