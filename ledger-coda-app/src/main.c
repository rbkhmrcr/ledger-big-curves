/*******************************************************************************
 *   Ledger Blue
 *   (c) 2016 Ledger
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

#include "os.h"
#include "cx.h"
#include "os_io_seproxyhal.h"

#include "crypto/group.h"
#include "crypto/sign.h"
#include "transaction.h"
#include "keys.h"
#include "ui.h"

unsigned char G_io_seproxyhal_spi_buffer[IO_SEPROXYHAL_BUFFER_SIZE_B];


#define OFFSET_CLA    0
#define OFFSET_INS    1
#define OFFSET_P1     2
#define OFFSET_P2     3
#define OFFSET_LC     4
#define OFFSET_CDATA  5


#define CLA 		  0x80
#define INS_SIGN 	0x02
#define INS_GET_PUBLIC_KEY 0x04
#define P1_LAST 	0x80
#define P1_MORE 	0x00

// From Ledger docs:
// All global variables that are declared as const are stored in read-only flash
// memory, right next to code.
//
// All normal global variables that are declared as non-const are stored in RAM.
// However, thanks to the link script (script.ld) in the SDK, global variables
// that are declared as non-const and are given the prefix N_ are placed in a
// special write-permitted location of NVRAM. This data can be read in the same
// way that regular global variables are read.
//
// However, writing to NVRAM variables must be done using the nvm_write(...)
// function defined by the SDK, which performs a syscall. When loading the app,
// NVRAM variables are initialized with data specified in the app's hexfile
// (this is usually just zero bytes).
//
// Warning: Initializers of global non-const variables (including NVRAM
// variables) are ignored.  As such, this data must be initialized by
// application code.

unsigned char G_io_seproxyhal_spi_buffer[IO_SEPROXYHAL_BUFFER_SIZE_B];

struct transaction current_transaction;

void transaction_approve() {
  unsigned int tx = 0;

  // Avoid large stack allocation; there is no reentry into
  // transaction_approve().
  static unsigned char msg[96];
  signature *sig = NULL;
  unsigned int msg_len, sig_len;

  msg_len = sizeof(msg);
  sig_len = sizeof(sig);

  PRINTF("Signing message: %.*h\n", msg_len, msg);

  scalar private_key;
  group *public_key = 0;
  generate_keypair(public_key, private_key);
  sig_len = sign(sig, public_key, private_key, msg, sig_len);

  tx = sig_len;
  G_io_apdu_buffer[tx++] = 0x90;
  G_io_apdu_buffer[tx++] = 0x00;

  os_memmove(G_io_apdu_buffer, sig->rx, field_BYTES);
  os_memmove(G_io_apdu_buffer + field_BYTES, sig->s, scalar_BYTES);

  // Send back the response, do not restart the event loop
  io_exchange(CHANNEL_APDU | IO_RETURN_AFTER_TX, tx);

  // Display back the original UX
  ui_idle();
}

void transaction_deny() {
  G_io_apdu_buffer[0] = 0x69;
  G_io_apdu_buffer[1] = 0x85;

  // Send back the response, do not restart the event loop
  io_exchange(CHANNEL_APDU | IO_RETURN_AFTER_TX, 2);

  // Display back the original UX
  ui_idle();
}

static void coda_main(void) {
  volatile unsigned int rx = 0;
  volatile unsigned int tx = 0;
  volatile unsigned int flags = 0;

  // next timer callback in 500 ms
  UX_CALLBACK_SET_INTERVAL(500);

  // DESIGN NOTE: the bootloader ignores the way APDU are fetched. The only
  // goal is to retrieve APDU.
  // When APDU are to be fetched from multiple IOs, like NFC+USB+BLE, make
  // sure the io_event is called with a
  // switch event, before the apdu is replied to the bootloader. This avoid
  // APDU injection faults.
  for (;;) {
    volatile unsigned short sw = 0;

    BEGIN_TRY {
      TRY {
        rx = tx;
        tx = 0; // ensure no race in catch_other if io_exchange throws
                // an error
        rx = io_exchange(CHANNEL_APDU | flags, rx);
        flags = 0;

        // no apdu received, well, reset the session, and reset the
        // bootloader configuration
        if (rx == 0) {
          THROW(0x6982);
        }

        if (G_io_apdu_buffer[OFFSET_CLA] != CLA) {
          THROW(0x6E00);
        }

        uint8_t ins = G_io_apdu_buffer[OFFSET_INS];
        switch (ins) {
        case INS_SIGN: {
          os_memset(&current_transaction, 0, sizeof(current_transaction));
          uint8_t *p;
          p = &G_io_apdu_buffer[OFFSET_CDATA];
          os_memmove(p, &current_transaction, sizeof(current_transaction));
          transaction_ui();
          flags |= IO_ASYNCH_REPLY;
        } break;

        case INS_GET_PUBLIC_KEY: {
          group *public_key = 0; // should have printable type? base58
          generate_public_key(public_key);
          os_memmove(G_io_apdu_buffer, public_key, sizeof(public_key));
          tx = sizeof(public_key);
          THROW(0x9000);
        } break;

        case 0xFF: // return to dashboard
          goto return_to_dashboard;

        default:
          THROW(0x6D00);
          break;
        }
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
        // Unexpected exception => report
        G_io_apdu_buffer[tx] = sw >> 8;
        G_io_apdu_buffer[tx + 1] = sw;
        tx += 2;
      }
      FINALLY {}
    }
    END_TRY;
  }

return_to_dashboard:
  return;
}

void io_seproxyhal_display(const bagl_element_t *element) {
  io_seproxyhal_display_default((bagl_element_t *)element);
}

unsigned char io_event(unsigned char channel) {
  // nothing done with the event, throw an error on the transport layer if
  // needed

  // can't have more than one tag in the reply, not supported yet.
  switch (G_io_seproxyhal_spi_buffer[0]) {
  case SEPROXYHAL_TAG_FINGER_EVENT:
    UX_FINGER_EVENT(G_io_seproxyhal_spi_buffer);
    break;

  case SEPROXYHAL_TAG_BUTTON_PUSH_EVENT: // for Nano S
    UX_BUTTON_PUSH_EVENT(G_io_seproxyhal_spi_buffer);
    break;

  case SEPROXYHAL_TAG_DISPLAY_PROCESSED_EVENT:
    UX_DISPLAYED_EVENT();
    break;

  case SEPROXYHAL_TAG_TICKER_EVENT:
    UX_TICKER_EVENT(G_io_seproxyhal_spi_buffer, {
      // defaulty retrig very soon (will be overriden during
      // stepper_prepro)
      UX_CALLBACK_SET_INTERVAL(500);
      UX_REDISPLAY();
    });
    break;

  // unknown events are acknowledged
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
      return 0; // nothing received from the master so far (it's a tx
                // transaction)
    } else {
      return io_seproxyhal_spi_recv(G_io_apdu_buffer, sizeof(G_io_apdu_buffer),
                                    0);
    }

  default:
    THROW(INVALID_PARAMETER);
  }
  return 0;
}

__attribute__((section(".boot"))) int main(void) {
  // exit critical section
  __asm volatile("cpsie i");

  line_buffer[0] = '\0';

  // ensure exception will work as planned
  os_boot();

  UX_INIT();
  UX_MENU_INIT();

  BEGIN_TRY {
    TRY {
      io_seproxyhal_init();

      USB_power(0);
      USB_power(1);

      ui_idle();

      coda_main();
    }
    CATCH_OTHER(e) {}
    FINALLY {}
  }
  END_TRY;
}
