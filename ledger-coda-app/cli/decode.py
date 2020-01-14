import hashlib, struct, sys, base58, json, binascii
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
        b'\x9a': 'signature',
        b'\xcd': 'non_zero_curve_point',
        b'\xca': 'non_zero_curve_point_compressed' ,
        b'\x03': 'random_oracle_base',
        b'\x5a': 'private_key',
        b'\xa2': 'user_command_memo',
        b'\x9d': 'receipt_chain_hash',
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

def pk_encode(hex_pk):
    l = len(hex_pk)
    assert l//2 * 2 == l
    pkx = hex_pk[:l//2]
    pky = hex_pk[l//2:]
    v = value_to_version_byte['non_zero_curve_point_compressed'] + bytes(hex_pk, encoding='utf8')
    return base58.b58encode_check(v).decode("utf8")

# b58_pk is version byte || sign bit || x coord
def pk_decode(b58_pk):
    bpk = base58.b58decode_check(b58_pk)
    vb = bytes([bpk[0]])
    # assert vb == value_to_version_byte['non_zero_curve_point_compressed']
    head_bit = bytes([bpk[1]])
    tail_bit = bytes([bpk[-1]])
    pkx = bpk[3:-1]
    x = int.from_bytes(pkx, byteorder='little')
    pkx = x.to_bytes(96, byteorder='little')
    assert schnorr.is_curve_point(x) == True
    return tail_bit, pkx


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

# flip endianness
def sig_decode(r):
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
    assert len(reply)//2 * 2 == len(reply)
    r = sig_decode(reply[:len(reply)//2])
    s = sig_decode(reply[len(reply)//2:])
    # print(schnorr.schnorr_verify(msg, pk, s))
    print(json.dumps({'field': r, 'scalar': s}))
    return

def handle_pk_reply(pk):
    b58_pk = base58.b58encode(value_to_version_byte['non_zero_curve_point_compressed'] + bytes(pk)).decode("utf-8")
    print(json.dumps({'public_key': b58_pk}))
    return

def test_decode(pk):
    b, x = pk_decode(pk)
    # print(binascii.hexlify(x))
    return

def test_encode(pk):
    print(pk_encode(pk))
    return

hex_keys = ['00002e01950636082a7c75441fd029aaa09b6b6e079efe13f59f09d08c67263d9d2f2d66809836f1bbd0eff8536ea3876778d5cc3fa5497a99cd28653a526b379cfe0ba33e35ef2df75b8de30ae1750aa6340eccab679683497f827bef5a677f0001650e5a527bfcac623deee15a5984e0a4d5fde26629b85379b233cfb5e5738959148d45c0b0577708b1da1a792d2012338d905311f3a9334fa4608388d8d52c1e63cbf7cdc5ac6d26b446ecf1eb0d5f2a9434d415b447c2188f317699f979',
    '0000ce2db5b6a9ad988892a62ae3123886ec37a2b90e563ece53eeab8acc2a9fbfac87d76fdf4a7c760045f9967b5541e96dddaa53631234d82e31751f53e34ed998253f393e9c6414e27614cb65eafe02cdae5db99f1a363820fd6307830592000029448b2a6089c9222d6642335d904f2eab357ea9b98919cbecea34de62b70ba744c30be97163c7d2a2e2dd3921eec7a93a3fd89bc368eab637c354c895354d8e7ee46f3abddbebb7481cf03485b794adf0d6d944b96e31a1bbe4cbc953cc']


b58_keys = [b'tNci9iZe1p3KK4MCcqDa52mpxBTveEm3kqZMm7vwJF9uKzGGt1pCHVNa2oMevDb1HDAs4bNdMQLNbD8N3tkCtKNGM53obE9qFkkhmqMnKRLNLiSfPJuLGsSwqnL3HxSqciJoqJJJmq5Cfb',
    b'tNciczxpMfZ4eW1ZPP9NVK2vxcm9cCHvTBWMe8Nskn2A25P1YqcdvFCD5LvgngraiCmAsnC8zWAiv5pwMYjrUwpMNYDePMQYiXX7HVMjrnB1JkEckyayvsAm2Bo4EQBWbHXD5Cxp65PZy5',
    b'tNciGSnmKWkTLsEx49jgKEXtWMddaT72YxarPhEcSGcHTJJm3C6QTY2bic2KnRfvDT4LPRYMkdYno93JEZeH5SfTzNmMpFuERuLcvWrEU2wjr5xyvaqpmReuGsgSHsKjopPASz6K3WNrS1',
    b'tNci9P6wfuz1eahE1ZmXMYFRkAr4dP6Bk4NpioS5DhrnLcmmRuWQpFjAHAcPMggsqR4NbxuGqGtmaMDexo59WqYi6QDLMtPJkpN8FCYacQTgnJKDA581xRFDUvZKFdmq7qvz7ekg4c6mYE',
    b'tNcihEh77DbibdWrdGgTsrhSb3ccUm2cGGVDRitpxRKXjTP9f8KuxBoes9MBTxRH7rzoGA3Kzs27L2gmrBEppPTubX2AgN9GkBJ6fHGWBKyB39zhaemjWQFjdG4YZkGrESp7ZxNd9r2n3S',
    b'tdNDx8htq6yao9NC7SnwGh8KGGs9iwtDy9HfYHk512hH2FVj8EPxczE5dTp4yZ6Dee9S5BP2XbSFcKUeDKFeQNMjraEV4iQQz8xGdsgJHAFgrvU4kPSw34KoJchZQYma3EZKbghwbT2CeU',
    b'tdNEJC1j1xYpY7SdUEurBEyDQpPbCnAYefsqmCcRbNx42VWaGq5GsvsxRQuxVuvH3GmLVbhSPuST9GWBeLEB8LGP7QHQjAUTpLkHfdW1GUV2S4aQQH86ut3BRx1q7ZaKijrYpoQJN7mn5p']

if __name__ == "__main__":
    assert base58.b58encode(b'hello') == b'Cn8eVZg'
    assert base58.b58encode_check(b'hello') == b'2L5B5yqsVG8Vt'
    assert base58.b58decode(b'Cn8eVZg') == b'hello'
    assert base58.b58decode_check(b'2L5B5yqsVG8Vt') == b'hello'
    [test_encode(pk) for pk in hex_keys]
    [test_decode(pk) for pk in b58_keys]
