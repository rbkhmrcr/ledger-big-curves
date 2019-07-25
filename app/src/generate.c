#include "os.h"
#include "cx.h"
#include "os_io_seproxyhal.h"
#include "glyphs.h"

// Ledger uses: 
// - BIP 39 to generate and interpret the master seed, which
// produces the 24 words shown on the device at startup.  
// - BIP 32 for HD key derivation (using the child key derivation function) 
// - BIP 44 for HD account derivation (so e.g. btc and coda keys don't clash)

// HD tree


derive_private_key

generate_public_key

generate_pair

