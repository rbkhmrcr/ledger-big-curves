import hashlib, struct, sys, base58, json
import schnorr

# whole version byte tables for coda
value_to_version_byte = {
        'user_command':                     b'\x17',
        'web_pipe':                         b'\x41',
        'data_hash':                        b'\x37',
        'proof':                            b'\x70',
        'signature':                        b'\x9A',
        'non_zero_curve_point':             b'\xCD',
        'non_zero_curve_point_compressed':  b'\xCA',
        'random_oracle_base':               b'\x03',
        'private_key':                      b'\x5A',
        'user_command_memo':                b'\xA2',
        'receipt_chain_hash':               b'\x9D',
        'secret_box_byteswr':               b'\x02',
        'ledger_hash':                      b'\x63'
}

version_byte_to_value = {
        b'\x17': 'user_command',
        b'\x41': 'web_pipe',
        b'\x37': 'data_hash',
        b'\x70': 'proof',
        b'\x9A': 'signature',
        b'\xCD': 'non_zero_curve_point',
        b'\xCA': 'non_zero_curve_point_compressed' ,
        b'\x03': 'random_oracle_base',
        b'\x5A': 'private_key',
        b'\xA2': 'user_command_memo',
        b'\x9D': 'receipt_chain_hash',
        b'\x02': 'secret_box_byteswr',
        b'\x63': 'ledger_hash'
}

"""
{
  "sendPayment": {
    "is_delegation": "False",
    "nonce": 37,
    "from": 123 (this should be the nonce in hardware wallet),
    "to": "tNci9iZe1p3KK4MCcqDa52mpxBTveEm3kqZMm7vwJF9uKzGGt1pCHVNa2oMevDb1HDAs4bNdMQLNbD8N3tkCtKNGM53obE9qFkkhmqMnKRLNLiSfPJuLGsSwqnL3HxSqciJoqJJJmq5Cfb",
    "amount": 1000,
    "fee": 8,
    "valid_until": 1600,
    "memo": "2pmu64f2x97tNiDXMycnLwBSECDKbX77MTXVWVsG8hcRFsedhXDWWq"
    }
}
"""

# versionbyte length = 1 byte
# checksum len = 4 bytes
def b58_encode(payload):
    return base58.b58encode_check(version_string + payload + checksum)

# on input b58 encoded pk, returns version_string || payload
def b58_decode(str58):
    return base58.b58decode_check(str58)

# b58_to is version byte || sign bit || x coord
def b58_decode_to(b58_to):
    bto = b58_decode(b58_to)
    sign_bit = bytes([bto[1]])
    to = bto[2:]
    return sign_bit, to

"""
field elts:
  [ x_coord of receiver/delegate pubkey
  ]

bitstrings:
  [ sign_bit of receiver/delegate pubkey
  , tag bits
  , amount (8 bytes)
  , fee (8 bytes)
  , nonce (4 bytes)
  , valid-until (4 bytes)
  , memo (32 bytes)
  ]
"""

def txn_dict_to_bytes(txn_info):
    sign_bit, to = b58_decode_to(txn_info['to'])
    tag = bytes([1]) if txn_info['is_delegation'] == 'True' else bytes([0])
    amount  = txn_info['amount'].to_bytes(8, 'big')
    fee     = txn_info['fee'].to_bytes(8, 'big')
    nonce   = txn_info['nonce'].to_bytes(4, 'big')
    vu      = txn_info['valid_until'].to_bytes(4, 'big')
    memo    = bytes(txn_info['memo'], 'utf8')
    return to, sign_bit + tag + amount + fee + nonce + vu + memo

def json_to_transaction(txn):
    data = json.loads(txn)
    for _, txn_info in data.items():
        return txn_dict_to_bytes(txn_info)

def jsonfile_to_transaction(infile):
    with open(infile) as json_file:
        data = json.load(json_file)
        for _, payment_dict in data.items():
            for _, txn_info in payment_dict.items():
                return txn_dict_to_bytes(txn_info)

def sig_decode(r):
    # r = bytearray.fromhex(str_r)
    b58_r = base58.b58encode(bytes(r)).decode("utf-8")
    return b58_r

# INS_VERSION       0x01
# INS_PUBLIC_KEY    0x02
# INS_SIGN          0x04
# INS_HASH          0x08

def handle_txn_input(pkno, txn):
    to, msg = json_to_transaction(txn)
    apdu = b'\xE0'  # CLA byte
    apdu += b'\x04' # INS byte
    apdu += b'\x00' # P1 byte
    apdu += b'\x00' # P2 byte
    apdu += b'\x00' # LC byte
    apdu += struct.pack('<I', int(pkno)) # DATA bytes
    apdu += struct.pack("<96s", to)
    apdu += struct.pack("<96s", msg)
    return apdu

def handle_txn_reply(reply):
    r = sig_decode(reply[:len(reply)//2])
    s = sig_decode(reply[len(reply)//2:])
    # print(schnorr.schnorr_verify(msg, pk, s))
    print(json.dumps({'field': r, 'scalar': s}))
    return

if __name__ == "__main__":
    assert base58.b58encode(b'hello') == b'Cn8eVZg'
    assert base58.b58decode(b'Cn8eVZg') == b'hello'
    assert base58.b58decode_check(b'2L5B5yqsVG8Vt') == b'hello'
