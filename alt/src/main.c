/*******************************************************************************
*
*  (c) 2016 Ledger
*  (c) 2018 Nebulous
*
*  Licensed under the Apache License, Version 2.0 (the "License");
*  you may not use this file except in compliance with the License.
*  You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
*  Unless required by applicable law or agreed to in writing, software
*  distributed under the License is distributed on an "AS IS" BASIS,
*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*  See the License for the specific language governing permissions and
*  limitations under the License.
********************************************************************************/

#include <stdint.h>
#include <stdbool.h>
#include <os_io_seproxyhal.h>
#include "glyphs.h"
#include "blake2b.h"
#include "sia.h"
#include "ux.h"

commandContext global;
ux_state_t ux;
static const ux_menu_entry_t menu_main[];

static const ux_menu_entry_t menu_about[] = {
	{
		.menu     = NULL,    
		.callback = NULL,   
		.userid   = 0,     
		.icon     = NULL, 
		.line1    = "Version", 
		.line2    = APPVERSION,
		.text_x   = 0,        
		.icon_x   = 0,       
	},
	{menu_main, NULL, 0, &C_icon_back, "Back", NULL, 61, 40},
	UX_MENU_END,
};

static const ux_menu_entry_t menu_main[] = {
	{NULL, NULL, 0, NULL, "Waiting for", "commands...", 0, 0},
	{menu_about, NULL, 0, NULL, "About", NULL, 0, 0},
	{NULL, os_sched_exit, 0, &C_icon_dashboard, "Quit app", NULL, 50, 29},
	UX_MENU_END,
};

void ui_idle(void) {
	UX_MENU_DISPLAY(0, menu_main, NULL);
}

void io_exchange_with_code(uint16_t code, uint16_t tx) {
	G_io_apdu_buffer[tx++] = code >> 8;
	G_io_apdu_buffer[tx++] = code & 0xFF;
	io_exchange(CHANNEL_APDU | IO_RETURN_AFTER_TX, tx);
}

#define INS_GET_VERSION    0x01
#define INS_GET_PUBLIC_KEY 0x02
#define INS_SIGN_HASH      0x04
#define INS_GET_TXN_HASH   0x08

typedef void handler_fn_t(uint8_t p1, uint8_t p2, uint8_t *dataBuffer, uint16_t dataLength, volatile unsigned int *flags, volatile unsigned int *tx);

handler_fn_t handleGetVersion;
handler_fn_t handleGetPublicKey;
handler_fn_t handleSignHash;
handler_fn_t handleCalcTxnHash;

static handler_fn_t* lookupHandler(uint8_t ins) {
	switch (ins) {
	case INS_GET_VERSION:    return handleGetVersion;
	case INS_GET_PUBLIC_KEY: return handleGetPublicKey;
	case INS_SIGN_HASH:      return handleSignHash;
	case INS_GET_TXN_HASH:   return handleCalcTxnHash;
	default:                 return NULL;
	}
}

#define CLA          0xE0
#define OFFSET_CLA   0x00
#define OFFSET_INS   0x01
#define OFFSET_P1    0x02
#define OFFSET_P2    0x03
#define OFFSET_LC    0x04
#define OFFSET_CDATA 0x05

static void sia_main(void) {
	// Mark the transaction context as uninitialized.
	global.calcTxnHashContext.initialized = false;

	volatile unsigned int rx = 0;
	volatile unsigned int tx = 0;
	volatile unsigned int flags = 0;

	// Exchange APDUs until EXCEPTION_IO_RESET is thrown.
	for (;;) {
		volatile unsigned short sw = 0;

		BEGIN_TRY {
			TRY {
				rx = tx;
				tx = 0; // ensure no race in CATCH_OTHER if io_exchange throws an error
				rx = io_exchange(CHANNEL_APDU | flags, rx);
				flags = 0;

				// No APDU received; trigger a reset.
				if (rx == 0) {
					THROW(EXCEPTION_IO_RESET);
				}
				// Malformed APDU.
				if (G_io_apdu_buffer[OFFSET_CLA] != CLA) {
					THROW(0x6E00);
				}
				// Lookup and call the requested command handler.
				handler_fn_t *handlerFn = lookupHandler(G_io_apdu_buffer[OFFSET_INS]);
				if (!handlerFn) {
					THROW(0x6D00);
				}
				handlerFn(G_io_apdu_buffer[OFFSET_P1], G_io_apdu_buffer[OFFSET_P2],
				          G_io_apdu_buffer + OFFSET_CDATA, G_io_apdu_buffer[OFFSET_LC], &flags, &tx);
			}
			CATCH(EXCEPTION_IO_RESET) {
				THROW(EXCEPTION_IO_RESET);
			}
			CATCH_OTHER(e) {
				switch (e & 0xF000) {
				case 0x6000:
				case 0x9000:
					sw = e;
					break;
				default:
					sw = 0x6800 | (e & 0x7FF);
					break;
				}
				G_io_apdu_buffer[tx++] = sw >> 8;
				G_io_apdu_buffer[tx++] = sw & 0xFF;
			}
			FINALLY {
			}
		}
		END_TRY;
	}
}


// override point, but nothing more to do
void io_seproxyhal_display(const bagl_element_t *element) {
	io_seproxyhal_display_default((bagl_element_t *)element);
}

unsigned char G_io_seproxyhal_spi_buffer[IO_SEPROXYHAL_BUFFER_SIZE_B];

unsigned char io_event(unsigned char channel) {
	// can't have more than one tag in the reply, not supported yet.
	switch (G_io_seproxyhal_spi_buffer[0]) {
	case SEPROXYHAL_TAG_FINGER_EVENT:
		UX_FINGER_EVENT(G_io_seproxyhal_spi_buffer);
		break;

	case SEPROXYHAL_TAG_BUTTON_PUSH_EVENT:
		UX_BUTTON_PUSH_EVENT(G_io_seproxyhal_spi_buffer);
		break;

	case SEPROXYHAL_TAG_STATUS_EVENT:
		if (G_io_apdu_media == IO_APDU_MEDIA_USB_HID &&
			!(U4BE(G_io_seproxyhal_spi_buffer, 3) &
			  SEPROXYHAL_TAG_STATUS_EVENT_FLAG_USB_POWERED)) {
			THROW(EXCEPTION_IO_RESET);
		}
		UX_DEFAULT_EVENT();
		break;

	case SEPROXYHAL_TAG_DISPLAY_PROCESSED_EVENT:
		UX_DISPLAYED_EVENT({});
		break;

	case SEPROXYHAL_TAG_TICKER_EVENT:
		UX_TICKER_EVENT(G_io_seproxyhal_spi_buffer, {});
		break;

	default:
		UX_DEFAULT_EVENT();
		break;
	}

	// close the event if not done previously (by a display or whatever)
	if (!io_seproxyhal_spi_is_status_sent()) {
		io_seproxyhal_general_status();
	}

	// command has been processed, DO NOT reset the current APDU transport
	return 1;
}

unsigned short io_exchange_al(unsigned char channel, unsigned short tx_len) {
	switch (channel & ~(IO_FLAGS)) {
	case CHANNEL_KEYBOARD:
		break;
	// multiplexed io exchange over a SPI channel and TLV encapsulated protocol
	case CHANNEL_SPI:
		if (tx_len) {
			io_seproxyhal_spi_send(G_io_apdu_buffer, tx_len);
			if (channel & IO_RESET_AFTER_REPLIED) {
				reset();
			}
			return 0; // nothing received from the master so far (it's a tx transaction)
		} else {
			return io_seproxyhal_spi_recv(G_io_apdu_buffer, sizeof(G_io_apdu_buffer), 0);
		}
	default:
		THROW(INVALID_PARAMETER);
	}
	return 0;
}

static void app_exit(void) {
	BEGIN_TRY_L(exit) {
		TRY_L(exit) {
			os_sched_exit(-1);
		}
		FINALLY_L(exit) {
		}
	}
	END_TRY_L(exit);
}

__attribute__((section(".boot"))) int main(void) {
	// exit critical section
	__asm volatile("cpsie i");

	for (;;) {
		UX_INIT();
		os_boot();
		BEGIN_TRY {
			TRY {
				io_seproxyhal_init();
				USB_power(0);
				USB_power(1);
				ui_idle();
				sia_main();
			}
			CATCH(EXCEPTION_IO_RESET) {
				// reset IO and UX before continuing
				continue;
			}
			CATCH_ALL {
				break;
			}
			FINALLY {
			}
		}
		END_TRY;
	}
	app_exit();
	return 0;
}
