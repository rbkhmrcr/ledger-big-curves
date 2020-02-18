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

#include <stdint.h>
#include <os.h>
#include <os_io_seproxyhal.h>
#include "crypto.h"
#include "poseidon.h"
#include "ux.h"

static void divWW10(uint64_t u1, uint64_t u0, uint64_t *q, uint64_t *r) {
  const uint64_t s = 60ULL;
  const uint64_t v = 11529215046068469760ULL;
  const uint64_t vn1 = 2684354560ULL;
  const uint64_t _B2 = 4294967296ULL;
  uint64_t un32 = u1<<s | u0>>(64-s);
  uint64_t un10 = u0 << s;
  uint64_t un1 = un10 >> 32;
  uint64_t un0 = un10 & (_B2-1);
  uint64_t q1 = un32 / vn1;
  uint64_t rhat = un32 - q1*vn1;

  while (q1 >= _B2) {
    q1--;
    rhat += vn1;
    if (rhat >= _B2) {
      break;
    }
  }

  uint64_t un21 = un32*_B2 + un1 - q1*v;
  uint64_t q0 = un21 / vn1;
  rhat = un21 - q0*vn1;

  while (q0 >= _B2) {
    q0--;
    rhat += vn1;
    if (rhat >= _B2) {
      break;
    }
  }

  *q = q1*_B2 + q0;
  *r = (un21*_B2 + un0 - q0*v) >> s;
}

static uint64_t quorem10(uint64_t nat[], int len) {
  uint64_t r = 0;
  for (int i = len - 1; i >= 0; i--) {
    divWW10(r, nat[i], &nat[i], &r);
  }
  return r;
}

// what is a currency string?
// cur2dec converts a currency value to a decimal string and
// appends a final NUL byte. It returns the length of the string. If the value
// is too large, it throws TXN_STATE_ERR.
static int cur2dec(uint8_t *out, uint8_t *cur) {
  if (cur[0] == 0) {
    out[0] = '\0';
    return 0;
  }

  // sanity check the size of the value. The size (in bytes) is given in the
  // first byte; it should never be greater than 18 (18 bytes = 144 bits)
  if (cur[0] > 18) {
    THROW(TXN_STATE_ERR);
  }

  // convert big-endian uint8_t[] to little-endian uint64_t[]
  //
  // NOTE: the encoding omits any leading zeros, so the first "uint64"
  // may not be a full 8 bytes. We handle this by treating the length prefix
  // as part of the first uint64. This is safe as long as the length prefix
  // has only 1 non-zero byte, which should be enforced elsewhere.
  uint64_t nat[32];
  int len = (cur[0] / 8) + ((cur[0] % 8) != 0);
  cur += 8 - (len*8 - cur[0]);
  for (int i = 0; i < len; i++) {
    nat[len-i-1] = U8BE(cur, i*8);
  }

  // decode digits into buf, right-to-left
  //
  // NOTE: buf must be large enough to hold the decimal representation of
  // 2^144, which has 44 digits.
  uint8_t buf[64];
  int i = sizeof(buf);
  buf[--i] = '\0';
  while (len > 0) {
    if (i <= 0) {
      THROW(TXN_STATE_ERR);
    }
    buf[--i] = '0' + quorem10(nat, len);
    // normalize nat
    while (len > 0 && nat[len-1] == 0) {
      len--;
    }
  }

  // copy buf->out, trimming whitespace
  os_memmove(out, buf+i, sizeof(buf)-i);
  return sizeof(buf)-i-1;
}

static void need_at_least(txn_state *txn, uint64_t n) {
  if ((txn->buf_len - txn->pos) < n) {
    THROW(TXN_STATE_PARTIAL);
  }
}

static void seek(txn_state *txn, uint64_t n) {
  need_at_least(txn, n);
  txn->pos += n;
}

static void advance(txn_state *txn) {
  // if elem is covered, add it to the hash
  // && txn->pos >= 96 below?
  if (txn->slice_index == txn->sig_index) {
    poseidon_2in(txn->hash_state, txn->buf, txn->buf + scalar_bytes);
  }

  txn->buf_len -= txn->pos;
  os_memmove(txn->buf, txn->buf+txn->pos, txn->buf_len);
  txn->pos = 0;
}

static uint64_t read_int(txn_state *txn) {
  need_at_least(txn, 8);
  uint64_t u = U8LE(txn->buf, txn->pos);
  seek(txn, 8);
  return u;
}

// should we not make the value have a fixed length?
static void read_currency(txn_state *txn, uint8_t *out_val) {
  uint64_t val_len = read_int(txn);
  need_at_least(txn, val_len);
  if (out_val) {
    txn->val_len = cur2dec(out_val, txn->buf+txn->pos-8);
  }
  seek(txn, val_len);
}

static void read_key(txn_state *txn) {
  need_at_least(txn, group_bytes);
  seek(txn, group_bytes);
}

// throws txn_decoder_state_e
static void __txn_next_elem(txn_state *txn) {
  // if we're on a slice boundary, read the next length prefix and bump the
  // element type
  PRINTF("%s:%d\n", __FILE__, __LINE__);
  PRINTF("slice len = %d\n", txn->slice_len);
  PRINTF("slice len = %d\n", txn->slice_len >> 32);
  PRINTF("txn read int = %16x\n", read_int(txn));
  PRINTF("txn read int = %16x\n", read_int(txn) >> 32);

  while (txn->slice_index == txn->slice_len) {
  PRINTF("%s:%d\n", __FILE__, __LINE__);
    if (txn->elem_type == TXN_ELEM_MEMO) {
      PRINTF("%s:%d\n", __FILE__, __LINE__);
      // store final hash -- XXX msg isn't hashed before signing so this doesn't make sense anymore
      poseidon_digest(txn->hash_state, txn->hash);
      THROW(TXN_STATE_FINISHED);
    }
    txn->slice_len = read_int(txn);
    txn->slice_index = 0;
    txn->elem_type++;
    advance(txn);

    if ((txn->elem_type == TXN_ELEM_MEMO) && (txn->sig_index >= txn->slice_len)) {
      THROW(TXN_STATE_ERR);
    }
  }

/*
 *  TXN_ELEM_IS_DELEGATION,
 *  TXN_ELEM_NONCE,
 *  TXN_ELEM_FROM,
 *  TXN_ELEM_TO,
 *  TXN_ELEM_AMOUNT,
 *  TXN_ELEM_FEE,
 *  TXN_ELEM_MEMO,
 */

  switch (txn->elem_type) {

  // these elements should be displayed

  case TXN_ELEM_IS_DELEGATION:
    PRINTF("%s:%d\n", __FILE__, __LINE__);
    read_int(txn);                // read the first element of txn, txn.del
    advance(txn);
    txn->slice_index++;
    THROW(TXN_STATE_READY);

  case TXN_ELEM_NONCE:
    PRINTF("%s:%d\n", __FILE__, __LINE__);
    read_int(txn);                // read the second element of txn, txn.nonce
    advance(txn);
    txn->slice_index++;
    THROW(TXN_STATE_READY);

  case TXN_ELEM_FROM:
    read_key(txn);                // read 'from' key : txn->in_key
    advance(txn);
    txn->slice_index++;
    THROW(TXN_STATE_READY);

  case TXN_ELEM_TO:
    read_key(txn);                // read 'to' key : txn->out_key
    advance(txn);
    txn->slice_index++;
    THROW(TXN_STATE_READY);

  case TXN_ELEM_AMOUNT:
    read_currency(txn, txn->out_val);
    advance(txn);
    txn->slice_index++;
    THROW(TXN_STATE_READY);

  case TXN_ELEM_FEE:
    read_currency(txn, txn->fee_val);
    advance(txn);
    txn->slice_index++;
    THROW(TXN_STATE_READY);

  // these elements are decoded, but not displayed
  // memo should not be present
  case TXN_ELEM_MEMO:
    if (txn->slice_len != 0) {
      THROW(TXN_STATE_ERR);
    }
    return;
  }
}

txn_decoder_state_e txn_next_elem(txn_state *txn) {
  // Like many transaction decoders, we use exceptions to jump out of deep
  // call stacks when we encounter an error. There are two important rules
  // for Ledger exceptions: declare modified variables as volatile, and do
  // not THROW(0). Presumably, 0 is the sentinel value for "no exception
  // thrown." So be very careful when throwing enums, since enums start at 0
  // by default.
  volatile txn_decoder_state_e result;
  BEGIN_TRY {
    TRY {
      // read until we reach a displayable element or the end of the buffer
      for (;;) {
        __txn_next_elem(txn);
      }
    }
    CATCH_OTHER(e) {
      result = e;
    }
    FINALLY {
    }
  }
  END_TRY;
  if (txn->buf_len + 255 > sizeof(txn->buf)) {
    // we filled the buffer to max capacity, but there still wasn't enough
    // to decode a full element. This generally means that the txn is
    // corrupt in some way, since elements shouldn't be very large.
    return TXN_STATE_ERR;
  }
  return result;
}

void txn_init(txn_state *txn, uint16_t sig_index) {
  os_memset(txn, 0, sizeof(txn_state));
  txn->buf_len = txn->pos = txn->slice_index = txn->slice_len = txn->val_len = 0;
  txn->elem_type = -1;
  txn->sig_index = sig_index;
}

void txn_update(txn_state *txn, uint8_t *in, uint8_t in_len) {
  // the buffer should never overflow; any elements should always be drained
  // before the next read.
  if (txn->buf_len + in_len > sizeof(txn->buf)) {
    THROW(SW_DEVELOPER_ERR);
  }

  // append to the buffer
  os_memmove(txn->buf + txn->buf_len, in, in_len);
  txn->buf_len += in_len;

  // reset the seek position; if we previously threw TXN_STATE_PARTIAL, now
  // we can try decoding again from the beginning.
  txn->pos = 0;
}
