#ifndef CODA_FIELD
#define CODA_FIELD

#include<cx.h>

struct field_element {
  uint64_t val[12];
}

const field_element MODULUS = 123;

void addf(field_element *a, field_element *b);
void subf(field_element *a, field_element *b);
void mulf(field_element *a, field_element *b);
void squaref(field_element *a);
void divf(field_element *a, field_element *b);
void invf(field_element *a);

#endif // CODA_FIELD
