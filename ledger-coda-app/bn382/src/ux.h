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

// exception codes
#define SW_DEVELOPER_ERR 0x6B00
#define SW_INVALID_PARAM 0x6B01
#define SW_IMPROPER_INIT 0x6B02
#define SW_USER_REJECTED 0x6985
#define SW_OK            0x9000

// macros for converting raw bytes to uint64_t
#define U8BE(buf, off) (((uint64_t)(U4BE(buf, off))     << 32) | ((uint64_t)(U4BE(buf, off + 4)) & 0xFFFFFFFF))
#define U8LE(buf, off) (((uint64_t)(U4LE(buf, off + 4)) << 32) | ((uint64_t)(U4LE(buf, off))     & 0xFFFFFFFF))

#define UI_BACKGROUND() {{BAGL_RECTANGLE,0,0,0,128,32,0,0,BAGL_FILL,0,0xFFFFFF,0,0},NULL,0,0,0,NULL,NULL,NULL}
#define UI_ICON_LEFT(userid, glyph) {{BAGL_ICON,userid,3,12,7,7,0,0,0,0xFFFFFF,0,0,glyph},NULL,0,0,0,NULL,NULL,NULL}
#define UI_ICON_RIGHT(userid, glyph) {{BAGL_ICON,userid,117,13,8,6,0,0,0,0xFFFFFF,0,0,glyph},NULL,0,0,0,NULL,NULL,NULL}
#define UI_TEXT(userid, x, y, w, text) {{BAGL_LABELINE,userid,x,y,w,12,0,0,0,0xFFFFFF,0,BAGL_FONT_OPEN_SANS_REGULAR_11px|BAGL_FONT_ALIGNMENT_CENTER,0},(char *)text,0,0,0,NULL,NULL,NULL}

// ux is a magic global variable implicitly referenced by the UX_ macros.
// Apps should never need to reference it directly.
extern ux_state_t ux;

// txn_decoder_state_e indicates a transaction decoder status
typedef enum {
    TXN_STATE_ERR = 1,  // invalid transaction (NOTE: it's illegal to THROW(0))
    TXN_STATE_PARTIAL,  // no elements have been fully decoded yet
    TXN_STATE_READY,    // at least one element is fully decoded
    TXN_STATE_FINISHED, // reached end of transaction
} txn_decoder_state_e;

// txn_elem_type_e indicates a transaction element type.
typedef enum {
  TXN_ELEM_IS_DELEGATION,
  TXN_ELEM_NONCE,
  TXN_ELEM_FROM,
  TXN_ELEM_TO,
  TXN_ELEM_AMOUNT,
  TXN_ELEM_FEE,
  TXN_ELEM_MEMO,
} txn_elem_type_e;

#define CVAL_LEN 128

// txn_state is a helper object for computing the hash of a streamed
// transaction.
typedef struct {
  uint8_t buf[510];           // holds raw tx bytes; large enough for two 0xFF reads
  uint16_t buf_len;           // number of valid bytes in buf
  uint16_t pos;               // mid-decode offset; reset to 0 after each elem

  txn_elem_type_e elem_type;  // type of most-recently-seen element
  uint64_t slice_len;         // most-recently-seen slice length prefix
  uint16_t slice_index;       // offset within current element slice

  uint16_t sig_index;         // index of TxnSig being computed
  scalar hash_state[3];       // buffer to hold intermediate hash state
  scalar hash;                // buffer to hold final hash

  uint8_t del;                // is delegation : 1 = true, 0 = false
  uint32_t nonce;             // transaction nonce
  uint8_t out_val[CVAL_LEN];  // currency value, in decimal
  uint8_t val_len;            // length of out_val
  uint8_t fee_val[CVAL_LEN];  // currency value, in decimal
  uint32_t key_index;         // 'from' public key
  scalar out_key;             // 'to' public key
} txn_state;

// txn_init initializes a transaction decoder, preparing it to calculate the
// requested hash.
void txn_init(txn_state *txn, uint16_t sig_index);

// txn_update adds data to a transaction decoder.
void txn_update(txn_state *txn, uint8_t *in, uint8_t inlen);

// txn_next_elem decodes the next element of the transaction. If the element
// is ready for display, txn_next_elem returns TXN_STATE_READY. If more data
// is required, it returns TXN_STATE_PARTIAL. If a decoding error is
// encountered, it returns TXN_STATE_ERR. If the transaction has been fully
// decoded, it returns TXN_STATE_FINISHED.
txn_decoder_state_e txn_next_elem(txn_state *txn);

#define CSTR_LEN 40
#define CPART_LEN 13 // display 12 chars, + 1 for NULL
#define KHEX_LEN (field_bytes * 2 * 2)
#define SMSG_LEN (field_bytes * 2 * 2)
#define SHEX_LEN (field_bytes * 2 * 2 * 2)

typedef struct {
  uint32_t key_index;
  uint8_t display_index;
  uint8_t type_str[CSTR_LEN];
  uint8_t key_str[CSTR_LEN];
  uint8_t full_str[KHEX_LEN];
  uint8_t partial_str[CPART_LEN];
} pubkey_context;

typedef struct {
  uint32_t key_index;
  uint8_t msg[SMSG_LEN];
  uint8_t hex_msg[SHEX_LEN];
  uint32_t display_index;
  uint8_t index_str[CSTR_LEN];
  uint8_t partial_msg_str[CPART_LEN];
} signature_context;

// TODO get rid of dead code :)
// this is never used -- we dont hash messages before signing
typedef struct {
  uint32_t key_index;
  int sign;
  uint8_t elem_len;
  uint8_t display_index;
  uint8_t elem_part; // screen index of elements
  txn_state txn;
  uint8_t label_str[CSTR_LEN];
  uint8_t full_str[128];
  uint8_t partial_str[CPART_LEN];
  int initialized; // protects against certain attacks
} hash_context;

// To save memory, we store all the context types in a single global union,
// taking advantage of the fact that only one command is executed at a time.
typedef union {
  pubkey_context pk;
  signature_context s;
  hash_context h;
} command_context;
extern command_context global;

void ui_idle(void);
void io_exchange_with_code(uint16_t code, uint16_t tx);
void bin2hex(uint8_t *dst, uint8_t *data, uint64_t inlen);
int bin2dec(uint8_t *dst, uint64_t n);
