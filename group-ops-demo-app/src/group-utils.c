#include "field.h"
#include "group-utils.h"
#include <string.h>

bool is_zero(gmnt6753 *p) {
  if (os_memcmp(p->X, fmnt6753_zero, fmnt6753_BYTES) == 0 &&
      os_memcmp(p->Y, fmnt6753_one, fmnt6753_BYTES) == 0) {
    return true;
  }
  return false;
};

bool is_on_curve(gmnt6753 *p) {
  if (is_zero(p)) {
    return true;
  }

  unsigned char x2[fmnt6753_BYTES];
  unsigned char x2_a[fmnt6753_BYTES];
  unsigned char x3_ax[fmnt6753_BYTES];
  unsigned char x3_ax_b[fmnt6753_BYTES];
  unsigned char y2[fmnt6753_BYTES];

  // y^2 mod fmnt6753_modulus
  cx_math_multm(y2, gmnt6753_one.Y, gmnt6753_one.Y, fmnt6753_modulus,
                fmnt6753_BYTES);
  // x^2
  cx_math_multm(x2, gmnt6753_one.X, gmnt6753_one.X, fmnt6753_modulus,
                fmnt6753_BYTES);
  // x^2 + a
  cx_math_addm(x2_a, x2, gmnt6753_coeff_a, fmnt6753_modulus, fmnt6753_BYTES);
  // x^3 + ax
  cx_math_multm(x3_ax, x2_a, gmnt6753_one.X, fmnt6753_modulus, fmnt6753_BYTES);
  // x^3 + ax + b
  cx_math_addm(x3_ax_b, x3_ax, gmnt6753_coeff_b, fmnt6753_modulus,
               fmnt6753_BYTES);

  return os_memcmp(y2, x3_ax_b, fmnt6753_BYTES);
};

const scalar6753 gmnt6753_group_order = {
    0x00, 0x01, 0xc4, 0xc6, 0x2d, 0x92, 0xc4, 0x11, 0x10, 0x22, 0x90, 0x22,
    0xee, 0xe2, 0xcd, 0xad, 0xb7, 0xf9, 0x97, 0x50, 0x5b, 0x8f, 0xaf, 0xed,
    0x5e, 0xb7, 0xe8, 0xf9, 0x6c, 0x97, 0xd8, 0x73, 0x07, 0xfd, 0xb9, 0x25,
    0xe8, 0xa0, 0xed, 0x8d, 0x99, 0xd1, 0x24, 0xd9, 0xa1, 0x5a, 0xf7, 0x9d,
    0xb1, 0x17, 0xe7, 0x76, 0xf2, 0x18, 0x05, 0x9d, 0xb8, 0x0f, 0x0d, 0xa5,
    0xcb, 0x53, 0x7e, 0x38, 0x68, 0x5a, 0xcc, 0xe9, 0x76, 0x72, 0x54, 0xa4,
    0x63, 0x88, 0x10, 0x71, 0x9a, 0xc4, 0x25, 0xf0, 0xe3, 0x9d, 0x54, 0x52,
    0x2c, 0xdd, 0x11, 0x9f, 0x5e, 0x90, 0x63, 0xde, 0x24, 0x5e, 0x80, 0x01};

const unsigned char *const_scptr(scalar6753 a) {
  return (const unsigned char *)a;
};

void ptr_to_scalar6753(scalar6753 a, unsigned char *olda) {
  memcpy(&a, olda, scalar6753_BYTES);
};

void scalar6753_add(scalar6753 c, scalar6753 a, scalar6753 b) {
  unsigned char *d = 0;
  cx_math_addm(d, const_scptr(a), const_scptr(b), gmnt6753_group_order,
               scalar6753_BYTES);
  ptr_to_scalar6753(c, d);
};

void scalar6753_mul(scalar6753 c, scalar6753 a, scalar6753 b) {
  unsigned char *d = 0;
  cx_math_multm(d, const_scptr(a), const_scptr(b), gmnt6753_group_order,
                scalar6753_BYTES);
  ptr_to_scalar6753(c, d);
};
