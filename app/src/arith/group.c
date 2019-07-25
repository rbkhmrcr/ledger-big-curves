#include "group.h"

bool is_on_curve(group_element *p) {
  if is_zero(p) {
    return true
  } else {
    field_element y2 = &p.y;
    y2 = squaref(y2);

    field_element x3_ax_b = &p.x; // x
    squaref(x3_ax_b); // x*2
    addf(x3_ax_b, a_coeff); // x*2 + a
    mulf(x3_ax_b, p.x); // x*3 + ax
    addf(x3_ax_b, b_coeff); // x*3 + ax + b

    return y2 == x3_ax_b
  }
}

// From https://www.hyperelliptic.org/EFD/g1p/auto-shortw-jacobian.html#doubling-dbl-2007-bl
group_element doubleg(group_element *a) {
      /*
      XX = X1^2
      YY = Y1^2
      YYYY = YY^2
      ZZ = Z1^2
      S = 2*((X1+YY)^2-XX-YYYY)
      M = 3*XX+a*ZZ^2
      T = M^2-2*S
      X3 = T
      Y3 = M*(S-T)-8*YYYY
      Z3 = (Y1+Z1)^2-YY-ZZ
      */

}

// From https://www.hyperelliptic.org/EFD/g1p/auto-shortw-jacobian.html#addition-madd-2007-bl
// for a = (X1, Y1, Z1), b = (X2, Y2, Z2) assumes Z2 = 1
void addg(group_element *a, group_element *b) {
      /*
      Z1Z1 = Z1^2
      U2 = X2*Z1Z1
      S2 = Y2*Z1*Z1Z1
      H = U2-X1
      HH = H^2
      I = 4*HH
      J = H*I
      r = 2*(S2-Y1)
      V = X1*I
      X3 = r^2-J-2*V
      Y3 = r*(V-X3)-2*Y1*J
      Z3 = (Z1+H)^2-Z1Z1-HH
      */
}

void scalar_mul(field_element *k, group_element *a) {
}
