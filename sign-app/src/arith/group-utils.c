#include "field.h"
#include "group.h"
#include "group-utils.h"

// mnt6753 generator
//  .x = "3458420969484235708806261200128850544017070333833944116801482064540723268149235477762870414664917360605949659630933184751526227993647030875167687492714052872195770088225183259051403087906158701786758441889742618916006546636728",
//  .y = "27460508402331965149626600224382137254502975979168371111640924721589127725376473514838234361114855175488242007431439074223827742813911899817930728112297763448010814764117701403540298764970469500339646563344680868495474127850569"


bool is_zero(gmnt6753 *p) {
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
    fmnt6753_sq(x3_ax_b, p->X);                     // x*2
    fmnt6753_add(x3_ax_b, x3_ax_b, gmnt6753_coeff_a);        // x*2 + a
    fmnt6753_mul(x3_ax_b, x3_ax_b, p->X);           // x*3 + ax
    fmnt6753_add(x3_ax_b, x3_ax_b, gmnt6753_coeff_b);        // x*3 + ax + b

    return fmnt6753_eq(y2, x3_ax_b);
  }
  else {
    // we check (y/z)^2 == (x/z)^3 + a(x/z) + b
    // or z(y^2 - bz^2) == x(x^2 + az^2) 

    fmnt6753 X2, Y2, Z2;
    fmnt6753_sq(X2, p->X);
    fmnt6753_sq(Y2, p->Y);
    fmnt6753_sq(Z2, p->Z);
  }

  return false;
};
