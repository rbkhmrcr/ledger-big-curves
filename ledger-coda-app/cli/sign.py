from ledgerblue.comm import getDongle
from ledgerblue.commException import CommException
import argparse, sys, struct, os, hashlib
import decode, schnorr

# ledger functions

def get_version():
    apdu = b'\xE0'  # CLA byte
    apdu += b'\x01' # INS byte
    apdu += b'\x00' # P1 byte
    apdu += b'\x00' # P2 byte
    apdu += b'\x00' # LC byte
    apdu += b'\x00' # DATA byte
    v = dongle.exchange(apdu)
    print( "v" + str(v[0]) + '.' + str(v[1]) + '.' + str(v[2]) )
    return

def get_publickey(pkno):
    apdu = b'\xE0'  # CLA byte
    apdu += b'\x02' # INS byte
    apdu += b'\x00' # P1 byte
    apdu += b'\x00' # P2 byte
    apdu += b'\x00' # LC byte
    apdu += struct.pack('<I', int(pkno)) # DATA bytes
    pubkey = dongle.exchange(apdu)
    decode.handle_pk_reply(pubkey)
    return

# we should have the message stored so we can verify the signature?
def get_transaction(pkno, txn):
    apdu = decode.handle_txn_input(pkno, txn)
    reply = dongle.exchange(apdu)
    decode.handle_txn_reply(reply)
    return

## parsing

parser = argparse.ArgumentParser(description='Get public keys and signatures from Ledger device.')
parser.add_argument('--request',
        help='publickey or sign (for signing a transaction)')
parser.add_argument('--nonce',
        help='the nonce with which to derive the keys')
parser.add_argument('--transaction',
        help='the transaction to sign (in JSON)')

args = parser.parse_args()

dongle = getDongle(True)

try:
    if args.request == 'version':
        get_version()
    elif args.request == 'publickey':
        get_publickey(args.nonce)
    elif args.request == 'transaction':
        get_transaction(args.nonce, args.transaction)

except CommException as comm:
    if comm.sw == 0x6985:
        print('Aborted by user')
    elif comm.sw == 0x6B00:
        print('SW_DEVELOPER_ERR')
    elif comm.sw == 0x6B01:
        print('SW_INVALID_PARAM')
    elif comm.sw == 0x6B02:
        print('SW_IMPROPER_INIT')
    else:
        print('Invalid status from Ledger: ', comm.sw, 'Is the device unlocked?')
