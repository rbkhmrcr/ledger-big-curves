import json
from ledgerblue.comm import getDongle
from ledgerblue.commException import CommException
import hashlib
import struct
import sys
import base58
import schnorr

# whole version byte tables for extensibility
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

# lengths in bytes (for reference)
# version_len = 1
# checksum_len = 4

def b58_encode(payload):
    return base58.b58encode_check(version_string + payload + checksum)

# on input one of our public keys, returns version_string || payload (|| = concat)
def b58_decode(str58):
    return base58.b58decode_check(str58)

def is_curvepoint(x):
    z = (pow(x, 3, schnorr.p) + schnorr.a_coeff * x + schnorr.b_coeff) % schnorr.p
    return schnorr.is_square(z)

def str58_to_bytes(str58):
    return b58_decode(str58)

assert base58.b58encode(b'hello') == b'Cn8eVZg'
assert base58.b58decode(b'Cn8eVZg') == b'hello'
assert base58.b58decode_check(b'2L5B5yqsVG8Vt') == b'hello'

def packtxn(indict, output):
    # output += struct.pack("", indict['id'])
    output += struct.pack('?', indict['isDelegation'])
    output += struct.pack("<I", indict['nonce'])
    output += struct.pack("<96s", indict['from'])
    output += struct.pack("<96s", indict['to'])
    output += struct.pack("<I", indict['amount'])
    output += struct.pack("<I", indict['fee'])
    output += struct.pack("<32s", indict['memo'])
    return output

#define INS_VERSION       0x01
#define INS_PUBLIC_KEY    0x02
#define INS_SIGN          0x04
#define INS_HASH          0x08

def handle_txn_input(pkno, to, h):
    apdu = b'\xE0'  # CLA byte
    apdu += b'\x04' # INS byte
    apdu += b'\x00' # P1 byte
    apdu += b'\x00' # P2 byte
    apdu += b'\x00' # LC byte
    apdu += struct.pack('<I', int(pkno)) # DATA bytes
    apdu += struct.pack("<96s", to)
    apdu += struct.pack("<96s", h)
    return apdu

def handle_txn_reply(reply, outfile):
    (msg, pk, s) = reply
    schnorr.schnorr_verify(msg, pk, s)
    print("Verified signature")
    # FIXME encode with base58
    with open(outfile, 'w') as f:
        json.dump(signature.hex())
    print('Signed transaction written to ', outfile)
    return

def handle_stream_input(pkno, infile):
    apdu = b'\xE0'  # CLA byte
    apdu += b'\x08' # INS byte
    apdu += b'\x00' # P1 byte
    apdu += b'\x00' # P2 byte
    apdu += b'\x00' # LC byte
    indict = handle_transaction(pkno, infile)
    apdu = packtxn(indict, apdu)
    return apdu

def handle_stream_reply(reply, outfile):
    (pk, s) = reply
    schnorr.schnorr_verify(msg, pk, s)
    print("Verified signature")
    # FIXME encode with base58 : decode.b58_encode(signature)
    with open(outfile, 'w') as f:
        json.dump(signature.hex())
    print('Signed transaction written to ', outfile)
    return

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

def json_to_transaction(infile):
    with open(infile) as json_file:
        data = json.load(json_file)
        # splits into msg_type 'sendPayment': payment_dict
        for _, payment_dict in data.items():
            # splits entries of 'payment' : txn_info
            for _, txn_info in payment_dict.items():
                print(txn_info)
                b58_to = txn_info['to']
                # to is version byte, sign bit, x coord
                bto = b58_decode(b58_to)
                sign_bit = bytes([bto[1]])
                to = bto[2:]
                print(to)
                # sign = bytes(txn_info['sign_bit'], 'utf8')
                if txn_info['is_delegation'] == 'True':
                    tag = bytes([1])
                else:
                    tag = bytes([0])
                amount  = txn_info['amount'].to_bytes(8, 'big')
                fee     = txn_info['fee'].to_bytes(8, 'big')
                nonce   = txn_info['nonce'].to_bytes(4, 'big')
                vu      = txn_info['valid_until'].to_bytes(4, 'big')
                memo    = bytes(txn_info['memo'], 'utf8')
        return to, sign_bit + tag + amount + fee + nonce + vu + memo

def handle_transaction(pkno, infile):
    with open(infile) as json_file:
        data = json.load(json_file)
        # splits into msg_type 'sendPayment': payment_dict
        for _, payment_dict in data.items():
            # splits entries of 'payment' : txn_info
            for _, txn_info in payment_dict.items():
                print(txn_info)
                txn_info['to'] = bytes(txn_info['to'], 'utf8')
                txn_info['from'] = bytes(txn_info['from'], 'utf8')
                txn_info['memo'] = bytes(txn_info['memo'], 'utf8')
                return txn_info
