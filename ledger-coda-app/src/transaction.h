struct transaction {
  // uint8_t transaction_id[32];
  uint8_t isDelegation;
  uint64_t nonce;
  uint8_t from[96];
  uint8_t to[96];
  uint64_t amount;
  uint64_t fee;
  uint8_t memo[32];
};

// tx_encode produces a canonical msgpack encoding of a transaction.
// buflen is the size of the buffer.  The return value is the length
// of the resulting encoding.
unsigned int tx_encode(struct transaction *t, uint8_t *buf, int buflen);

// We have a global transaction that is the subject of the current
// operation, if any.
extern struct transaction current_transaction;

// Two callbacks into the main code: approve and deny signing.
void transaction_approve();
void transaction_deny();
void transaction_ui();
