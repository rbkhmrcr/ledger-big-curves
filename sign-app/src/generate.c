#include "cx.h"
#include "os.h"

#include "os_io_seproxyhal.h"

// Ledger uses:
// - BIP 39 to generate and interpret the master seed, which
// produces the 24 words shown on the device at startup.
// - BIP 32 for HD key derivation (using the child key derivation function)
// - BIP 44 for HD account derivation (so e.g. btc and coda keys don't clash)

void secp256k1_generate_keypair(cx_ecfp_public_key_t *publicKey,
                                cx_ecfp_private_key_t *privateKey,
                                const uint8_t pvkey[32]) {
  cx_ecfp_init_private_key(CX_CURVE_256K1, pvkey, 32, privateKey);
  cx_ecfp_init_public_key(CX_CURVE_256K1, NULL, 0, publicKey);
  cx_ecfp_generate_pair(CX_CURVE_256K1, publicKey, privateKey, 1);
}

// get_public_key
uint8_t bip32_depth;
uint32_t bip32_path[10];

void get_public_key(cx_ecfp_public_key_t *publicKey) {
  cx_ecfp_private_key_t privateKey;
  uint8_t pvkey[32];

  os_perso_derive_node_bip32(CX_CURVE_256K1, bip32_path, bip32_depth, pvkey,
                             NULL);

  secp256k1_generate_keypair(publicKey, &privateKey, pvkey);
  memset(pvkey, 0, sizeof(pvkey));
  memset(&privateKey, 0, sizeof(privateKey));
}

void generate_keypair(uint64_t privateKey[12], uint64_t publicKey[24]) {
  secp256k1_generate_keypair;
}
