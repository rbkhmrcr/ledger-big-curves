#include "os.h"
#include "cx.h"
#include "crypto/group-utils.h"
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

void generate_keypair(scalar6753 priv_key, gmnt6753 *pub_key) {

  cx_ecfp_public_key_t *public_key = 0;
  cx_ecfp_private_key_t *private_key = 0;
  uint8_t private_key_data[64];
  uint32_t bip32_path[5];

  bip32_path[0] = 44     | 0x80000000;
  bip32_path[1] = 49370  | 0x80000000;
  bip32_path[2] = 0      | 0x80000000;
  bip32_path[3] = 0;
  bip32_path[4] = 0;

  os_perso_derive_node_bip32(CX_CURVE_256K1, bip32_path,
                             sizeof(bip32_path) / sizeof(bip32_path[0]),
                             private_key_data,
                             NULL);
  // curve, rawkey, key_len, outparam private key
  cx_ecfp_init_private_key(CX_CURVE_256K1, private_key_data, 64, private_key);
  // curve, rawkey, keylength, out param public key
  cx_ecfp_init_public_key(CX_CURVE_256K1, NULL, 0, public_key);
  // curve, pub key, priv key, 1 = keep_private -> 0 would mean a new priv key is generated
  cx_ecfp_generate_pair(CX_CURVE_256K1, public_key, private_key, 1);

  // TODO concatenate into a good private key?
  //
  // TODO create public key from that

  os_memset(private_key_data, 0, sizeof(private_key_data));
  os_memset(&private_key, 0, sizeof(private_key));
}

void generate_public_key(gmnt6753 *pub_key) {
  scalar6753 priv_key;
  generate_keypair(pub_key, priv_key);
  os_memset(priv_key, 0, sizeof(priv_key));
}
