#include "field.h"
#include "syscalls.h"
#include <string.h>

const unsigned char fmnt6753_modulus[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

unsigned char *ptr(fmnt6753 a) { return (unsigned char *)a; };

const unsigned char *const_ptr(fmnt6753 a) { return (const unsigned char *)a; };

void ptr_to_fmnt6753(fmnt6753 a, unsigned char *olda) {
  // uint64_t a[fmnt6753_uint64];
  memcpy(&a, olda, fmnt6753_BYTES);
};

/**
 * All of the cx_ operations are similar to the
 * following:
 *
 * Modular addition of two big integer: r = a+b mod m
 * @param r    where to put result        // unsigned char *
 * @param a    first operand              // const unsigned char *
 * @param b    second operand             // const unsigned char *
 * @param m    modulo                     // const unsigned char *
 * @param len  byte length of r, a, b, m
 *
 * cx works in BE
 **/

void fmnt6753_add(fmnt6753 c, fmnt6753 a, fmnt6753 b) {
  unsigned char *d = 0;
  cx_math_addm(d, const_ptr(a), const_ptr(b), fmnt6753_modulus, fmnt6753_BYTES);
  ptr_to_fmnt6753(c, d);
};

void fmnt6753_sub(fmnt6753 c, fmnt6753 a, fmnt6753 b) {
  unsigned char *d = 0;
  cx_math_subm(d, const_ptr(a), const_ptr(b), fmnt6753_modulus, fmnt6753_BYTES);
  ptr_to_fmnt6753(c, d);
};

void fmnt6753_mul(fmnt6753 c, fmnt6753 a, fmnt6753 b) {
  unsigned char *d = 0;
  cx_math_multm(d, const_ptr(a), const_ptr(b), fmnt6753_modulus,
                fmnt6753_BYTES);
  ptr_to_fmnt6753(c, d);
};

void fmnt6753_sq(fmnt6753 c, fmnt6753 a) {
  unsigned char *d = 0;
  cx_math_multm(d, const_ptr(a), const_ptr(a), fmnt6753_modulus,
                fmnt6753_BYTES);
  ptr_to_fmnt6753(c, d);
};

void fmnt6753_div(fmnt6753 c, fmnt6753 a, fmnt6753 b);

void fmnt6753_inv(fmnt6753 c, fmnt6753 a) {
  unsigned char *d = 0;
  cx_math_invprimem(d, const_ptr(a), fmnt6753_modulus, fmnt6753_BYTES);
  ptr_to_fmnt6753(c, d);
};

void fmnt6753_int_mul(fmnt6753 c, uint64_t b, fmnt6753 a);

bool fmnt6753_eq(const fmnt6753 a, const fmnt6753 b); // loop
