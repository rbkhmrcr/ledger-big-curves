// This file contains the implementation of the calcTxnHash command. It is
// significantly more complicated than the other commands, mostly due to the
// transaction parsing.
//
// A high-level description of calcTxnHash is as follows. The user initiates
// the command on their computer by requesting the hash of a specific
// transaction. A flag in the request controls whether the resulting hash
// should be signed. The command handler then begins reading transaction data
// from the computer, in packets of up to 255 bytes at a time. The handler
// buffers this data until a full "element" in parsed. Depending on the type
// of the element, it may then be displayed to the user for comparison. Once
// all elements have been received and parsed, the final screen differs
// depending on whether a signature was requested. If so, the user is prompted
// to approve the signature; if they do, the signature is sent to the
// computer, and the app returns to the main menu. If no signature was
// requested, the transaction hash is immediately sent to the computer and
// displayed on a comparison screen. Pressing both buttons returns the user to
// the main menu.
//
// Keep this description in mind as you read through the implementation.

#include <stdint.h>
#include <stdbool.h>
#include <os.h>
#include <os_io_seproxyhal.h>
#include "blake2b.h"
#include "sia.h"
#include "ux.h"

static calcTxnHashContext_t *ctx = &global.calcTxnHashContext;

static const bagl_element_t ui_calcTxnHash_compare[] = {
	UI_BACKGROUND(),
	UI_ICON_LEFT(0x01, BAGL_GLYPH_ICON_LEFT),
	UI_ICON_RIGHT(0x02, BAGL_GLYPH_ICON_RIGHT),
	UI_TEXT(0x00, 0, 12, 128, "Compare Hash:"),
	UI_TEXT(0x00, 0, 26, 128, global.calcTxnHashContext.partialStr),
};

static const bagl_element_t* ui_prepro_calcTxnHash_compare(const bagl_element_t *element) {
	if ((element->component.userid == 1 && ctx->displayIndex == 0) ||
	    (element->component.userid == 2 && ctx->displayIndex == ctx->elemLen-12)) {
		return NULL;
	}
	return element;
}

static unsigned int ui_calcTxnHash_compare_button(unsigned int button_mask, unsigned int button_mask_counter) {
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
		if (ctx->displayIndex < ctx->elemLen-12) {
			ctx->displayIndex++;
		}
		os_memmove(ctx->partialStr, ctx->fullStr+ctx->displayIndex, 12);
		UX_REDISPLAY();
		break;

	case BUTTON_EVT_RELEASED | BUTTON_LEFT | BUTTON_RIGHT: // PROCEED
		ui_idle();
		break;
	}
	return 0;
}

static const bagl_element_t ui_calcTxnHash_sign[] = {
	UI_BACKGROUND(),
	UI_ICON_LEFT(0x00, BAGL_GLYPH_ICON_CROSS),
	UI_ICON_RIGHT(0x00, BAGL_GLYPH_ICON_CHECK),
	UI_TEXT(0x00, 0, 12, 128, "Sign this Txn"),
	UI_TEXT(0x00, 0, 26, 128, global.calcTxnHashContext.fullStr), // "with Key #123?"
};

static unsigned int ui_calcTxnHash_sign_button(unsigned int button_mask, unsigned int button_mask_counter) {
	switch (button_mask) {
	case BUTTON_EVT_RELEASED | BUTTON_LEFT: // REJECT
		io_exchange_with_code(SW_USER_REJECTED, 0);
		ui_idle();
		break;

	case BUTTON_EVT_RELEASED | BUTTON_RIGHT: // APPROVE
		deriveAndSign(G_io_apdu_buffer, ctx->keyIndex, ctx->txn.sigHash);
		io_exchange_with_code(SW_OK, 64);
		ui_idle();
		break;
	}
	return 0;
}

static const bagl_element_t ui_calcTxnHash_elem[] = {
	UI_BACKGROUND(),
	UI_ICON_LEFT(0x01, BAGL_GLYPH_ICON_LEFT),
	UI_ICON_RIGHT(0x02, BAGL_GLYPH_ICON_RIGHT),
	UI_TEXT(0x00, 0, 12, 128, global.calcTxnHashContext.labelStr),
	UI_TEXT(0x00, 0, 26, 128, global.calcTxnHashContext.partialStr),
};

static const bagl_element_t* ui_prepro_calcTxnHash_elem(const bagl_element_t *element) {
	if ((element->component.userid == 1 && ctx->displayIndex == 0) ||
	    (element->component.userid == 2 && ((ctx->elemLen < 12) || (ctx->displayIndex == ctx->elemLen-12)))) {
		return NULL;
	}
	return element;
}

// This is a helper function that prepares an element of the transaction for
// display. It stores the type of the element in labelStr, and a human-
// readable representation of the element in fullStr. As in previous screens,
// partialStr holds the visible portion of fullStr.
static void fmtTxnElem(calcTxnHashContext_t *ctx) {
	txn_state_t *txn = &ctx->txn;

	switch (txn->elemType) {
	case TXN_ELEM_SC_OUTPUT:
		os_memmove(ctx->labelStr, "Siacoin Output #", 16);
		bin2dec(ctx->labelStr+16, txn->sliceIndex);
		// An element can have multiple screens. For each siacoin output, the
		// user needs to see both the destination address and the amount.
		// These are rendered in separate screens, and elemPart is used to
		// identify which screen is being viewed.
		if (ctx->elemPart == 0) {
			os_memmove(ctx->fullStr, txn->outAddr, sizeof(txn->outAddr));
			os_memmove(ctx->partialStr, ctx->fullStr, 12);
			ctx->elemLen = 76;
			ctx->elemPart++;
		} else {
			os_memmove(ctx->fullStr, txn->outVal, sizeof(txn->outVal));
			ctx->elemLen = formatSC(ctx->fullStr, txn->valLen);
			os_memmove(ctx->partialStr, ctx->fullStr, 12);
			ctx->elemPart = 0;
		}
		break;

	case TXN_ELEM_SF_OUTPUT:
		os_memmove(ctx->labelStr, "Siafund Output #", 16);
		bin2dec(ctx->labelStr+16, txn->sliceIndex);
		if (ctx->elemPart == 0) {
			os_memmove(ctx->fullStr, txn->outAddr, sizeof(txn->outAddr));
			os_memmove(ctx->partialStr, ctx->fullStr, 12);
			ctx->elemLen = 76;
			ctx->elemPart++;
		} else {
			os_memmove(ctx->fullStr, txn->outVal, sizeof(txn->outVal));
			os_memmove(ctx->fullStr+txn->valLen, " SF", 4);
			os_memmove(ctx->partialStr, ctx->fullStr, 12);
			ctx->elemLen = txn->valLen + 4;
			ctx->elemPart = 0;
		}
		break;

	case TXN_ELEM_MINER_FEE:
		// Miner fees only have one part.
		os_memmove(ctx->labelStr, "Miner Fee #", 11);
		bin2dec(ctx->labelStr+11, txn->sliceIndex);
		os_memmove(ctx->fullStr, txn->outVal, sizeof(txn->outVal));
		ctx->elemLen = formatSC(ctx->fullStr, txn->valLen);
		os_memmove(ctx->partialStr, ctx->fullStr, 12);
		ctx->elemPart = 0;
		break;

	default:
		// This should never happen.
		io_exchange_with_code(SW_DEVELOPER_ERR, 0);
		ui_idle();
		break;
	}

	// Regardless of what we're displaying, the displayIndex should always be
	// reset to 0, because we're displaying the beginning of fullStr.
	ctx->displayIndex = 0;
}

static unsigned int ui_calcTxnHash_elem_button(unsigned int button_mask, unsigned int button_mask_counter) {
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
		if (ctx->displayIndex < ctx->elemLen-12) {
			ctx->displayIndex++;
		}
		os_memmove(ctx->partialStr, ctx->fullStr+ctx->displayIndex, 12);
		UX_REDISPLAY();
		break;

	case BUTTON_EVT_RELEASED | BUTTON_LEFT | BUTTON_RIGHT: // PROCEED
		if (ctx->elemPart > 0) {
			// We're in the middle of displaying a multi-part element; display
			// the next part.
			fmtTxnElem(ctx);
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
			ctx->elemPart = 0;
			fmtTxnElem(ctx);
			UX_REDISPLAY();
			break;
		case TXN_STATE_FINISHED:
			// We've finished decoding the transaction, and all elements have
			// been displayed.
			if (ctx->sign) {
				// If we're signing the transaction, prepare and display the
				// approval screen.
				os_memmove(ctx->fullStr, "with Key #", 10);
				os_memmove(ctx->fullStr+10+(bin2dec(ctx->fullStr+10, ctx->keyIndex)), "?", 2);
				UX_DISPLAY(ui_calcTxnHash_sign, NULL);
			} else {
				// If we're just computing the hash, send it immediately and
				// display the comparison screen
				os_memmove(G_io_apdu_buffer, ctx->txn.sigHash, 32);
				io_exchange_with_code(SW_OK, 32);
				bin2hex(ctx->fullStr, ctx->txn.sigHash, sizeof(ctx->txn.sigHash));
				os_memmove(ctx->partialStr, ctx->fullStr, 12);
				ctx->elemLen = 64;
				ctx->displayIndex = 0;
				UX_DISPLAY(ui_calcTxnHash_compare, ui_prepro_calcTxnHash_compare);
			}
			// Reset the initialization state.
			ctx->initialized = false;
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

// handleCalcTxnHash reads a signature index and a transaction, calculates the
// SigHash of the transaction, and optionally signs the hash using a specified
// key. The transaction is processed in a streaming fashion and displayed
// piece-wise to the user.
void handleCalcTxnHash(uint8_t p1, uint8_t p2, uint8_t *dataBuffer, uint16_t dataLength, volatile unsigned int *flags, volatile unsigned int *tx) {
	if ((p1 != P1_FIRST && p1 != P1_MORE) || (p2 != P2_DISPLAY_HASH && p2 != P2_SIGN_HASH)) {
		THROW(SW_INVALID_PARAM);
	}

	if (p1 == P1_FIRST) {
		// If this is the first packet of a transaction, the transaction
		// context must not already be initialized. (Otherwise, an attacker
		// could fool the user by concatenating two transactions.)
		//
		// NOTE: ctx->initialized is set to false when the Sia app loads.
		if (ctx->initialized) {
			THROW(SW_IMPROPER_INIT);
		}
		ctx->initialized = true;

		// If this is the first packet, it will include the key index and sig
		// index in addition to the transaction data. Use these to initialize
		// the ctx and the transaction decoder.
		ctx->keyIndex = U4LE(dataBuffer, 0); // NOTE: ignored if !ctx->sign
		dataBuffer += 4; dataLength -= 4;
		uint16_t sigIndex = U2LE(dataBuffer, 0);
		dataBuffer += 2; dataLength -= 2;
		// The official Sia Nano S app only signs transactions on the forked
		// chain. To use the app on dissenting chains, pass false instead.
		const bool useASICHardforkChain = true;
		txn_init(&ctx->txn, sigIndex, useASICHardforkChain);

		// Set ctx->sign according to P2.
		ctx->sign = (p2 & P2_SIGN_HASH);

		// Initialize some display-related variables.
		ctx->partialStr[12] = '\0';
		ctx->elemPart = ctx->elemLen = ctx->displayIndex = 0;
	} else {
		// If this is not P1_FIRST, the transaction must have been
		// initialized previously.
		if (!ctx->initialized) {
			THROW(SW_IMPROPER_INIT);
		}
	}

	// Add the new data to transaction decoder.
	txn_update(&ctx->txn, dataBuffer, dataLength);

	// Attempt to decode the next element of the transaction. Note that this
	// code is essentially identical to ui_calcTxnHash_elem_button. Sadly,
	// there doesn't seem to be a clean way to avoid this duplication.
	switch (txn_next_elem(&ctx->txn)) {
	case TXN_STATE_ERR:
		THROW(SW_INVALID_PARAM);
	case TXN_STATE_PARTIAL:
		THROW(SW_OK);
	case TXN_STATE_READY:
		ctx->elemPart = 0;
		fmtTxnElem(ctx);
		UX_DISPLAY(ui_calcTxnHash_elem, ui_prepro_calcTxnHash_elem);
		*flags |= IO_ASYNCH_REPLY;
		break;
	case TXN_STATE_FINISHED:
		if (ctx->sign) {
			os_memmove(ctx->fullStr, "with Key #", 10);
			bin2dec(ctx->fullStr+10, ctx->keyIndex);
			os_memmove(ctx->fullStr+10+(bin2dec(ctx->fullStr+10, ctx->keyIndex)), "?", 2);
			UX_DISPLAY(ui_calcTxnHash_sign, NULL);
			*flags |= IO_ASYNCH_REPLY;
		} else {
			os_memmove(G_io_apdu_buffer, ctx->txn.sigHash, 32);
			io_exchange_with_code(SW_OK, 32);
			bin2hex(ctx->fullStr, ctx->txn.sigHash, sizeof(ctx->txn.sigHash));
			os_memmove(ctx->partialStr, ctx->fullStr, 12);
			ctx->elemLen = 64;
			ctx->displayIndex = 0;
			UX_DISPLAY(ui_calcTxnHash_compare, ui_prepro_calcTxnHash_compare);
			// The above code does something strange: it calls io_exchange
			// directly from the command handler. You might wonder: why not
			// just prepare the APDU buffer and let sia_main call io_exchange?
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
			// sia_main send the APDU with io_exchange. What happens at the
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
		ctx->initialized = false;
		break;
	}
}

// It is not necessary to completely understand this handler to write your own
// Nano S app; much of it is Sia-specific and will not generalize to other
// apps. The important part is knowing how to structure handlers that involve
// multiple APDU exchanges. If you would like to dive deeper into how the
// handler buffers transaction data and parses elements, proceed to txn.c.
// Otherwise, this concludes the walkthrough. Feel free to fork this app and
// modify it to suit your own needs.
