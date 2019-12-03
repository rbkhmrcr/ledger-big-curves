from ledgerblue.comm import getDongle
from ledgerblue.commException import CommException
import json
import sys
import os
import decode
import schnorr
import hashlib

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
    print("public key " + pubkey.hex())
    pkx = pubkey[:96]
    print("hash digest " + hashlib.sha256(pkx).hexdigest())
    return

def get_transaction(pkno, h, outfile):
    apdu = decode.handle_txn_input(pkno, h)
    reply = dongle.exchange(apdu)
    print("signature " + signature.hex())
    decode.handle_txn_reply(reply, outfile)
    return

def stream_sign(dongle, pkno, infile, outfile):
    apdu = decode.handle_stream_input(pkno, infile)
    reply = dongle.exchange(apdu)
    print("signature " + signature.hex())
    decode.handle_stream_reply(reply, outfile)
    return

errstring = 'Format command as \n %s request pk input output \n with request = {version, publickey, transaction, streamedtransaction}'
dongle = getDongle(True)

try:

    if len(sys.argv) == 2:
        (_, request) = sys.argv
        if request == 'version':
            get_version(dongle)
        else:
            print(errstring % sys.argv[0])

    elif len(sys.argv) == 3:
        (_, request, pkno) = sys.argv
        if request == 'publickey':
            get_publickey(dongle, pkno)
        else:
            print(errstring % sys.argv[0])

    elif len(sys.argv) == 5:
        (_, request, pkno, h, outfile) = sys.argv
        if request == 'transaction':
            get_transaction(dongle, pkno, h, outfile)
        elif request == 'streamedtransaction':
            # in this case h will be a file
            stream_sign(dongle, pkno, h, outfile)
        else:
            print(errstring % sys.argv[0])

    else:
        print(errstring % sys.argv[0])

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
