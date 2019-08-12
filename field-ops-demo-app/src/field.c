#include "field.h"
#include "syscalls.h"
#include <string.h>

const unsigned char fmnt6753_modulus[96] = {
    0x00, 0x01, 0xc4, 0xc6, 0x2d, 0x92, 0xc4, 0x11, 0x10, 0x22, 0x90, 0x22,
    0xee, 0xe2, 0xcd, 0xad, 0xb7, 0xf9, 0x97, 0x50, 0x5b, 0x8f, 0xaf, 0xed,
    0x5e, 0xb7, 0xe8, 0xf9, 0x6c, 0x97, 0xd8, 0x73, 0x07, 0xfd, 0xb9, 0x25,
    0xe8, 0xa0, 0xed, 0x8d, 0x99, 0xd1, 0x24, 0xd9, 0xa1, 0x5a, 0xf7, 0x9d,
    0xb2, 0x6c, 0x5c, 0x28, 0xc8, 0x59, 0xa9, 0x9b, 0x3e, 0xeb, 0xca, 0x94,
    0x29, 0x21, 0x26, 0x36, 0xb9, 0xdf, 0xf9, 0x76, 0x34, 0x99, 0x3a, 0xa4,
    0xd6, 0xc3, 0x81, 0xbc, 0x3f, 0x00, 0x57, 0x97, 0x4e, 0xa0, 0x99, 0x17,
    0x0f, 0xa1, 0x3a, 0x4f, 0xd9, 0x07, 0x76, 0xe2, 0x40, 0x00, 0x00, 0x01};

unsigned char *ptr(fmnt6753 a) { return (unsigned char *)a; };

const unsigned char *const_ptr(fmnt6753 a) { return (const unsigned char *)a; };

void ptr_to_fmnt6753(fmnt6753 a, unsigned char *olda) {
  // uint64_t a[fmnt6753_uint64];
  os_memcpy(&a, olda, fmnt6753_BYTES);
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

// -a is equivalent to modulus - a
void fmnt6753_negate(fmnt6753 c, fmnt6753 a) {
  unsigned char *d = 0;
  cx_math_subm(d, fmnt6753_modulus, const_ptr(a), fmnt6753_modulus,
               fmnt6753_BYTES);
  ptr_to_fmnt6753(c, d);
};

void fmnt6753_mul(fmnt6753 c, fmnt6753 a, fmnt6753 b) {
  unsigned char *d = 0;
  cx_math_multm(d, const_ptr(a), const_ptr(b), fmnt6753_modulus,
                fmnt6753_BYTES);
  ptr_to_fmnt6753(c, d);
};

void fmnt6753_int_mul(fmnt6753 c, uint64_t a, fmnt6753 b) {
  unsigned char *d = 0;
  // FIXME
  cx_math_multm(d, (const unsigned char *)a, const_ptr(b), fmnt6753_modulus,
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
