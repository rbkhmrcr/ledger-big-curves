#ifndef GROUP_UTILS
#define GROUP_UTILS 

#include "field.h"
#include "group.h"

typedef struct gmnt6753 {
  fmnt6753 X;
  fmnt6753 Y;
  fmnt6753 Z;
} gmnt6753;

const fmnt6753 gmnt6753_group_order =
{
  1234, 1234, 1234, 1234,
  1234, 1234, 1234, 1234,
  1234, 1234, 1234, 1234
};

const fmnt6753 gmnt6753_coeff_a =
{
  1234, 1234, 1234, 1234,
  1234, 1234, 1234, 1234,
  1234, 1234, 1234, 1234
};

const fmnt6753 gmnt6753_coeff_b =
{
  1234, 1234, 1234, 1234,
  1234, 1234, 1234, 1234,
  1234, 1234, 1234, 1234
};

const gmnt6753 gmnt6753_zero =
{
  fmnt6753_zero,
  fmnt6753_zero,
  fmnt6753_zero
};

const gmnt6753 gmnt6753_one =
{
  {
    1234, 1234, 1234, 1234,
    1234, 1234, 1234, 1234,
    1234, 1234, 1234, 1234
  },
  {
    1234, 1234, 1234, 1234,
    1234, 1234, 1234, 1234,
    1234, 1234, 1234, 1234
  },
  fmnt6753_one
};

bool gmnt6753_is_zero(gmnt6753 *p);
bool gmnt6753_is_on_curve(gmnt6753 *p);
void gmnt6753_copy(gmnt6753 *r, gmnt6753 *p);

#endif //GROUP_UTILS
