from ledgerblue.comm import getDongle
from ledgerblue.commException import CommException
import argparse, sys, struct, os, hashlib, json
from codaledgercli import decode, schnorr

# ledger functions

def get_version(dongle):
    apdu = b'\xE0'  # CLA byte
    apdu += b'\x01' # INS byte
    apdu += b'\x00' # P1 byte
    apdu += b'\x00' # P2 byte
    apdu += b'\x00' # LC byte
    apdu += b'\x00' # DATA byte
    v = dongle.exchange(apdu)
    print( "v" + str(v[0]) + '.' + str(v[1]) + '.' + str(v[2]) )
    return

def get_publickey(pkno, dongle):
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
def get_transaction(pkno, txn, dongle):
    apdu = decode.handle_txn_input(pkno, txn)
    reply = dongle.exchange(apdu)
    decode.handle_txn_reply(reply)
    return

def get_transaction_from_ints(pkno, msgx, msgm, dongle):
    apdu = decode.handle_ints_input(pkno, msgx, msgm)
    reply = dongle.exchange(apdu)
    decode.handle_txn_reply(reply)
    return

def report_error(req, err):
    if req == 'sign':
        print(json.dumps({'status': err, 'field': 'null', 'scalar': 'null'}))
    else:
        print(json.dumps({'status': err, 'x': 'null', 'y': 'null'}))
    return

def main():
    try:
        parser = argparse.ArgumentParser(description='Get public keys and signatures from Ledger device.')
        parser.add_argument('--request',
                help='publickey or sign (for signing a transaction)')
        parser.add_argument('--nonce',
                help='the nonce with which to derive the keys')
        parser.add_argument('--msgx',
                help='the x coordinate of the reciever pk')
        parser.add_argument('--msgm',
                help='the remaining transaction information to be signed')
        parser.add_argument('--transaction',
                help='the transaction to sign (in JSON)')

        args = parser.parse_args()

        dongle = getDongle(False)

        if args.request == 'version':
            get_version(dongle)
        elif args.request == 'publickey':
            get_publickey(args.nonce, dongle)
        elif args.request == 'transaction':
            get_transaction(args.nonce, args.transaction, dongle)
        elif args.request == 'sign':
            get_transaction_from_ints(args.nonce, args.msgx, args.msgm, dongle)
        else:
            report_error(args.request, 'Computation_aborted')

    except:
        comp_error_codes = [0x6985, 0x6B00, 0x6B01, 0x6B02]
        conn_error_codes = ['No dongle found', 'read error']
        if CommException in comp_error_codes:
            report_error(args.request, 'Computation_aborted')
        else:
            report_error(args.request, 'Hardware_wallet_not_found')

if __name__ == '__main__':
    main()
