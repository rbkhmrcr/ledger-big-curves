#include "os.h"
#include "os_io_seproxyhal.h"

// que?
static char *u64str(uint64_t v) {
  static char buf[24];
  char *p = &buf[sizeof(buf)];
  *(--p) = '\0';

  if (v == 0) {
    *(--p) = '0';
    return p;
  }
  while (v > 0) {
    *(--p) = '0' + (v % 10);
    v = v / 10;
  }
  return p;
}

static const bagl_element_t bagl_ui_approval_nanos[] = {
    {
        {BAGL_RECTANGLE, 0x00, 0, 0, 128, 32, 0, 0, BAGL_FILL, 0x000000,
         0xFFFFFF, 0, 0},
        NULL,
        0,
        0,
        0,
        NULL,
        NULL,
        NULL,
    },
    {
        {BAGL_LABELINE, 0x02, 0, 12, 128, 11, 0, 0, 0, 0xFFFFFF, 0x000000,
         BAGL_FONT_OPEN_SANS_REGULAR_11px | BAGL_FONT_ALIGNMENT_CENTER, 0},
        "Sign transaction",
        0,
        0,
        0,
        NULL,
        NULL,
        NULL,
    },
    {
        {BAGL_ICON, 0x00, 3, 12, 7, 7, 0, 0, 0, 0xFFFFFF, 0x000000, 0,
         BAGL_GLYPH_ICON_CROSS},
        NULL,
        0,
        0,
        0,
        NULL,
        NULL,
        NULL,
    },
    {
        {BAGL_ICON, 0x00, 117, 13, 8, 6, 0, 0, 0, 0xFFFFFF, 0x000000, 0,
         BAGL_GLYPH_ICON_CHECK},
        NULL,
        0,
        0,
        0,
        NULL,
        NULL,
        NULL,
    },
};

static unsigned int
bagl_ui_approval_nanos_button(unsigned int button_mask,
                              unsigned int button_mask_counter) {
  switch (button_mask) {
  case BUTTON_EVT_RELEASED | BUTTON_RIGHT:
    txn_approve();
    break;

  case BUTTON_EVT_RELEASED | BUTTON_LEFT:
    txn_deny();
    break;
  }
  return 0;
}

#define DISPLAY_ELEMENTS(header)                                               \
  {                                                                            \
    {                                                                          \
        {BAGL_RECTANGLE, 0x00, 0, 0, 128, 32, 0, 0, BAGL_FILL, 0x000000,       \
         0xFFFFFF, 0, 0},                                                      \
        NULL,                                                                  \
        0,                                                                     \
        0,                                                                     \
        0,                                                                     \
        NULL,                                                                  \
        NULL,                                                                  \
        NULL,                                                                  \
    },                                                                         \
        {                                                                      \
            {BAGL_LABELINE, 0x02, 0, 12, 128, 11, 0, 0, 0, 0xFFFFFF, 0x000000, \
             BAGL_FONT_OPEN_SANS_REGULAR_11px | BAGL_FONT_ALIGNMENT_CENTER,    \
             0},                                                               \
            header,                                                            \
            0,                                                                 \
            0,                                                                 \
            0,                                                                 \
            NULL,                                                              \
            NULL,                                                              \
            NULL,                                                              \
        },                                                                     \
        {                                                                      \
            {BAGL_LABELINE, 0x02, 23, 26, 82, 11, 0x80 | 10, 0, 0, 0xFFFFFF,   \
             0x000000,                                                         \
             BAGL_FONT_OPEN_SANS_EXTRABOLD_11px | BAGL_FONT_ALIGNMENT_CENTER,  \
             26},                                                              \
            lineBuffer,                                                        \
            0,                                                                 \
            0,                                                                 \
            0,                                                                 \
            NULL,                                                              \
            NULL,                                                              \
            NULL,                                                              \
        },                                                                     \
        {                                                                      \
            {BAGL_ICON, 0x00, 3, 12, 7, 7, 0, 0, 0, 0xFFFFFF, 0x000000, 0,     \
             BAGL_GLYPH_ICON_CROSS},                                           \
            NULL,                                                              \
            0,                                                                 \
            0,                                                                 \
            0,                                                                 \
            NULL,                                                              \
            NULL,                                                              \
            NULL,                                                              \
        },                                                                     \
        {                                                                      \
            {BAGL_ICON, 0x00, 117, 13, 8, 6, 0, 0, 0, 0xFFFFFF, 0x000000, 0,   \
             BAGL_GLYPH_ICON_RIGHT},                                           \
            NULL,                                                              \
            0,                                                                 \
            0,                                                                 \
            0,                                                                 \
            NULL,                                                              \
            NULL,                                                              \
            NULL,                                                              \
        },                                                                     \
  }

#define DISPLAY_HANDLER(after_func)                                            \
  switch (button_mask) {                                                       \
  case BUTTON_EVT_RELEASED | BUTTON_RIGHT:                                     \
    if (ui_text_more()) {                                                      \
      UX_REDISPLAY();                                                          \
    } else {                                                                   \
      after_func();                                                            \
    }                                                                          \
    break;                                                                     \
  case BUTTON_EVT_RELEASED | BUTTON_LEFT:                                      \
    txn_deny();                                                                \
    break;                                                                     \
  }                                                                            \
  return 0;

/* Transactions are structured as follows:
  bool      : isDelegation
  uint      : nonce
  char[96]  : from
  char[96]  : to
  uint      : amount
  uint      : fee
  char[32]  : memo

The following are the functions to go through these, defined in reverse order.
*/

// memo

static void after_memo(void) { UX_DISPLAY(bagl_ui_approval_nanos, NULL); }

static const bagl_element_t bagl_ui_memo_nanos[] = DISPLAY_ELEMENTS("Memo:");

static unsigned int
bagl_ui_memo_nanos_button(unsigned int button_mask,
                          unsigned int button_mask_counter) {
  DISPLAY_HANDLER(after_memo)
}

// fee

static void after_fee(void) {
  ui_text_put(current_txn.memo);
  ui_text_more();
  UX_DISPLAY(bagl_ui_memo_nanos, NULL);
}

static const bagl_element_t bagl_ui_fee_nanos[] = DISPLAY_ELEMENTS("Fee:");

static unsigned int bagl_ui_fee_nanos_button(unsigned int button_mask,
                                             unsigned int button_mask_counter) {
  DISPLAY_HANDLER(after_fee)
}

// amount

static void after_amount(void) {
  ui_text_put(u64str(current_txn.fee));
  ui_text_more();
  UX_DISPLAY(bagl_ui_fee_nanos, NULL);
}

static const bagl_element_t bagl_ui_amount_nanos[] =
    DISPLAY_ELEMENTS("Amount:");

static unsigned int
bagl_ui_amount_nanos_button(unsigned int button_mask,
                            unsigned int button_mask_counter) {
  DISPLAY_HANDLER(after_amount)

  // to

  static void after_to(void) {
    ui_text_put(u64str(current_txn.amount));
    ui_text_more();
    UX_DISPLAY(bagl_ui_amount_nanos, NULL);
  }

  static const bagl_element_t bagl_ui_to_nanos[] = DISPLAY_ELEMENTS("To:");

  static unsigned int bagl_ui_to_nanos_button(
      unsigned int button_mask, unsigned int button_mask_counter) {
    DISPLAY_HANDLER(after_to)
  }

  // from

  static void after_from(void) {
    char checksummed[96];
    checksummed_addr(current_txn.to, checksummed);
    ui_text_put(checksummed);
    ui_text_more();
    UX_DISPLAY(bagl_ui_to_nanos, NULL);
  }

  static const bagl_element_t bagl_ui_from_nanos[] = DISPLAY_ELEMENTS("From:");

  static unsigned int bagl_ui_from_nanos_button(
      unsigned int button_mask, unsigned int button_mask_counter) {
    DISPLAY_HANDLER(after_from)
  }

  // nonce

  static void after_nonce(void) {
    char checksummed[96];
    checksummed_addr(current_txn.from, checksummed);
    ui_text_put(checksummed);
    ui_text_more();
    UX_DISPLAY(bagl_ui_from_nanos, NULL);
  }

  static const bagl_element_t bagl_ui_nonce_nanos[] =
      DISPLAY_ELEMENTS("Nonce:");

  static unsigned int bagl_ui_nonce_nanos_button(
      unsigned int button_mask, unsigned int button_mask_counter) {
    DISPLAY_HANDLER(after_nonce)
  }

  // isDelegation

  static void after_isDelegation(void) {
    ui_text_put(u64str(current_txn.nonce));
    ui_text_more();
    UX_DISPLAY(bagl_ui_nonce_nanos, NULL);
  }

  static const bagl_element_t bagl_ui_isDelegation_nanos[] =
      DISPLAY_ELEMENTS("isDelegation:");

  static unsigned int bagl_ui_isDelegation_nanos_button(
      unsigned int button_mask, unsigned int button_mask_counter) {
    DISPLAY_HANDLER(after_isDelegation)
  }

  void transaction_ui() {
    PRINTF("Transaction:\n");
    PRINTF("  isDelegation: %d\n", current_txn.isDelegation);
    PRINTF("  Nonce: %s\n", u64str(current_txn.nonce));
    PRINTF("  From: %.*h\n", 96, current_txn.from);
    PRINTF("  To: %.*h\n", 96, current_txn.to);
    PRINTF("  Amount: %s\n", u64str(current_txn.amount));
    PRINTF("  Fee: %s\n", u64str(current_txn.fee));
    PRINTF("  Memo: %.*h\n", 32, current_txn.memo);

    ui_text_put("Sign transaction:");
    ui_text_more();
    UX_DISPLAY(bagl_ui_isDelegation_nanos, NULL);
  }
