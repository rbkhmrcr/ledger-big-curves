#ifndef GROUP_UTILS
#define GROUP_UTILS

#include "field.h"
#include "group.h"

// scalar length in uint64_t
#define scalar6753_uint64_length 12

typedef uint64_t scalar6753[scalar6753_uint64_length];

typedef struct gmnt6753 {
  fmnt6753 X;
  fmnt6753 Y;
  fmnt6753 Z;
} gmnt6753;

#define gmnt6753_struct_size 36

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
  {
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0
  },
  {
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 1
  },
  {
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0
  }
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
  {
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 1
  }
};

bool gmnt6753_is_zero(gmnt6753 *p);
bool gmnt6753_is_on_curve(gmnt6753 *p);
void gmnt6753_copy(gmnt6753 *r, gmnt6753 *p);

#endif //GROUP_UTILS
