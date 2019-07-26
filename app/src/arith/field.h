#ifndef CODA_FIELD
#define CODA_FIELD

#include "os.h"
#include "cx.h"

typedef uint64_t fmnt6753[12];

const fmnt6753 MODULUS =
  {1234, 1234, 1234, 1234,
  1234, 1234, 1234, 1234,
  1234, 1234, 1234, 1234};

void fmnt6753_add(fmnt6753 *c, fmnt6753 *a, fmnt6753 *b);
void fmnt6753_sub(fmnt6753 *c, fmnt6753 *a, fmnt6753 *b);
void fmnt6753_mul(fmnt6753 *c, fmnt6753 *a, fmnt6753 *b);
void fmnt6753_div(fmnt6753 *c, fmnt6753 *a, fmnt6753 *b);
void fmnt6753_sq(fmnt6753 *c, fmnt6753 *a);
void fmnt6753_inv(fmnt6753 *c, fmnt6753 *a);
void fmnt6753_int_mul(fmnt6753 *c, uint64_t b, fmnt6753 *a);

#endif // CODA_FIELD
