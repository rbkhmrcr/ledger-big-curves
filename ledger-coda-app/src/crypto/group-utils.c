#include "group-utils.h"
#include "field.h"
#include <string.h>

bool is_zero(gmnt6753 *p) {

  if (os_memcmp(p->X, fmnt6753_zero, fmnt6753_BYTES) == 0 &&
      os_memcmp(p->Y, fmnt6753_one, fmnt6753_BYTES) == 0 &&
      os_memcmp(p->Z, fmnt6753_zero, fmnt6753_BYTES) == 0) {

    return true;
  }

  return false;
};

bool is_on_curve(gmnt6753 *p) {

  if (is_zero(p)) {

    return true;

  } else if (fmnt6753_eq(p->Z, fmnt6753_one)) {
    // we can check y^2 == x^3 + ax + b
    fmnt6753 y2;
    fmnt6753_sq(y2, p->Y);

    fmnt6753 x3_ax_b;
    fmnt6753_sq(x3_ax_b, p->X);                       // x^2
    fmnt6753_add(x3_ax_b, x3_ax_b, gmnt6753_coeff_a); // x^2 + a
    fmnt6753_mul(x3_ax_b, x3_ax_b, p->X);             // x^3 + ax
    fmnt6753_add(x3_ax_b, x3_ax_b, gmnt6753_coeff_b); // x^3 + ax + b

    return fmnt6753_eq(y2, x3_ax_b);

  } else {
    // we check (y/z)^2 == (x/z)^3 + a(x/z) + b
    // => z(y^2 - bz^2) == x(x^2 + az^2)
    fmnt6753 x2, y2, z2;
    fmnt6753_sq(x2, p->X);
    fmnt6753_sq(y2, p->Y);
    fmnt6753_sq(z2, p->Z);

    fmnt6753 lhs, rhs;
    fmnt6753_mul(lhs, z2, gmnt6753_coeff_b);          // bz^2
    fmnt6753_sub(lhs, y2, lhs);                       // y^2 - bz^2
    fmnt6753_mul(lhs, p->Z, lhs);                     // z(y^2 - bz^2)
    fmnt6753_mul(rhs, z2, gmnt6753_coeff_a);          // az^2
    fmnt6753_add(rhs, x2, rhs);                       // x^2 + az^2
    fmnt6753_mul(rhs, p->X, rhs);                     // x(x^2 + az^2)

    return fmnt6753_eq(lhs, rhs);
  }

  return false;
};

const unsigned char gmnt6753_group_order[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

const unsigned char *const_scptr(scalar6753 a) { return (const unsigned char *)a; };

void ptr_to_scalar6753(scalar6753 a, unsigned char *olda) {
  memcpy(&a, olda, scalar6753_BYTES);
};

void scalar6753_add(scalar6753 c, scalar6753 a, scalar6753 b) {
  unsigned char *d = 0;
  cx_math_addm(d, const_scptr(a), const_scptr(b), gmnt6753_group_order, scalar6753_BYTES);
  ptr_to_scalar6753(c, d);
};

void scalar6753_mul(scalar6753 c, scalar6753 a, scalar6753 b) {
  unsigned char *d = 0;
  cx_math_multm(d, const_scptr(a), const_scptr(b), gmnt6753_group_order,
                scalar6753_BYTES);
  ptr_to_scalar6753(c, d);
};