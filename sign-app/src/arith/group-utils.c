#include "group-utils.h"
#include "field.h"
#include "group.h"

bool is_zero(gmnt6753 *p) { return false; };

bool is_on_curve(gmnt6753 *p) {
  if (is_zero(p)) {
    return true;
  } else if (fmnt6753_eq(p->Z, fmnt6753_one)) {
    // we can check y^2 == x^3 + ax + b
    fmnt6753 y2;
    fmnt6753_sq(y2, p->Y);

    fmnt6753 x3_ax_b;
    fmnt6753_sq(x3_ax_b, p->X);                       // x*2
    fmnt6753_add(x3_ax_b, x3_ax_b, gmnt6753_coeff_a); // x*2 + a
    fmnt6753_mul(x3_ax_b, x3_ax_b, p->X);             // x*3 + ax
    fmnt6753_add(x3_ax_b, x3_ax_b, gmnt6753_coeff_b); // x*3 + ax + b

    return fmnt6753_eq(y2, x3_ax_b);
  } else {
    // we check (y/z)^2 == (x/z)^3 + a(x/z) + b
    // or z(y^2 - bz^2) == x(x^2 + az^2)

    fmnt6753 X2, Y2, Z2;
    fmnt6753_sq(X2, p->X);
    fmnt6753_sq(Y2, p->Y);
    fmnt6753_sq(Z2, p->Z);
  }

  return false;
};
