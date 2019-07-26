#include "group.h"

bool is_zero(gmnt6753 *p) {
  return false;
};

bool is_on_curve(gmnt6753 *p) {
  if (is_zero(p)) {
    return true;
  } else if (Z == 1) {
    fmnt6753 y2;
    fmnt6753_sq(y2, p->Y);

    fmnt6753 x3_ax_b;
    fmnt6753_sq(&x3_ax_b, p->X);                    // x*2
    fmnt6753_add(&x3_ax_b, &x3_ax_b, &coeff_a);     // x*2 + a
    fmnt6753_mul(&x3_ax_b, &x3_ax_b, &p->X);        // x*3 + ax
    fmnt6753_add(&x3_ax_b, &x3_ax_b, &coeff_b);     // x*3 + ax + b

    return y2.val == x3_ax_b.val;
  } else {
    return false;
  }
};

// will we always be doubling things with z = 1?
// From https://www.hyperelliptic.org/EFD/g1p/auto-shortw-jacobian.html#doubling-dbl-2007-bl
void gmnt6753_double(gmnt6753 *r, gmnt6753 *p) {

  // XX = X1^2
  fmnt6753 xx;
  fmnt6753_sq(xx, p->X);
  // YY = Y1^2
  // YYYY = YY^2
  // YYYY = YY^2
  // ZZ = Z1^2
  fmnt6753 yy, yyyy, zz;
  fmnt6753_sq(yy, p->Y);
  fmnt6753_sq(yyyy, yy);
  fmnt6753_sq(zz, p->Z);
  // S = 2*((X1+YY)^2-XX-YYYY)
  fmnt6753 s;
  fmnt6753_add(s, p->X, yy);
  fmnt6753_sq(s, s);
  fmnt6753_sub(s, xx);
  fmnt6753_sub(s, yyyy);
  fmnt6753 s = const_mul(2, s);
  // M = 3*XX+a*ZZ^2
  fmnt6753 m;
  fmnt6753_sq(m, zz);
  fmnt6753_mul(m, m, &coeff_a);
  fmnt6753_add(m, m, const_mul(3, xx));
  // T = M^2-2*S
  fmnt6753 t;
  fmnt6753_sq(t, m);
  fmnt6753_sub(t, t, const_mul(2, s));
  // X3 = T
  r->X = t;
  // Y3 = M*(S-T)-8*YYYY
  fmnt6753_sub(r->Y, s, t);
  fmnt6753_mul(r->Y, r->Y, m);
  fmnt6753_sub(r->Y, r->Y, const_mul(8, yyyy));
  // Z3 = (Y1+Z1)^2-YY-ZZ
  fmnt6753_add(r->Z, p->Y, p->Z);
  fmnt6753_sq(r->Z, r->Z);
  fmnt6753_sub(r->Z, r->Z, yy);
  fmnt6753_sub(r->Z, r->Z, zz);

};

// From https://www.hyperelliptic.org/EFD/g1p/auto-shortw-jacobian.html#addition-madd-2007-bl
// for a = (X1, Y1, Z1), b = (X2, Y2, Z2) assumes Z2 = 1
void gmnt6753_add(gmnt6753 *r, gmnt6753 *p, gmnt6753 *q) {

  // Z1Z1 = Z1^2
  fmnt6753 z1z1;
  fmnt6753_sq(z1z1, p->Z);
  // U2 = X2*Z1Z1
  fmnt6753 u2;
  fmnt6753_mul(u2, q->X, z1z1);
  // S2 = Y2*Z1*Z1Z1
  fmnt6753 s2;
  fmnt6753_mul(s2, q->Y, p->Z);
  fmnt6753_mul(s2, s2, z1z1);
  // H = U2-X1
  // HH = H^2
  fmnt6753 h, hh;
  fmnt6753_sub(h, u2, p->X);
  fmnt6753_sq(hh, h);
  // I = 4*HH
  fmnt6753 i;
  i = const_mul(4, hh);
  // J = H*I
  // V = X1*I
  fmnt6753 j, v;
  fmnt6753_mul(j, h, i);
  fmnt6753_mul(v, p->X, i);
  // w = 2*(S2-Y1) -> we'll call this w
  fmnt6753 w;
  fmnt6753_sub(w, s2, p->Y);
  // X3 = w^2-J-2*V
  fmnt6753_sq(r->X, w);
  fmnt6753_sub(r->X, r->X, j);
  fmnt6753_sub(r->X, r->X, const_mul(2, v));
  // Y3 = w*(V-X3)-2*Y1*J
  fmnt6753_sub(r->Y, v, r->X);
  fmnt6753_mul(r->Y, r->Y, w);
  fmnt6753_mul(j, j, p->Y);
  fmnt6753_sub(r->Y, r->Y, const_mul(2, j));
  // Z3 = (Z1+H)^2-Z1Z1-HH
  fmnt6753_add(r->Z, p->Z, h);
  fmnt6753_sq(r->Z, r->Z);
  fmnt6753_sub(r->Z, r->Z, z1z1);
  fmnt6753_sub(r->Z, r->Z, hh);

};

void gmnt6753_scalar_mul(gmnt6753 *r, fmnt6753 *k, gmnt6753 *p) {
//   R0 ← 0
//  R1 ← P
//  for i from m downto 0 do
//     if di = 0 then
//        R1 ← point_add(R0, R1)
//        R0 ← point_double(R0)
//     else
//        R0 ← point_add(R0, R1)
//        R1 ← point_double(R1)
//  return R0
};
