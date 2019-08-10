#include "os.h"
#include "os_io_seproxyhal.h"

#include "keys.h"
#include "ui.h"

ux_state_t ux;

static const ux_menu_entry_t menu_top[];
static const ux_menu_entry_t menu_about[];

static const ux_menu_entry_t menu_about[] = {
    {NULL, NULL, 0, NULL, "Version", APPVERSION, 0, 0},
    {menu_top, NULL, 1, NULL, "Back", NULL, 0, 0},
    UX_MENU_END};

static const ux_menu_entry_t menu_top[] = {
    {NULL, ui_address, 0, NULL, "Address", NULL, 0, 0},
    {menu_about, NULL, 0, NULL, "About", NULL, 0, 0},
    {NULL, os_sched_exit, 0, NULL, "Quit app", NULL, 0, 0},
    UX_MENU_END};

void ui_idle() { UX_MENU_DISPLAY(0, menu_top, NULL); }

#define MAX_CHARS_PER_LINE 8 // for testing

static char text[256];
static int line_buffer_pos;

char line_buffer[MAX_CHARS_PER_LINE + 2 + 1];

int ui_text_more() {
  int line_pos;

  if (text[line_buffer_pos] == '\0') {
    line_buffer[0] = '\0';
    return 0;
  }

  for (line_pos = 0; line_pos < MAX_CHARS_PER_LINE; line_pos++) {
    if (text[line_buffer_pos] == '\0') {
      break;
    }

    if (text[line_buffer_pos] == '\n') {
      line_buffer_pos++;
      break;
    }

    line_buffer[line_pos] = text[line_buffer_pos];
    line_buffer_pos++;
  }

  if (text[line_buffer_pos] != '\0') {
    line_buffer[line_pos++] = '.';
    line_buffer[line_pos++] = '.';
  }

  line_buffer[line_pos++] = '\0';
  return 1;
}

void ui_text_put(const char *msg) {
  for (unsigned int i = 0; i < sizeof(text); i++) {
    text[i] = msg[i];

    if (msg[i] == '\0') {
      break;
    }
  }

  text[sizeof(text) - 1] = '\0';
  line_buffer_pos = 0;

  PRINTF("ui_text_put: text %s\n", &text[0]);

}
