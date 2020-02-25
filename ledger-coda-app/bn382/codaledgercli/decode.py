import hashlib, struct, sys, base58, json, binascii

p = 5543634365110765627805495722742127385843376434033820803590214255538854698464778703795540858859767700241957783601153
n = 5543634365110765627805495722742127385843376434033820803592568747918351978899288491582778380528407187068941959692289
a = 0
b = 7
scalar_bytes = 48
field_bytes = 48

def ba_to_b58(in_bytes, r, i):
    b0 = bytes(in_bytes)
    int0 = int.from_bytes(b0, 'big')
    if i == 0 or r == 'publickey':
        assert int0 < p
    else:
        assert int0 < n
    return base58.b58encode(b0).decode("utf8")

def split(in_bytes, request):
    l = len(in_bytes)
    assert l//2 * 2 == l
    bb0 = ba_to_b58(in_bytes[:l//2], request, 0)
    bb1 = ba_to_b58(in_bytes[l//2:], request, 1)
    return bb0, bb1

# INS_VERSION       0x01
# INS_PUBLIC_KEY    0x02
# INS_SIGN          0x04
# INS_HASH          0x08

def handle_ints_input(pkno, msgx, msgm):
    to = int(msgx).to_bytes(field_bytes, byteorder='big')
    msg = int(msgm).to_bytes(field_bytes, byteorder='big')
    apdu = b'\xE0'  # CLA byte
    apdu += b'\x04' # INS byte
    apdu += b'\x00' # P1 byte
    apdu += b'\x00' # P2 byte
    apdu += b'\x00' # LC byte
    apdu += struct.pack('<I', int(pkno)) # DATA bytes
    apdu += struct.pack("<48s", to)
    apdu += struct.pack("<48s", msg)
    return apdu

def handle_txn_reply(reply):
    r, s = split(bs, 'sig')
    print(json.dumps({'status': 'Ok', 'field': r, 'scalar': s}))
    return

def handle_pk_reply(pk):
    x, y = split(bpk, 'pk')
    print(json.dumps({'status': 'Ok', 'x': x, 'y': y}))
    return
