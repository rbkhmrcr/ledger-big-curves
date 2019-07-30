#include "field.h"

/**
 * All of the cx_ operations are similar to the
 * following:
 *
 * Modular addition of two big integer: r = a+b mod m
 * @param r    where to put result        // const unsigned char *
 * @param a    first operand              // const unsigned char *
 * @param b    second operand             // const unsigned char *
 * @param m    modulo                     // const unsigned char *
 * @param len  byte length of r, a, b, m
 **/

void fmnt6753_add(fmnt6753 c, fmnt6753 a, fmnt6753 b) {
  cx_math_addm(c, a, b, fmnt6753_modulus, fmnt6753_length);
};

void fmnt6753_sub(fmnt6753 c, fmnt6753 a, fmnt6753 b) {
  cx_math_subm(c, a, b, fmnt6753_modulus, fmnt6753_length);
};

void fmnt6753_mul(fmnt6753 c, fmnt6753 a, fmnt6753 b) {
  cx_math_multm(c, a, b, fmnt6753_modulus, fmnt6753_length);
};

void fmnt6753_div(fmnt6753 c, fmnt6753 a, fmnt6753 b);

void fmnt6753_sq(fmnt6753 c, fmnt6753 a) {
  cx_math_multm(c, a, a, fmnt6753_modulus, fmnt6753_length);
};

void fmnt6753_inv(fmnt6753 c, fmnt6753 a) {
  cx_math_invprimem(c, a, fmnt6753_modulus, fmnt6753_length);
};

void fmnt6753_int_mul(fmnt6753 c, uint64_t b,
                      fmnt6753 a); 

bool fmnt6753_eq(fmnt6753 a, fmnt6753 b); // loop
