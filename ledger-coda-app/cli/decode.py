from ledgerblue.comm import getDongle
from ledgerblue.commException import CommException
import json
import hashlib
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

# lengths in bytes (for reference?)
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

def handle_transaction(infile):
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
                return(txn_info)

def handle_pubkey(infile):
    with open(infile) as json_file:
        data = json.load(json_file)
        for key, value in data.items():
            bytes_val = b58_decode(value)
            lead_byte = bytes([bytes_val[0]])
            if lead_byte == value_to_version_byte['non_zero_curve_point_compressed']:
                bytes_x = bytes_val[1:]
                x = int.from_bytes(bytes_x, 'little')
                if is_curvepoint(x):
                    return bytes_x
                else:
                    raise RuntimeError('Failure: the public key you supplied is not on the curve.')
            else:
                raise RuntimeError('Failure: the file you supplied doesn\'t start with the compressed public key value byte.')

def handle_input(request, infile):
    if request == 'transaction':
        return handle_transaction(infile)
    elif request == 'publickey':
        return infile
    else:
      raise RuntimeError('Usage: %s request infile outfile, request = {transaction, publickey}' % sys.argv[0])
