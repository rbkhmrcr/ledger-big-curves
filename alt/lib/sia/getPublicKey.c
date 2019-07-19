// This file contains the implementation of the getPublicKey command. It is
// broadly similar to the signHash command, but with a few new features. Since
// much of the code is the same, expect far fewer comments.
//
// A high-level description of getPublicKey is as follows. The user initiates
// the command on their computer by requesting the generation of a specific
// public key. The command handler then displays a screen asking the user to
// confirm the action. If the user presses the 'approve' button, the requested
// key is generated, sent to the computer, and displayed on the device. The
// user may then visually compare the key shown on the device to the key
// received by the computer. Augmenting this, the user may optionally request
// that an address be generated from the public key, in which case this
// address is displayed instead of the public key. A final two-button press
// returns the user to the main screen.
//
// Note that the order of the getPublicKey screens is the reverse of signHash:
// first approval, then comparison.
//
// Keep this description in mind as you read through the implementation.

#include <stdint.h>
#include <stdbool.h>
#include <os.h>
#include <os_io_seproxyhal.h>
#include "blake2b.h"
#include "sia.h"
#include "ux.h"

// Get a pointer to getPublicKey's state variables.
static getPublicKeyContext_t *ctx = &global.getPublicKeyContext;

// Define the comparison screen. This is where the user will compare the
// public key (or address) on their device to the one shown on the computer.
static const bagl_element_t ui_getPublicKey_compare[] = {
	UI_BACKGROUND(),
	UI_ICON_LEFT(0x01, BAGL_GLYPH_ICON_LEFT),
	UI_ICON_RIGHT(0x02, BAGL_GLYPH_ICON_RIGHT),
	UI_TEXT(0x00, 0, 12, 128, "Compare:"),
	// The visible portion of the public key or address.
	UI_TEXT(0x00, 0, 26, 128, global.getPublicKeyContext.partialStr),
};

// Define the preprocessor for the comparison screen. As in signHash, this
// preprocessor selectively hides the left/right arrows. The only difference
// is that, since public keys and addresses have different lengths, checking
// for the end of the string is slightly more complicated.
static const bagl_element_t* ui_prepro_getPublicKey_compare(const bagl_element_t *element) {
	int fullSize = ctx->genAddr ? 76 : 64;
	if ((element->component.userid == 1 && ctx->displayIndex == 0) ||
	    (element->component.userid == 2 && ctx->displayIndex == fullSize-12)) {
		return NULL;
	}
	return element;
}

// Define the button handler for the comparison screen. Again, this is nearly
// identical to the signHash comparison button handler.
static unsigned int ui_getPublicKey_compare_button(unsigned int button_mask, unsigned int button_mask_counter) {
	int fullSize = ctx->genAddr ? 76 : 64;
	switch (button_mask) {
	case BUTTON_LEFT:
	case BUTTON_EVT_FAST | BUTTON_LEFT: // SEEK LEFT
		if (ctx->displayIndex > 0) {
			ctx->displayIndex--;
		}
		os_memmove(ctx->partialStr, ctx->fullStr+ctx->displayIndex, 12);
		UX_REDISPLAY();
		break;

	case BUTTON_RIGHT:
	case BUTTON_EVT_FAST | BUTTON_RIGHT: // SEEK RIGHT
		if (ctx->displayIndex < fullSize-12) {
			ctx->displayIndex++;
		}
		os_memmove(ctx->partialStr, ctx->fullStr+ctx->displayIndex, 12);
		UX_REDISPLAY();
		break;

	case BUTTON_EVT_RELEASED | BUTTON_LEFT | BUTTON_RIGHT: // PROCEED
		// The user has finished comparing, so return to the main screen.
		ui_idle();
		break;
	}
	return 0;
}

// Define the approval screen. This is where the user will approve the
// generation of the public key (or address).
static const bagl_element_t ui_getPublicKey_approve[] = {
	UI_BACKGROUND(),
	UI_ICON_LEFT(0x00, BAGL_GLYPH_ICON_CROSS),
	UI_ICON_RIGHT(0x00, BAGL_GLYPH_ICON_CHECK),
	// These two lines form a complete sentence:
	//
	//    Generate Public
	//       Key #123?
	//
	// or:
	//
	//    Generate Address
	//     from Key #123?
	//
	// Since both lines differ based on user-supplied parameters, we can't use
	// compile-time string literals for either of them.
	UI_TEXT(0x00, 0, 12, 128, global.getPublicKeyContext.typeStr),
	UI_TEXT(0x00, 0, 26, 128, global.getPublicKeyContext.keyStr),
};

// This is the button handler for the approval screen. If the user approves,
// it generates and sends the public key and address. (For simplicity, we
// always send both, regardless of which one the user requested.)
static unsigned int ui_getPublicKey_approve_button(unsigned int button_mask, unsigned int button_mask_counter) {
	// The response APDU will contain multiple objects, which means we need to
	// remember our offset within G_io_apdu_buffer. By convention, the offset
	// variable is named 'tx'.
	uint16_t tx = 0;
	cx_ecfp_public_key_t publicKey;
	switch (button_mask) {
	case BUTTON_EVT_RELEASED | BUTTON_LEFT: // REJECT
		io_exchange_with_code(SW_USER_REJECTED, 0);
		ui_idle();
		break;

	case BUTTON_EVT_RELEASED | BUTTON_RIGHT: // APPROVE
		// Derive the public key and address and store them in the APDU
		// buffer. Even though we know that tx starts at 0, it's best to
		// always add it explicitly; this prevents a bug if we reorder the
		// statements later.
		deriveSiaKeypair(ctx->keyIndex, NULL, &publicKey);
		extractPubkeyBytes(G_io_apdu_buffer + tx, &publicKey);
		tx += 32;
		pubkeyToSiaAddress(G_io_apdu_buffer + tx, &publicKey);
		tx += 76;
		// Flush the APDU buffer, sending the response.
		io_exchange_with_code(SW_OK, tx);

		// Prepare the comparison screen, filling in the header and body text.
		os_memmove(ctx->typeStr, "Compare:", 9);
		if (ctx->genAddr) {
			// The APDU buffer already contains the hex-encoded address, so
			// copy it directly.
			os_memmove(ctx->fullStr, G_io_apdu_buffer + 32, 76);
			ctx->fullStr[76] = '\0';
		} else {
			// The APDU buffer contains the raw bytes of the public key, so
			// first we need to convert to a human-readable form.
			bin2hex(ctx->fullStr, G_io_apdu_buffer, 32);
		}
		os_memmove(ctx->partialStr, ctx->fullStr, 12);
		ctx->partialStr[12] = '\0';
		ctx->displayIndex = 0;

		// Display the comparison screen.
		UX_DISPLAY(ui_getPublicKey_compare, ui_prepro_getPublicKey_compare);
		break;
	}
	return 0;
}

// These are APDU parameters that control the behavior of the getPublicKey
// command.
#define P2_DISPLAY_ADDRESS 0x00
#define P2_DISPLAY_PUBKEY  0x01

// handleGetPublicKey is the entry point for the getPublicKey command. It
// reads the command parameters, prepares and displays the approval screen,
// and sets the IO_ASYNC_REPLY flag.
void handleGetPublicKey(uint8_t p1, uint8_t p2, uint8_t *dataBuffer, uint16_t dataLength, volatile unsigned int *flags, volatile unsigned int *tx) {
	// Sanity-check the command parameters.
	if ((p2 != P2_DISPLAY_ADDRESS) && (p2 != P2_DISPLAY_PUBKEY)) {
		// Although THROW is technically a general-purpose exception
		// mechanism, within a command handler it is basically just a
		// convenient way of bailing out early and sending an error code to
		// the computer. The exception will be caught by sia_main, which
		// appends the code to the response APDU and sends it, much like
		// io_exchange_with_code. THROW should not be called from
		// preprocessors or button handlers.
		THROW(SW_INVALID_PARAM);
	}

	// Read the key index from dataBuffer and set the genAddr flag according
	// to p2.
	ctx->keyIndex = U4LE(dataBuffer, 0);
	ctx->genAddr = (p2 == P2_DISPLAY_ADDRESS);

	// Prepare the approval screen, filling in the header and body text.
	if (ctx->genAddr) {
		os_memmove(ctx->typeStr, "Generate Address", 17);
		os_memmove(ctx->keyStr, "from Key #", 10);
		int n = bin2dec(ctx->keyStr+10, ctx->keyIndex);
		os_memmove(ctx->keyStr+10+n, "?", 2);
	} else {
		os_memmove(ctx->typeStr, "Generate Public", 16);
		os_memmove(ctx->keyStr, "Key #", 5);
		int n = bin2dec(ctx->keyStr+5, ctx->keyIndex);
		os_memmove(ctx->keyStr+5+n, "?", 2);
	}
	UX_DISPLAY(ui_getPublicKey_approve, NULL);
	
	*flags |= IO_ASYNCH_REPLY;
}

// Having previously read through signHash.c, getPublicKey.c shouldn't be too
// difficult to make sense of. We'll move on to the last (and most complex)
// command file in the walkthrough, calcTxnHash.c. Hold onto your hat!
