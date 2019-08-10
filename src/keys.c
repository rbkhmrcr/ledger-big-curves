#include "cx.h"
#include "os.h"

#include "keys.h"

// Ledger uses:
// - BIP 39 to generate and interpret the master seed, which
//   produces the 24 words shown on the device at startup.
// - BIP 32 for HD key derivation (using the child key derivation function)
// - BIP 44 for HD account derivation (so e.g. btc and coda keys don't clash)

// There's no 'get key_pair', or 'get_private_key' function, as that in done
// in the signature code? or should we store it?

// TODO we should be generating out public and private keypairs, this just
// generates a secp256k1 pair, with which we can seed our generation.
// We will have to replace cx_ecfp_init_public_key and
// cx_ecfp_generate_pair entirely

void get_public_key(cx_ecfp_public_key_t *public_key) {
  cx_ecfp_public_key_t private_key;
  uint8_t private_key_data[64];
  uint32_t bip32_path[5];

  bip32Path[0] = 44     | 0x80000000;
  bip32Path[1] = 49370  | 0x80000000;
  bip32Path[2] = 0      | 0x80000000;
  bip32Path[3] = 0;
  bip32Path[4] = 0;

  os_perso_derive_node_bip32(CX_CURVE_256K1, bip32_path,
                             sizeof(bip32_path) / sizeof(bip32_path[0]),
                             private_key,
                             NULL); // TODO could also use bip32_depth?
  cx_ecfp_init_private_key(CX_CURVE_256K1, private_key_data, 32, private_key);
  cx_ecfp_init_public_key(CX_CURVE_256K1, NULL, 0,
                          publicKey); // TODO what is this NULL
  cx_ecfp_generate_pair(CX_CURVE_256K1, publicKey, privateKey,
                        1); // TODO what is this 1
  memset(private_key_data, 0, sizeof(private_key_data));
  memset(&private_key, 0, sizeof(private_key));
}
