#ifndef CODA_GROUP_UTILS
#define CODA_GROUP_UTILS

#include "field.h"
#include "group.h"

// scalar length in uint64_t
#define scalar6753_uint64_length 12
// scalar length in bits
#define scalar6753_bit_length 753

typedef uint64_t scalar6753[scalar6753_uint64_length];

typedef struct gmnt6753 {
  fmnt6753 X;
  fmnt6753 Y;
  fmnt6753 Z;
} gmnt6753;

// struct size in bytes FIXME
#define gmnt6753_struct_size 36

static fmnt6753 gmnt6753_group_order = {1234, 1234, 1234, 1234, 1234, 1234,
                                        1234, 1234, 1234, 1234, 1234, 1234};

static fmnt6753 gmnt6753_coeff_a = {1234, 1234, 1234, 1234, 1234, 1234,
                                    1234, 1234, 1234, 1234, 1234, 1234};

static fmnt6753 gmnt6753_coeff_b = {1234, 1234, 1234, 1234, 1234, 1234,
                                    1234, 1234, 1234, 1234, 1234, 1234};

static const struct gmnt6753 gmnt6753_zero = {
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}};

// mnt6753 generator
//  .x =
//  "3458420969484235708806261200128850544017070333833944116801482064540723268149235477762870414664917360605949659630933184751526227993647030875167687492714052872195770088225183259051403087906158701786758441889742618916006546636728",
//  .y =
//  "27460508402331965149626600224382137254502975979168371111640924721589127725376473514838234361114855175488242007431439074223827742813911899817930728112297763448010814764117701403540298764970469500339646563344680868495474127850569"

static const struct gmnt6753 gmnt6753_one = {
    {1234, 1234, 1234, 1234, 1234, 1234, 1234, 1234, 1234, 1234, 1234, 1234},
    {1234, 1234, 1234, 1234, 1234, 1234, 1234, 1234, 1234, 1234, 1234, 1234},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1}};

bool gmnt6753_is_zero(gmnt6753 *p);
bool gmnt6753_is_on_curve(gmnt6753 *p);
void gmnt6753_copy(gmnt6753 *r, gmnt6753 *p);

#endif // CODA_GROUP_UTILS
