/*
 * The MIT License (MIT)
 *
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

static hash_context *ctx = &global.h;

static const bagl_element_t ui_hash_compare[] = {
  UI_BACKGROUND(),
  UI_ICON_LEFT(0x01, BAGL_GLYPH_ICON_LEFT),
  UI_ICON_RIGHT(0x02, BAGL_GLYPH_ICON_RIGHT),
  UI_TEXT(0x00, 0, 12, 128, "Compare Hash:"),
  UI_TEXT(0x00, 0, 26, 128, global.h.partial_str),
};

static const bagl_element_t* ui_prepro_hash_compare(const bagl_element_t *element) {
  if ((element->component.userid == 1 && ctx->display_index == 0) ||
      (element->component.userid == 2 && ctx->display_index == ctx->elem_len-12)) {
    return NULL;
  }
  return element;
}

#define ZEROS 24

int format(uint8_t *buf, uint8_t dec_len) {
  if (dec_len < ZEROS + 1) {
    // if < 1, pad with leading zeros
    os_memmove(buf + (ZEROS - dec_len) + 2, buf, dec_len+1);
    os_memset(buf, '0', ZEROS + 2 - dec_len);
    dec_len = ZEROS + 1;
  } else {
    os_memmove(buf + (dec_len - ZEROS)+2, buf + (dec_len - ZEROS + 1), ZEROS + 1);
  }
  // add decimal point, trim trailing zeros, and add units
  buf[dec_len - ZEROS] = '.';
  while (dec_len > 0 && buf[dec_len] == '0') {
    dec_len--;
  }
  if (buf[dec_len] == '.') {
    dec_len--;
  }
  os_memmove(buf + dec_len + 1, " CODA", 6);
  return dec_len + 6;
}


static unsigned int ui_hash_compare_button(unsigned int button_mask, unsigned int button_mask_counter) {
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
    if (ctx->display_index < ctx->elem_len-12) {
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

static const bagl_element_t ui_hash_sign[] = {
  UI_BACKGROUND(),
  UI_ICON_LEFT(0x00, BAGL_GLYPH_ICON_CROSS),
  UI_ICON_RIGHT(0x00, BAGL_GLYPH_ICON_CHECK),
  UI_TEXT(0x00, 0, 12, 128, "Sign this Txn"),
  UI_TEXT(0x00, 0, 26, 128, global.h.full_str), // "with Key #123?"
};

static unsigned int ui_hash_sign_button(unsigned int button_mask, unsigned int button_mask_counter) {
  switch (button_mask) {
  // REJECT
  case BUTTON_EVT_RELEASED | BUTTON_LEFT:
    io_exchange_with_code(SW_USER_REJECTED, 0);
    ui_idle();
    break;

  // APPROVE
  case BUTTON_EVT_RELEASED | BUTTON_RIGHT: {
    field sk;
    group pk;
    generate_keypair(ctx->key_index, &pk, sk);
    sign(G_io_apdu_buffer, G_io_apdu_buffer + field_bytes, &pk, sk, ctx->txn.hash);
    io_exchange_with_code(SW_OK, field_bytes + scalar_bytes);
    ui_idle();
    }
    break;
  }
  return 0;
}

static const bagl_element_t ui_hash_elem[] = {
  UI_BACKGROUND(),
  UI_ICON_LEFT(0x01, BAGL_GLYPH_ICON_LEFT),
  UI_ICON_RIGHT(0x02, BAGL_GLYPH_ICON_RIGHT),
  UI_TEXT(0x00, 0, 12, 128, global.h.label_str),
  UI_TEXT(0x00, 0, 26, 128, global.h.partial_str),
};

static const bagl_element_t* ui_prepro_hash_elem(const bagl_element_t *element) {
  if ((element->component.userid == 1 && ctx->display_index == 0) ||
      (element->component.userid == 2 && ((ctx->elem_len < 12) || (ctx->display_index == ctx->elem_len-12)))) {
    return NULL;
  }
  return element;
}

// This is a helper function that prepares an element of the transaction for
// display. It stores the type of the element in label_str, and a human-
// readable representation of the element in full_str. As in previous screens,
// partial_str holds the visible portion of full_str.
static void format_txn_elem(hash_context *ctx) {
  txn_state *txn = &ctx->txn;

  switch (txn->elem_type) {
  case TXN_ELEM_TO:
    os_memmove(ctx->label_str, "Output #", 16);
    bin2dec(ctx->label_str+16, txn->slice_index);
    if (ctx->elem_part == 0) {
      os_memmove(ctx->full_str, txn->out_key, sizeof(txn->out_key));
      os_memmove(ctx->partial_str, ctx->full_str, 12);
      ctx->elem_len = 76;
      ctx->elem_part++;
    } else {
      os_memmove(ctx->full_str, txn->out_val, sizeof(txn->out_val));
      ctx->elem_len = format(ctx->full_str, txn->val_len);
      os_memmove(ctx->partial_str, ctx->full_str, 12);
      ctx->elem_part = 0;
    }
    break;

  case TXN_ELEM_FEE:
    // Miner fees only have one part.
    os_memmove(ctx->label_str, "Miner Fee #", 11);
    bin2dec(ctx->label_str+11, txn->slice_index);
    os_memmove(ctx->full_str, txn->out_val, sizeof(txn->out_val));
    ctx->elem_len = format(ctx->full_str, txn->val_len);
    os_memmove(ctx->partial_str, ctx->full_str, 12);
    ctx->elem_part = 0;
    break;

  default:
    // This should never happen.
    io_exchange_with_code(SW_DEVELOPER_ERR, 0);
    ui_idle();
    break;
  }

  // Regardless of what we're displaying, the display_index should always be
  // reset to 0, because we're displaying the beginning of full_str.
  ctx->display_index = 0;
}

static unsigned int ui_hash_elem_button(unsigned int button_mask, unsigned int button_mask_counter) {
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
    if (ctx->display_index < ctx->elem_len-12) {
      ctx->display_index++;
    }
    os_memmove(ctx->partial_str, ctx->full_str+ctx->display_index, 12);
    UX_REDISPLAY();
    break;

  case BUTTON_EVT_RELEASED | BUTTON_LEFT | BUTTON_RIGHT: // PROCEED
    if (ctx->elem_part > 0) {
      // We're in the middle of displaying a multi-part element; display
      // the next part.
      format_txn_elem(ctx);
      UX_REDISPLAY();
      break;
    }
    // Attempt to decode the next element in the transaction.
    switch (txn_next_elem(&ctx->txn)) {
    case TXN_STATE_ERR:
      // The transaction is invalid.
      io_exchange_with_code(SW_INVALID_PARAM, 0);
      ui_idle();
      break;
    case TXN_STATE_PARTIAL:
      // We don't have enough data to decode the next element; send an
      // OK code to request more.
      io_exchange_with_code(SW_OK, 0);
      break;
    case TXN_STATE_READY:
      // We successively decoded one or more elements; display the first
      // part of the first element.
      ctx->elem_part = 0;
      format_txn_elem(ctx);
      UX_REDISPLAY();
      break;
    case TXN_STATE_FINISHED:
      // We've finished decoding the transaction, and all elements have
      // been displayed.
      if (ctx->sign) {
        // If we're signing the transaction, prepare and display the
        // approval screen.
        os_memmove(ctx->full_str, "with Key #", 10);
        os_memmove(ctx->full_str+10+(bin2dec(ctx->full_str+10, ctx->key_index)), "?", 2);
        UX_DISPLAY(ui_hash_sign, NULL);
      } else {
        // If we're just computing the hash, send it immediately and
        // display the comparison screen
        os_memmove(G_io_apdu_buffer, ctx->txn.hash, 32);
        io_exchange_with_code(SW_OK, 32);
        bin2hex(ctx->full_str, ctx->txn.hash, sizeof(ctx->txn.hash));
        os_memmove(ctx->partial_str, ctx->full_str, 12);
        ctx->elem_len = 64;
        ctx->display_index = 0;
        UX_DISPLAY(ui_hash_compare, ui_prepro_hash_compare);
      }
      // Reset the initialization state.
      ctx->initialized = 0;
      break;
    }
    break;
  }
  return 0;
}

// APDU parameters
#define P1_FIRST        0x00 // 1st packet of multi-packet transfer
#define P1_MORE         0x80 // nth packet of multi-packet transfer
#define P2_DISPLAY_HASH 0x00 // display transaction hash
#define P2_SIGN_HASH    0x01 // sign transaction hash

// handle_hash reads a signature index and a transaction, calculates the
// SigHash of the transaction, and optionally signs the hash using a specified
// key. The transaction is processed in a streaming fashion and displayed
// piece-wise to the user.
void handle_hash(uint8_t p1, uint8_t p2, uint8_t *data_buffer, uint16_t data_length, volatile unsigned int *flags, volatile unsigned int *tx) {
  if ((p1 != P1_FIRST && p1 != P1_MORE) || (p2 != P2_DISPLAY_HASH && p2 != P2_SIGN_HASH)) {
    THROW(SW_INVALID_PARAM);
  }

  if (p1 == P1_FIRST) {
    // If this is the first packet of a transaction, the transaction
    // context must not already be initialized. (Otherwise, an attacker
    // could fool the user by concatenating two transactions.)
    //
    // NOTE: ctx->initialized is set to false when the Coda app loads.
    if (ctx->initialized) {
      THROW(SW_IMPROPER_INIT);
    }
    ctx->initialized = 1;

    // If this is the first packet, it will include the key index and sig
    // index in addition to the transaction data. Use these to initialize
    // the ctx and the transaction decoder.
    ctx->key_index = U4LE(data_buffer, 0); // NOTE: ignored if !ctx->sign
    data_buffer += 4; data_length -= 4;
    uint16_t sig_index = U2LE(data_buffer, 0);
    data_buffer += 2; data_length -= 2;
    txn_init(&ctx->txn, sig_index);

    // Set ctx->sign according to P2.
    ctx->sign = (p2 & P2_SIGN_HASH);

    // Initialize some display-related variables.
    ctx->partial_str[12] = '\0';
    ctx->elem_part = ctx->elem_len = ctx->display_index = 0;
  } else {
    // If this is not P1_FIRST, the transaction must have been
    // initialized previously.
    if (!ctx->initialized) {
      THROW(SW_IMPROPER_INIT);
    }
  }

  // Add the new data to transaction decoder.
  txn_update(&ctx->txn, data_buffer, data_length);

  // Attempt to decode the next element of the transaction. Note that this
  // code is essentially identical to ui_hash_elem_button. Sadly,
  // there doesn't seem to be a clean way to avoid this duplication.
  switch (txn_next_elem(&ctx->txn)) {
  case TXN_STATE_ERR:
    THROW(SW_INVALID_PARAM);
  case TXN_STATE_PARTIAL:
    THROW(SW_OK);
  case TXN_STATE_READY:
    ctx->elem_part = 0;
    format_txn_elem(ctx);
    UX_DISPLAY(ui_hash_elem, ui_prepro_hash_elem);
    *flags |= IO_ASYNCH_REPLY;
    break;
  case TXN_STATE_FINISHED:
    if (ctx->sign) {
      os_memmove(ctx->full_str, "with Key #", 10);
      bin2dec(ctx->full_str+10, ctx->key_index);
      os_memmove(ctx->full_str+10+(bin2dec(ctx->full_str+10, ctx->key_index)), "?", 2);
      UX_DISPLAY(ui_hash_sign, NULL);
      *flags |= IO_ASYNCH_REPLY;
    } else {
      os_memmove(G_io_apdu_buffer, ctx->txn.hash, 32);
      io_exchange_with_code(SW_OK, 32);
      bin2hex(ctx->full_str, ctx->txn.hash, sizeof(ctx->txn.hash));
      os_memmove(ctx->partial_str, ctx->full_str, 12);
      ctx->elem_len = 64;
      ctx->display_index = 0;
      UX_DISPLAY(ui_hash_compare, ui_prepro_hash_compare);
      // The above code does something strange: it calls io_exchange
      // directly from the command handler. You might wonder: why not
      // just prepare the APDU buffer and let coda_main call io_exchange?
      // The answer, surprisingly, is that we also need to call
      // UX_DISPLAY, and UX_DISPLAY affects io_exchange in subtle ways.
      // To understand why, we'll need to dive deep into the Nano S
      // firmware. I recommend that you don't skip this section, even
      // though it's lengthy, because it will save you a lot of
      // frustration when you go "off the beaten path" in your own app.
      //
      // Recall that the Nano S has two chips. Your app (and the Ledger
      // OS, BOLOS) runs on the Secure Element. The SE is completely
      // self-contained; it doesn't talk to the outside world at all. It
      // only talks to the other chip, the MCU. The MCU is what
      // processes button presses, renders things on screen, and
      // exchanges APDU packets with the computer. The communication
      // layer between the SE and the MCU is called SEPROXYHAL. There
      // are some nice diagrams in the "Hardware Architecture" section
      // of Ledger's docs that will help you visualize all this.
      //
      // The SEPROXYHAL protocol, like any communication protocol,
      // specifies exactly when each party is allowed to talk.
      // Communication happens in a loop: first the MCU sends an Event,
      // then the SE replies with zero or more Commands, and finally the
      // SE sends a Status to indicate that it has finished processing
      // the Event, completing one iteration:
      //
      //    Event -> Commands -> Status -> Event -> Commands -> ...
      //
      // For our purposes, an "Event" is a request APDU, and a "Command"
      // is a response APDU. (There are other types of Events and
      // Commands, such as button presses, but they aren't relevant
      // here.) As for the Status, there is a "General" Status and a
      // "Display" Status. A General Status tells the MCU to send the
      // response APDU, and a Display Status tells it to render an
      // element on the screen. Remember, it's "zero or more Commands,"
      // so it's legal to send just a Status without any Commands.
      //
      // You may have some picture of the problem now. Imagine we
      // prepare the APDU buffer, then call UX_DISPLAY, and then let
      // coda_main send the APDU with io_exchange. What happens at the
      // SEPROXYHAL layer? First, UX_DISPLAY will send a Display Status.
      // Then, io_exchange will send a Command and a General Status. But
      // no Event was processed between the two Statuses! This causes
      // SEPROXYHAL to freak out and crash, forcing you to reboot your
      // Nano S.
      //
      // So why does calling io_exchange before UX_DISPLAY fix the
      // problem? Won't we just end up sending two Statuses again? The
      // secret is that io_exchange_with_code uses the
      // IO_RETURN_AFTER_TX flag. Previously, the only thing we needed
      // to know about IO_RETURN_AFTER_TX is that it sends a response
      // APDU without waiting for the next request APDU. But it has one
      // other important property: it tells io_exchange not to send a
      // Status! So the only Status we send comes from UX_DISPLAY. This
      // preserves the ordering required by SEPROXYHAL.
      //
      // Lastly: what if we prepare the APDU buffer in the handler, but
      // with the IO_RETURN_AFTER_TX flag set? Will that work?
      // Unfortunately not. io_exchange won't send a status, but it
      // *will* send a Command containing the APDU, so we still end up
      // breaking the correct SEPROXYHAL ordering.
      //
      // Here's a list of rules that will help you debug similar issues:
      //
      // - Always preserve the order: Event -> Commands -> Status
      // - UX_DISPLAY sends a Status
      // - io_exchange sends a Command and a Status
      // - IO_RETURN_AFTER_TX makes io_exchange not send a Status
      // - IO_ASYNCH_REPLY (or tx=0) makes io_exchange not send a Command
      //
      // Okay, that second rule isn't 100% accurate. UX_DISPLAY doesn't
      // necessarily send a single Status: it sends a separate Status
      // for each element you render! The reason this works is that the
      // MCU replies to each Display Status with a Display Processed
      // Event. That means you can call UX_DISPLAY many times in a row
      // without disrupting SEPROXYHAL. Anyway, as far as we're
      // concerned, it's simpler to think of UX_DISPLAY as sending just
      // a single Status.
    }
    // Reset the initialization state.
    ctx->initialized = 0;
    break;
  }
}
