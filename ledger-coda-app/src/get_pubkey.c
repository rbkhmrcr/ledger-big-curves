#include <os.h>
#include <os_io_seproxyhal.h>
#include <cx.h>
#include "crypto.h"
#include "ux.h"

static pubkey_context *ctx = &global.pk;

static const bagl_element_t ui_pubkey_compare[] = {
  UI_BACKGROUND(),
  UI_ICON_LEFT(0x01, BAGL_GLYPH_ICON_LEFT),
  UI_ICON_RIGHT(0x02, BAGL_GLYPH_ICON_RIGHT),
  UI_TEXT(0x00, 0, 12, 128, "Compare:"),
  UI_TEXT(0x00, 0, 26, 128, global.pk.partial_str),
};

static const bagl_element_t* ui_prepro_pubkey_compare(const bagl_element_t *element) {
  if ((element->component.userid == 1 && ctx->display_index == 0) ||
      (element->component.userid == 2 && ctx->display_index == group_bytes-12)) {
    return NULL;
  }
  return element;
}

static unsigned int ui_pubkey_compare_button(unsigned int button_mask, unsigned int button_mask_counter) {
  switch (button_mask) {
  case BUTTON_LEFT:
  case BUTTON_EVT_FAST | BUTTON_LEFT: // SEEK LEFT
    if (ctx->display_index > 0) {
      ctx->display_index--;
    }
    os_memmove(ctx->partial_str, ctx->full_str+ctx->display_index, 12);
    UX_REDISPLAY();
    break;

  case BUTTON_RIGHT:
  case BUTTON_EVT_FAST | BUTTON_RIGHT: // SEEK RIGHT
    if (ctx->display_index < group_bytes-12) {
      ctx->display_index++;
    }
    os_memmove(ctx->partial_str, ctx->full_str+ctx->display_index, 12);
    UX_REDISPLAY();
    break;

  case BUTTON_EVT_RELEASED | BUTTON_LEFT | BUTTON_RIGHT: // PROCEED
    ui_idle();
    break;
  }
  return 0;
}

void bin2hex(uint8_t *dst, uint8_t *data, uint64_t inlen) {
  static uint8_t const hex[] = "0123456789abcdef";
  for (uint64_t i = 0; i < inlen; i++) {
    dst[2*i+0] = hex[(data[i]>>4) & 0x0F];
    dst[2*i+1] = hex[(data[i]>>0) & 0x0F];
  }
  dst[2*inlen] = '\0';
}


int bin2dec(uint8_t *dst, uint64_t n) {
  if (n == 0) {
    dst[0] = '0';
    dst[1] = '\0';
    return 1;
  }
  // determine final length
  int len = 0;
  for (uint64_t nn = n; nn != 0; nn /= 10) {
    len++;
  }
  // write digits in big-endian order
  for (int i = len-1; i >= 0; i--) {
    dst[i] = (n % 10) + '0';
    n /= 10;
  }
  dst[len] = '\0';
  return len;
}

static const bagl_element_t ui_pubkey_approve[] = {
  UI_BACKGROUND(),
  UI_ICON_LEFT(0x00, BAGL_GLYPH_ICON_CROSS),
  UI_ICON_RIGHT(0x00, BAGL_GLYPH_ICON_CHECK),
  UI_TEXT(0x00, 0, 12, 128, global.pk.type_str),
  UI_TEXT(0x00, 0, 26, 128, global.pk.key_str),
};

static unsigned int ui_pubkey_approve_button(unsigned int button_mask, unsigned int button_mask_counter) {
  uint16_t tx = 0;
  group public_key;
  scalar priv_key;
  switch (button_mask) {
  case BUTTON_EVT_RELEASED | BUTTON_LEFT: // REJECT
    io_exchange_with_code(SW_USER_REJECTED, 0);
    ui_idle();
    break;

  case BUTTON_EVT_RELEASED | BUTTON_RIGHT: // APPROVE
    generate_keypair(ctx->key_index, &public_key, priv_key);
    os_memmove(G_io_apdu_buffer + tx, &public_key, group_bytes);
    tx += group_bytes;
    io_exchange_with_code(SW_OK, tx);
    os_memmove(ctx->type_str, "Compare:", 9);
    // hash pk to display (192B is too much to meaningfully compare)
    unsigned char hash[32];
    cx_hash_sha256(public_key.x, 96, hash);
    bin2hex(ctx->full_str, hash, 32);
    os_memmove(ctx->partial_str, ctx->full_str, 12);
    ctx->partial_str[12] = '\0';
    ctx->display_index = 0;
    UX_DISPLAY(ui_pubkey_compare, ui_prepro_pubkey_compare);
    break;
  }
  return 0;
}

void handle_pubkey(uint8_t p1, uint8_t p2, uint8_t *data_buffer, uint16_t data_length, volatile unsigned int *flags, volatile unsigned int *tx) {

  ctx->key_index = U4LE(data_buffer, 0);
  os_memmove(ctx->type_str, "Generate Public", 16);
  os_memmove(ctx->key_str, "Key #", 5);
  int n = bin2dec(ctx->key_str + 5, ctx->key_index);
  os_memmove(ctx->key_str + 5 + n, "?", 2);
  UX_DISPLAY(ui_pubkey_approve, NULL);

  *flags |= IO_ASYNCH_REPLY;
}
