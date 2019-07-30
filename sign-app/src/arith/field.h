#ifndef CODA_FIELD
#define CODA_FIELD

#include <stdbool.h>
#include "os.h"
#include "cx.h"

// length in bytes
#define fmnt6753_length 96
// length in uint64_t
#define fmnt6753_uint64_length 12
// length in bits
#define fmnt6753_bit_length 753

typedef uint64_t fmnt6753[fmnt6753_uint64_length];

const fmnt6753 fmnt6753_modulus =
{
  497830423872529,
  1162650133526138285,
  13256793349531086829,
  6825179918269667443,
  575819899841080717,
  11083680675069097885,
  12856752366465690011,
  4533940187572545078,
  13393698102907058852,
  15475355390085978007,
  5665696655557605967,
  5638598945175764993
};

const fmnt6753 fmnt6753_zero  =
{
  0, 0, 0, 0,
  0, 0, 0, 0,
  0, 0, 0, 0
};

const fmnt6753 fmnt6753_one =
{
  0, 0, 0, 0,
  0, 0, 0, 0,
  0, 0, 0, 1
};

void fmnt6753_add(fmnt6753 c, fmnt6753 a, fmnt6753 b);
void fmnt6753_sub(fmnt6753 c, fmnt6753 a, fmnt6753 b);
void fmnt6753_mul(fmnt6753 c, fmnt6753 a, fmnt6753 b);
void fmnt6753_div(fmnt6753 c, fmnt6753 a, fmnt6753 b);
void fmnt6753_sq(fmnt6753 c, fmnt6753 a);
void fmnt6753_inv(fmnt6753 c, fmnt6753 a);
void fmnt6753_int_mul(fmnt6753 c, uint64_t b, fmnt6753 a);
bool fmnt6753_eq(fmnt6753 a, fmnt6753 b);
#endif // CODA_FIELD
