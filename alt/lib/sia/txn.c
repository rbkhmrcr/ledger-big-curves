#include <stdbool.h>
#include <stdint.h>
#include <os.h>
#include "blake2b.h"
#include "sia.h"

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

// cur2dec converts a Sia-encoded currency value to a decimal string and
// appends a final NUL byte. It returns the length of the string. If the value
// is too large, it throws TXN_STATE_ERR.
static int cur2dec(uint8_t *out, uint8_t *cur) {
	if (cur[0] == 0) {
		out[0] = '\0';
		return 0;
	}

	// sanity check the size of the value. The size (in bytes) is given in the
	// first byte; it should never be greater than 18 (18 bytes = 144 bits,
	// i.e. a value of 2^144 H, or 22 quadrillion SC).
	if (cur[0] > 18) {
		THROW(TXN_STATE_ERR);
	}


	// convert big-endian uint8_t[] to little-endian uint64_t[]
	//
	// NOTE: the Sia encoding omits any leading zeros, so the first "uint64"
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

static void need_at_least(txn_state_t *txn, uint64_t n) {
	if ((txn->buflen - txn->pos) < n) {
		THROW(TXN_STATE_PARTIAL);
	}
}

static void seek(txn_state_t *txn, uint64_t n) {
	need_at_least(txn, n);
	txn->pos += n;
}

static void advance(txn_state_t *txn) {
	// if elem is covered, add it to the hash
	if (txn->elemType != TXN_ELEM_TXN_SIG) {
		blake2b_update(&txn->blake, txn->buf, txn->pos);
	} else if (txn->sliceIndex == txn->sigIndex && txn->pos >= 48) {
		// add just the ParentID, Timelock, and PublicKeyIndex
		blake2b_update(&txn->blake, txn->buf, 48);
	}

	txn->buflen -= txn->pos;
	os_memmove(txn->buf, txn->buf+txn->pos, txn->buflen);
	txn->pos = 0;
}

static uint64_t readInt(txn_state_t *txn) {
	need_at_least(txn, 8);
	uint64_t u = U8LE(txn->buf, txn->pos);
	seek(txn, 8);
	return u;
}

static void readCurrency(txn_state_t *txn, uint8_t *outVal) {
	uint64_t valLen = readInt(txn);
	need_at_least(txn, valLen);
	if (outVal) {
		txn->valLen = cur2dec(outVal, txn->buf+txn->pos-8);
	}
	seek(txn, valLen);
}

static void readHash(txn_state_t *txn, uint8_t *outAddr) {
	need_at_least(txn, 32);
	if (outAddr) {
		bin2hex(outAddr, txn->buf+txn->pos, 32);
		uint8_t checksum[6];
		blake2b(checksum, sizeof(checksum), txn->buf+txn->pos, 32);
		bin2hex(outAddr+64, checksum, sizeof(checksum));
	}
	seek(txn, 32);
}

static void readPrefixedBytes(txn_state_t *txn) {
	uint64_t len = readInt(txn);
	seek(txn, len);
}

static void readUnlockConditions(txn_state_t *txn) {
	readInt(txn); // Timelock
	uint64_t numKeys = readInt(txn); // PublicKeys
	while (numKeys --> 0) {
		seek(txn, 16);          // Algorithm
		readPrefixedBytes(txn); // Key
	}
	readInt(txn); // SignaturesRequired
}

static void readCoveredFields(txn_state_t *txn) {
	need_at_least(txn, 1);
	// for now, we require WholeTransaction = true
	if (txn->buf[txn->pos] != 1) {
		THROW(TXN_STATE_ERR);
	}
	seek(txn, 1);
	// all other fields must be empty
	for (int i = 0; i < 10; i++) {
		if (readInt(txn) != 0) {
			THROW(TXN_STATE_ERR);
		}
	}
}

static void addReplayProtection(cx_blake2b_t *S) {
	static uint8_t const replayPrefix[] = {0};
	blake2b_update(S, replayPrefix, 1);
}

// throws txnDecoderState_e
static void __txn_next_elem(txn_state_t *txn) {
	// if we're on a slice boundary, read the next length prefix and bump the
	// element type
	while (txn->sliceIndex == txn->sliceLen) {
		if (txn->elemType == TXN_ELEM_TXN_SIG) {
			// store final hash
			blake2b_final(&txn->blake, txn->sigHash, sizeof(txn->sigHash));
			THROW(TXN_STATE_FINISHED);
		}
		txn->sliceLen = readInt(txn);
		txn->sliceIndex = 0;
		txn->elemType++;
		advance(txn);

		// if we've reached the TransactionSignatures, check that sigIndex is
		// a valid index
		if ((txn->elemType == TXN_ELEM_TXN_SIG) && (txn->sigIndex >= txn->sliceLen)) {
			THROW(TXN_STATE_ERR);
		}
	}

	switch (txn->elemType) {
	// these elements should be displayed
	case TXN_ELEM_SC_OUTPUT:
		readCurrency(txn, txn->outVal); // Value
		readHash(txn, txn->outAddr);    // UnlockHash
		advance(txn);
		txn->sliceIndex++;
		THROW(TXN_STATE_READY);

	case TXN_ELEM_SF_OUTPUT:
		readCurrency(txn, txn->outVal); // Value
		readHash(txn, txn->outAddr);    // UnlockHash
		readCurrency(txn, NULL);        // ClaimStart
		advance(txn);
		txn->sliceIndex++;
		THROW(TXN_STATE_READY);

	case TXN_ELEM_MINER_FEE:
		readCurrency(txn, txn->outVal); // Value
		os_memmove(txn->outAddr, "[Miner Fee]", 12);
		advance(txn);
		txn->sliceIndex++;
		THROW(TXN_STATE_READY);

	// these elements should be decoded, but not displayed
	case TXN_ELEM_SC_INPUT:
		readHash(txn, NULL);       // ParentID
		readUnlockConditions(txn); // UnlockConditions
		if (txn->asicChain) {
			addReplayProtection(&txn->blake);
		}
		advance(txn);
		txn->sliceIndex++;
		return;

	case TXN_ELEM_SF_INPUT:
		readHash(txn, NULL);       // ParentID
		readUnlockConditions(txn); // UnlockConditions
		readHash(txn, NULL);       // ClaimUnlockHash
		if (txn->asicChain) {
			addReplayProtection(&txn->blake);
		}
		advance(txn);
		txn->sliceIndex++;
		return;

	case TXN_ELEM_TXN_SIG:
		readHash(txn, NULL);    // ParentID
		readInt(txn);           // PublicKeyIndex
		readInt(txn);           // Timelock
		readCoveredFields(txn); // CoveredFields
		readPrefixedBytes(txn); // Signature
		advance(txn);
		txn->sliceIndex++;
		return;

	// these elements should not be present
	case TXN_ELEM_FC:
	case TXN_ELEM_FCR:
	case TXN_ELEM_SP:
	case TXN_ELEM_ARB_DATA:
		if (txn->sliceLen != 0) {
			THROW(TXN_STATE_ERR);
		}
		return;
	}
}


txnDecoderState_e txn_next_elem(txn_state_t *txn) {
	// Like many transaction decoders, we use exceptions to jump out of deep
	// call stacks when we encounter an error. There are two important rules
	// for Ledger exceptions: declare modified variables as volatile, and do
	// not THROW(0). Presumably, 0 is the sentinel value for "no exception
	// thrown." So be very careful when throwing enums, since enums start at 0
	// by default.
	volatile txnDecoderState_e result;
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
	if (txn->buflen + 255 > sizeof(txn->buf)) {
		// we filled the buffer to max capacity, but there still wasn't enough
		// to decode a full element. This generally means that the txn is
		// corrupt in some way, since elements shouldn't be very large.
		return TXN_STATE_ERR;
	}
	return result;
}

void txn_init(txn_state_t *txn, uint16_t sigIndex, bool asicChain) {
	os_memset(txn, 0, sizeof(txn_state_t));
	txn->buflen = txn->pos = txn->sliceIndex = txn->sliceLen = txn->valLen = 0;
	txn->elemType = -1; // first increment brings it to SC_INPUT
	txn->sigIndex = sigIndex;
	txn->asicChain = asicChain;

	// initialize hash state
	blake2b_init(&txn->blake);
}

void txn_update(txn_state_t *txn, uint8_t *in, uint8_t inlen) {
	// the buffer should never overflow; any elements should always be drained
	// before the next read.
	if (txn->buflen + inlen > sizeof(txn->buf)) {
		THROW(SW_DEVELOPER_ERR);
	}

	// append to the buffer
	os_memmove(txn->buf + txn->buflen, in, inlen);
	txn->buflen += inlen;

	// reset the seek position; if we previously threw TXN_STATE_PARTIAL, now
	// we can try decoding again from the beginning.
	txn->pos = 0;
}
