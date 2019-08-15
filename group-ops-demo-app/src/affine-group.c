#include "field.h"
#include "group-utils.h"
#include <stdbool.h>
#include <stdint.h>

// maybe make gmnt6753 just const char gmnt6753[2*fmnt6753_BYTES] ?
void gmnt6753_affine_add(gmnt6753 r, gmnt6753 p, gmnt6753 q) {

  fmnt6753 lambda, xqxp, yqyp, ixqxp;
  cx_math_subm(xqxp, q.X, p.X, fmnt6753_modulus, fmnt6753_BYTES);       // xq - xp
  cx_math_subm(yqyp, q.Y, p.Y, fmnt6753_modulus, fmnt6753_BYTES);       // yq - yp
  cx_math_invprimem(ixqxp, xqxp, fmnt6753_modulus, fmnt6753_BYTES);     // 1 / (xq - xp)
  cx_math_multm(lambda, yqyp, ixqxp, fmnt6753_modulus, fmnt6753_BYTES); // (yq - yp)/(xq - xp)

  fmnt6753 l2, lxp;
  cx_math_multm(l2, lambda, lambda, fmnt6753_modulus, fmnt6753_BYTES);  // lambda^2
  cx_math_subm(lxp, l2, p.X, fmnt6753_modulus, fmnt6753_BYTES);         // lambda^2 - xp
  cx_math_subm(r.X, lxp, q.X, fmnt6753_modulus, fmnt6753_BYTES);        // lambda^2 - xp - xq

  fmnt6753 xpxr, lxpxr;
  cx_math_subm(xpxr, p.X, r.X, fmnt6753_modulus, fmnt6753_BYTES);       // xp - xr
  cx_math_multm(lxpxr, lambda, xpxr, fmnt6753_modulus, fmnt6753_BYTES); // lambda(xp - xr)
  cx_math_subm(r.Y, lxpxr, p.Y, fmnt6753_modulus, fmnt6753_BYTES);      // lambda(xp - xr) - yp
}

void gmnt6753_affine_double(gmnt6753 r, gmnt6753 p) {

  fmnt6753 lambda, xp2, xp22, xp23, xp23a, yp2, iyp2;
  cx_math_multm(xp2, p.X, p.X, fmnt6753_modulus, fmnt6753_BYTES);       // xp^2
  cx_math_addm(xp22, xp2, xp2, fmnt6753_modulus, fmnt6753_BYTES);       // 2xp^2
  cx_math_addm(xp23, xp22, xp2, fmnt6753_modulus, fmnt6753_BYTES);      // 3xp^2
  cx_math_addm(xp23a, xp23, gmnt6753_coeff_a, fmnt6753_modulus, fmnt6753_BYTES);  // 3xp^2 + a
  cx_math_addm(yp2, p.Y, p.Y, fmnt6753_modulus, fmnt6753_BYTES);        // 2yp
  cx_math_invprimem(iyp2, yp2, fmnt6753_modulus, fmnt6753_BYTES);       // 1/2yp
  cx_math_multm(lambda, xp23a, iyp2, fmnt6753_modulus, fmnt6753_BYTES); // (3xp^2 + a)/2yp

  fmnt6753 l2, lxp;
  cx_math_multm(l2, lambda, lambda, fmnt6753_modulus, fmnt6753_BYTES);  // lambda^2
  cx_math_subm(lxp, l2, p.X, fmnt6753_modulus, fmnt6753_BYTES);         // lambda^2 - xp
  cx_math_subm(r.X, lxp, p.X, fmnt6753_modulus, fmnt6753_BYTES);        // lambda^2 - xp - xp

  fmnt6753 xpxr, lxpxr;
  cx_math_subm(xpxr, p.X, r.X, fmnt6753_modulus, fmnt6753_BYTES);       // xp - xr
  cx_math_multm(lxpxr, lambda, xpxr, fmnt6753_modulus, fmnt6753_BYTES); // lambda(xp - xr)
  cx_math_subm(r.Y, lxpxr, p.Y, fmnt6753_modulus, fmnt6753_BYTES);      // lambda(xp - xr) - yp
}

void gmnt6753_affine_scalar_mul(gmnt6753 *r, scalar6753 k, const gmnt6753 *p) {
}
