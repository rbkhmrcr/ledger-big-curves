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

  fmnt6753 xx;
  fmnt6753_sq(xx, p->X);                            // XX = X1^2

  fmnt6753 yy, yyyy, zz;
  fmnt6753_sq(yy, p->Y);                            // YY = Y1^2
  fmnt6753_sq(yyyy, yy);                            // YYYY = YY^2
  fmnt6753_sq(zz, p->Z);                            // ZZ = Z1^2

  fmnt6753 s;                                       // S = 2*((X1+YY)^2-XX-YYYY)
  fmnt6753_add(s, p->X, yy);                        // X1+YY
  fmnt6753_sq(s, s);                                // (X1+YY)^2
  fmnt6753_sub(s, xx);                              // (X1+YY)^2-XX
  fmnt6753_sub(s, yyyy);                            // (X1+YY)^2-XX-YYYY
  fmnt6753 s = const_mul(2, s);                     // 2*((X1+YY)^2-XX-YYYY)

  fmnt6753 m;                                       // M = 3*XX+a*ZZ^2
  fmnt6753_sq(m, zz);                               // ZZ^2
  fmnt6753_mul(m, m, &coeff_a);                     // a*ZZ^2
  fmnt6753_add(m, m, const_mul(3, xx));             // 3*XX + a*ZZ^2

  fmnt6753 t;                                       // T = M^2-2*S
  fmnt6753_sq(t, m);                                // M^2
  fmnt6753_sub(t, t, const_mul(2, s));              // M^2-2*S

  // X3 = T
  r->X = t;

  // Y3 = M*(S-T)-8*YYYY
  fmnt6753_sub(r->Y, s, t);                         // S-T
  fmnt6753_mul(r->Y, r->Y, m);                      // Y3 = M*(S-T)
  fmnt6753_sub(r->Y, r->Y, const_mul(8, yyyy));

  // Z3 = (Y1+Z1)^2-YY-ZZ
  fmnt6753_add(r->Z, p->Y, p->Z);                   // Y1+Z1
  fmnt6753_sq(r->Z, r->Z);                          // (Y1+Z1)^2
  fmnt6753_sub(r->Z, r->Z, yy);                     // (Y1+Z1)^2-YY
  fmnt6753_sub(r->Z, r->Z, zz);                     // (Y1+Z1)^2-YY-ZZ

};

// From https://www.hyperelliptic.org/EFD/g1p/auto-shortw-jacobian.html#addition-madd-2007-bl
// for p = (X1, Y1, Z1), q = (X2, Y2, Z2); assumes Z2 = 1
void gmnt6753_add(gmnt6753 *r, gmnt6753 *p, gmnt6753 *q) {

  fmnt6753 z1z1, u2;
  fmnt6753_sq(z1z1, p->Z);                          // Z1Z1 = Z1^2
  fmnt6753_mul(u2, q->X, z1z1);                     // U2 = X2*Z1Z1

  fmnt6753 s2;
  fmnt6753_mul(s2, q->Y, p->Z);                     // S2 = Y2*Z1
  fmnt6753_mul(s2, s2, z1z1);                       // S2 = Y2*Z1*Z1Z1

  fmnt6753 h, hh;
  fmnt6753_sub(h, u2, p->X);                        // H = U2-X1
  fmnt6753_sq(hh, h);                               // HH = H^2

  fmnt6753 i, j, v, w;                              // w is r in link
  i = const_mul(4, hh);                             // I = 4*HH
  fmnt6753_mul(j, h, i);                            // J = H*I
  fmnt6753_mul(v, p->X, i);                         // V = X1*I
  fmnt6753_sub(w, s2, p->Y);                        // w = 2*(S2-Y1)

  // X3 = w^2-J-2*V
  fmnt6753_sq(r->X, w);                             // w^2
  fmnt6753_sub(r->X, r->X, j);                      // w^2-J
  fmnt6753_sub(r->X, r->X, const_mul(2, v));        // w^2-J-2*V

  // Y3 = w*(V-X3)-2*Y1*J
  fmnt6753_sub(r->Y, v, r->X);                      // V-X3
  fmnt6753_mul(r->Y, r->Y, w);                      // w*(V-X3)
  fmnt6753_mul(j, j, p->Y);                         // Y1*J
  fmnt6753_sub(r->Y, r->Y, const_mul(2, j));        // w*(V-X3)-2*Y1*J

  // Z3 = (Z1+H)^2-Z1Z1-HH
  fmnt6753_add(r->Z, p->Z, h);                      // Z1+H
  fmnt6753_sq(r->Z, r->Z);                          // (Z1+H)^2
  fmnt6753_sub(r->Z, r->Z, z1z1);                   // (Z1+H)^2-Z1Z1
  fmnt6753_sub(r->Z, r->Z, hh);                     // (Z1+H)^2-Z1Z1-HH

};

void gmnt6753_scalar_mul(gmnt6753 *r, fmnt6753 *k, gmnt6753 *p) {
//  R0 ← 0
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
