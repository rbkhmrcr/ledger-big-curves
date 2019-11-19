from ledgerblue.comm import getDongle
from ledgerblue.commException import CommException
import json
import sys
import os
import struct # struct converts between python values and C structs (represented as python bytes objects)
import decode # contains base58 decoding/encoding, and our specific json handling
import schnorr

def packtxn(indict, output):
    # output += struct.pack("", indict['id'])
    output += struct.pack('?', indict['isDelegation'])
    output += struct.pack("I", indict['nonce'])
    output += struct.pack("96s", indict['from'])
    output += struct.pack("96s", indict['to'])
    output += struct.pack("I", indict['amount'])
    output += struct.pack("I", indict['fee'])
    output += struct.pack("32s", indict['memo'])
    return output

dongle = getDongle(True)

if len(sys.argv) != 4:
    raise RuntimeError('Format command : %s request input output, request = {transaction, publickey}' % sys.argv[0])

(_, request, infile, outfile) = sys.argv
if request != 'publickey' and request != 'transaction':
    raise RuntimeError('Format command : %s request input output, request = {transaction, publickey}' % sys.argv[0])

try:
    apdu = b'\xE0'

    if request == 'publickey':
        x = decode.handle_input(request, infile)
        apdu += b'\x02' # FIXME need to change main.c - this is just for tests
        apdu += struct.pack('96s', x)

        signature = dongle.exchange(apdu)
        print("signature " + str(signature).encode('hex'))

    elif request == 'transaction':
        indict = decode.handle_input(request, infile)
        apdu += b'\x04'
        apdu = packtxn(indict, apdu)

        signature = dongle.exchange(apdu)
        print("signature " + str(signature).encode('hex'))

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
