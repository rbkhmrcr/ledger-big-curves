#include "field.h"
#include "group-utils.h"
#include <stdbool.h>
#include <stdint.h>

void gmnt6753_affine_add(gmnt6753 *r, gmnt6753 *p, gmnt6753 *q) {

  fmnt6753 lambda, xqxp, yqyp, ixqxp;
  cx_math_subm(xqxp, q->X, p->X, fmnt6753_modulus, fmnt6753_BYTES);     // xq - xp
  cx_math_subm(yqyp, q->Y, p->Y, fmnt6753_modulus, fmnt6753_BYTES);     // yq - yp
  cx_math_invprimem(ixqxp, xqxp, fmnt6753_modulus, fmnt6753_BYTES);     // 1 / (xq - xp)
  cx_math_multm(lambda, xqxp, ixqxp, fmnt6753_modulus, fmnt6753_BYTES); // (yq - yp)/(xq - xp)

  fmnt6753 l2, lxp;
  cx_math_multm(l2, lambda, lambda, fmnt6753_modulus, fmnt6753_BYTES);  // lambda^2
  cx_math_subm(lxp, l2, p->X, fmnt6753_modulus, fmnt6753_BYTES);        // lambda^2 - xp
  cx_math_subm(r->X, lxp, q->X, fmnt6753_modulus, fmnt6753_BYTES);      // lambda^2 - xp - xq

  fmnt6753 xpxr, lxpxr;
  cx_math_subm(xpxr, p->X, r->X, fmnt6753_modulus, fmnt6753_BYTES);     // xp - xr
  cx_math_multm(lxpxr, lambda, xpxr, fmnt6753_modulus, fmnt6753_BYTES); // lambda(xp - xr)
  cx_math_subm(r->Y, lxpxr, p->Y, fmnt6753_modulus, fmnt6753_BYTES);    // lambda(xp - xr) - yp
};

void gmnt6753_affine_double(gmnt6753 *r, gmnt6753 *p) {
};

void gmnt6753_affine_scalar_mul(gmnt6753 *r, scalar6753 k, const gmnt6753 *p) {
};
