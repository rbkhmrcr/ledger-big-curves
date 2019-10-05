// exception codes
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

typedef struct {
  uint32_t key_index;
  uint8_t display_index;
  // NUL-terminated strings for display
  uint8_t type_str[40];
  uint8_t key_str[40];
  uint8_t full_str[77];
  uint8_t partial_str[13];
} pubkey_context;

typedef struct {
  uint32_t key_index;
  uint8_t hash[32];
  uint8_t hex_hash[64];
  uint8_t display_index;
  // NUL-terminated strings for display
  uint8_t index_str[40];
  uint8_t partial_hash_str[13];
} signature_context;

// To save memory, we store all the context types in a single global union,
// taking advantage of the fact that only one command is executed at a time.
typedef union {
  pubkey_context pk;
  signature_context s;
} command_context;
extern command_context global;

void ui_idle(void);
void io_exchange_with_code(uint16_t code, uint16_t tx);
void bin2hex(uint8_t *dst, uint8_t *data, uint64_t inlen);
int bin2dec(uint8_t *dst, uint64_t n);
