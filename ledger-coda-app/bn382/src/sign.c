/*
 * The MIT License (MIT)

 * Copyright (c) 2018 Nebulous Inc.

 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <os.h>
#include <os_io_seproxyhal.h>
#include "crypto.h"
#include "ux.h"

static signature_context *ctx = &global.s;

// Elements of a screen are declared const, so their fields cannot be
// modified at runtime. In other words, we can change the *contents* of
// the text buffer, but we cannot change the *pointer* to the buffer, and
// thus we cannot resize it. (This is also why we cannot write
// ctx->indexStr: ctx is not const.) So it is important to ensure that
// the buffer will be large enough to hold any string we want to display.
static const bagl_element_t ui_sign_approve[] = {
  UI_BACKGROUND(),
  UI_ICON_LEFT(0x00, BAGL_GLYPH_ICON_CROSS),
  UI_ICON_RIGHT(0x00, BAGL_GLYPH_ICON_CHECK),
  UI_TEXT(0x00, 0, 12, 128, "Sign this message"),
  UI_TEXT(0x00, 0, 26, 128, global.s.index_str),
};

static unsigned int ui_sign_approve_button(unsigned int button_mask, unsigned int button_mask_counter) {
  switch (button_mask) {
  case BUTTON_EVT_RELEASED | BUTTON_LEFT: // REJECT
    // Send an error code to the computer. The application on the computer
    // should recognize this code and display a "user refused to sign"
    // message instead of a generic error.
    io_exchange_with_code(SW_USER_REJECTED, 0);
    // Return to the main screen.
    ui_idle();
    break;

  case BUTTON_EVT_RELEASED | BUTTON_RIGHT: // APPROVE
    {
    affine public_key;
    scalar private_key;
    generate_keypair(ctx->key_index, &public_key, private_key);
    sign(G_io_apdu_buffer, G_io_apdu_buffer + field_bytes, &public_key, private_key, ctx->msg, ctx->msg + 96);
    // Send the data in the APDU buffer, along with a special code that
    // indicates approval. 192 is the number of bytes in the response APDU,
    // sans response code. The Ledger can only handle sending less than 260
    // bytes total per message.
    io_exchange_with_code(SW_OK, 192);
    // Return to the main screen.
    ui_idle();
    }
    break;
  }
  return 0;
}

static const bagl_element_t ui_sign_compare[] = {
  UI_BACKGROUND(),

  // Left and right buttons for scrolling the text. The 0x01 and 0x02 are
  // called userids; they allow the preprocessor (below) to know which
  // element it's examining.
  UI_ICON_LEFT(0x01, BAGL_GLYPH_ICON_LEFT),
  UI_ICON_RIGHT(0x02, BAGL_GLYPH_ICON_RIGHT),

  // Note that the userid of these fields is 0: this is a convention that
  // most apps use to indicate that the element should always be displayed.
  // UI_BACKGROUND() also has userid == 0. And if you revisit the approval
  // screen, you'll see that all of those elements have userid == 0 as well.
  UI_TEXT(0x00, 0, 12, 128, "Compare msg:"),
  UI_TEXT(0x00, 0, 26, 128, global.s.partial_msg_str),
};

static const bagl_element_t* ui_prepro_sign_compare(const bagl_element_t *element) {
  switch (element->component.userid) {
  case 1:
    // 0x01 is the left icon (see screen definition above), so return NULL
    // if we're displaying the beginning of the text.
    return (ctx->display_index == 0) ? NULL : element;
  case 2:
    // 0x02 is the right, so return NULL if we're displaying the end of the text.
    return (ctx->display_index == sizeof(ctx->hex_msg)-12) ? NULL : element;
  default:
    // Always display all other elements.
    return element;
  }
}

// This is the button handler for the comparison screen. Unlike the approval
// button handler, this handler doesn't send any data to the computer.
static unsigned int ui_sign_compare_button(unsigned int button_mask, unsigned int button_mask_counter) {
  switch (button_mask) {
  // The available button mask values are LEFT, RIGHT, EVT_RELEASED, and
  // EVT_FAST. EVT_FAST is set when a button is held for 8 "ticks," i.e.
  // 800ms.
  case BUTTON_LEFT:
  case BUTTON_EVT_FAST | BUTTON_LEFT: // SEEK LEFT
    // Decrement the display_index when the left button is pressed (or held).
    if (ctx->display_index > 0) {
      ctx->display_index--;
    }
    os_memmove(ctx->partial_msg_str, ctx->hex_msg+ctx->display_index, 12);
    UX_REDISPLAY();
    break;

  case BUTTON_RIGHT:
  case BUTTON_EVT_FAST | BUTTON_RIGHT: // SEEK RIGHT
    if (ctx->display_index < sizeof(ctx->hex_msg)-12) {
      ctx->display_index++;
    }
    os_memmove(ctx->partial_msg_str, ctx->hex_msg+ctx->display_index, 12);
    UX_REDISPLAY();
    break;

  case BUTTON_EVT_RELEASED | BUTTON_LEFT | BUTTON_RIGHT: // PROCEED
    // Prepare to display the approval screen by printing the key index
    // into the index_str buffer. We copy two bytes in the final os_memmove
    // so as to include the terminating '\0' byte for the string.
    os_memmove(ctx->index_str, "with Key #", 10);
    int n = bin2dec(ctx->index_str+10, ctx->key_index);
    os_memmove(ctx->index_str+10+n, "?", 2);
    // Note that because the approval screen does not have a preprocessor,
    // we must pass NULL.
    UX_DISPLAY(ui_sign_approve, NULL);
    break;
  }
  // (The return value of a button handler is irrelevant; it is never
  // checked.)
  return 0;
}

void handle_sign(uint8_t p1, uint8_t p2, uint8_t *data_buffer, uint16_t data_length, volatile unsigned int *flags, volatile unsigned int *tx) {
  // Read the index of the signing key. U4LE is a helper macro for
  // converting a 4-byte buffer to a uint32_t.
  ctx->key_index = U4LE(data_buffer, 0);
  os_memmove(ctx->msg, data_buffer+4, sizeof(ctx->msg));

  // Prepare to display the comparison screen by converting the msg to hex
  // and moving the first 12 characters into the partial_msg_str buffer.
  bin2hex(ctx->hex_msg, ctx->msg, sizeof(ctx->msg));
  os_memmove(ctx->partial_msg_str, ctx->hex_msg, 12);
  ctx->partial_msg_str[12] = '\0';
  ctx->display_index = 0;

  // Call UX_DISPLAY to display the comparison screen, passing the
  // corresponding preprocessor. You might ask: Why doesn't UX_DISPLAY
  // also take the button handler as an argument, instead of using macro
  // magic? To which I can only reply: ¯\_(ツ)_/¯
  UX_DISPLAY(ui_sign_compare, ui_prepro_sign_compare);

  // Set the IO_ASYNC_REPLY flag. we aren't sending data to the computer
  // immediately; we need to wait for a button press first.
  *flags |= IO_ASYNCH_REPLY;
}
