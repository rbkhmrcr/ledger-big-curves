extern char line_buffer[];

void ui_idle();
void pubkey_ui();
void transaction_ui();

void ui_text_put(const char *msg);
int ui_text_more();
