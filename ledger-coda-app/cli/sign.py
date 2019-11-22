from ledgerblue.comm import getDongle
from ledgerblue.commException import CommException
import json
import sys
import os
import struct # struct converts between python values and C structs (represented as python bytes objects)
import decode # contains base58 decoding/encoding, and our specific json handling
import schnorr
import hashlib

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

dongle = getDongle(True)

if len(sys.argv) != 4:
   raise RuntimeError('Format command : %s request input output, request = {version, publickey, transaction, streamedtransaction}' % sys.argv[0])

(_, request, infile, outfile) = sys.argv
if request != 'version' and request != 'publickey' and request != 'transaction' and request != 'streamedtransaction':
    raise RuntimeError('Format command : %s request input output, request = {version, publickey, transaction, streamedtransaction}' % sys.argv[0])

#define INS_VERSION       0x01
#define INS_PUBLIC_KEY    0x02
#define INS_SIGN          0x04
#define INS_HASH          0x08
print(request)

try:

    if request == 'version':
        apdu = b'\xE0'  # CLA byte
        apdu += b'\x01' # INS byte
        apdu += b'\x00' # P1 byte
        apdu += b'\x00' # P2 byte
        apdu += b'\x00' # LC byte
        apdu += b'\x00' # DATA byte
        v = dongle.exchange(apdu)
        print("v" + str(v[0]) + '.' + str(v[1]) + '.' + str(v[2]) )

    elif request == 'publickey':
        x = decode.handle_input(request, infile)
        apdu = b'\xE0'  # CLA byte
        apdu += b'\x02' # INS byte
        apdu += b'\x00' # P1 byte
        apdu += b'\x00' # P2 byte
        apdu += b'\x00' # LC byte
        apdu += struct.pack('<I', int(x)) # DATA bytes

        pubkey = dongle.exchange(apdu)
        print("public key " + pubkey.hex())
        pkx = pubkey[:96]
        print("hash digest " + hashlib.sha256(pkx).hexdigest())

    elif request == 'transaction':
        indict = decode.handle_input(request, infile)
        apdu = b'\xE0'  # CLA byte
        apdu += b'\x04' # INS byte
        apdu += b'\x00' # P1 byte
        apdu += b'\x00' # P2 byte
        apdu += b'\x00' # LC byte
        apdu = packtxn(indict, apdu)

        signature = dongle.exchange(apdu)
        print("signature " + signature.hex())

        txbytes = b''
        txbytes = packtxn(indict, txbytes)
        schnorr.schnorr_verify(signature, txbytes, pk)
        print("Verified signature")
        # FIXME are signatures encoded with base58 before broadcast to the network?
        indict['signature'] = decode.b58_encode(signature)

        with open(outfile, 'w') as f:
            json.dump(indict)
        print('Signed transaction written to ',  outfile)

    elif request == 'streamedtransaction':
        indict = decode.handle_input(request, infile)
        apdu = b'\xE0'  # CLA byte
        apdu += b'\x08' # INS byte
        apdu += b'\x00' # P1 byte
        apdu += b'\x00' # P2 byte
        apdu += b'\x00' # LC byte
        apdu = packtxn(indict, apdu)

        signature = dongle.exchange(apdu)
        print("signature " + signature.hex())

        txbytes = b''
        txbytes = packtxn(indict, txbytes)
        schnorr.schnorr_verify(signature, txbytes, pk)
        print("Verified signature")
        # FIXME are signatures encoded with base58 before broadcast to the network?
        indict['signature'] = decode.b58_encode(signature)

        with open(outfile, 'w') as f:
            json.dump(indict)
        print('Signed transaction written to ',  outfile)

    else:
        raise RuntimeError('Format command : %s request infile outfile, request = {transaction, publickey}' % sys.argv[0])

except CommException as comm:
    if comm.sw == 0x6985:
        print('Aborted by user')
    else:
        print('Invalid status from Ledger: ', comm.sw, 'Is the device unlocked?')
