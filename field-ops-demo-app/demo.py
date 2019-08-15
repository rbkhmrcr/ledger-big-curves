#!/usr/bin/env python
#*******************************************************************************
#*   Ledger Blue
#*   (c) 2016 Ledger
#*
#*  Licensed under the Apache License, Version 2.0 (the "License");
#*  you may not use this file except in compliance with the License.
#*  You may obtain a copy of the License at
#*
#*      http://www.apache.org/licenses/LICENSE-2.0
#*
#*  Unless required by applicable law or agreed to in writing, software
#*  distributed under the License is distributed on an "AS IS" BASIS,
#*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#*  See the License for the specific language governing permissions and
#*  limitations under the License.
#********************************************************************************
from ledgerblue.comm import getDongle
from ledgerblue.commException import CommException
from secp256k1 import PublicKey

a = 0x00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000b
b = 0x00007da285e70863c79d56446237ce2e1468d14ae9bb64b2bb01b10e60a5d5dfe0a25714b7985993f62f03b22a9a3c737a1a1e0fcf2c43d7bf847957c34cca1e3585f9a80a95f401867c4e80f4747fde5aba7505ba6fcf2485540b13dfc8468a
mnt6_g1_x = 0x0000255f8e876e831147412cfb1002284f30338088131c2437e884c4997fd1dcb409367d0c0d5fc5e818771b931f1d5bdd069ce5e3c57b6df120cee3cd9d867e66d11acbf7da60895b8b3d9d442c4c4123329a6fefa9a1f3f7a1fbd93a7bffb8
mnt6_g1_y = 0x000128c02fff6e2eb3fca70dc1063bac34551801202a3585bdd6d7722c6c07d7873bb02d4c7a18ed9c4bd3c7ed0ffb31c57e610dc7a593cce5a792e94d0020c335b74d9992f5cbf4b2cc4c42eff9a5a6c4521df9855687139f0c51754c0ccc49

field_modulus = 0x0001c4c62d92c41110229022eee2cdadb7f997505b8fafed5eb7e8f96c97d87307fdb925e8a0ed8d99d124d9a15af79db26c5c28c859a99b3eebca9429212636b9dff97634993aa4d6c381bc3f0057974ea099170fa13a4fd90776e240000001
group_order = 0x0001c4c62d92c41110229022eee2cdadb7f997505b8fafed5eb7e8f96c97d87307fdb925e8a0ed8d99d124d9a15af79db117e776f218059db80f0da5cb537e38685acce9767254a4638810719ac425f0e39d54522cdd119f5e9063de245e8001


textToSign = b''
while True:
        data = input('Enter text to sign, end with an empty line : ')
        if len(data) == 0:
                break
        textToSign += data.encode() + b'\n'

dongle = getDongle(True) # True here means debug is on

try:
    publicKey = dongle.exchange(bytes.fromhex('8004000000'))
except:
    if comm.sw == 0x6804:
        raise RuntimeError('Invalid status from Ledger: %x. Is the device unlocked?' % comm.sw)
print('publicKey ', publicKey.hex())

try:
        offset = 0
        while offset != len(textToSign):
                if (len(textToSign) - offset) > 255:
                        chunk = textToSign[offset : offset + 255]
                else:
                        chunk = textToSign[offset:]
                if (offset + len(chunk)) == len(textToSign):
                        p1 = b'\x80'
                else:
                        p1 = b'\x00'
                apdu = bytes.fromhex('8002') + p1 + b'\x00' + bytes([len(chunk)]) + chunk
                signature = dongle.exchange(apdu)
                offset += len(chunk)
        print('signature ', signature.hex())
        # publicKey = PublicKey(bytes(publicKey), raw=True)
        # signature = publicKey.ecdsa_deserialize(bytes(signature))

        # y^2
        ans = hex(pow(mnt6_g1_y, 2, field_modulus))
        padded_ans = ans[2:].zfill(192)
        print("y^2 from python", padded_ans)
        print("y^2 from ledger", signature.hex()[:192])

        # x^3 + ax + b
        ansx = hex((pow(mnt6_g1_x, 3, field_modulus) + a*mnt6_g1_x  + b) % field_modulus)
        padded_ansx = ansx[2:].zfill(192)
        print("x^2 + ax + b from python", padded_ansx)
        print("x^2 + ax + b from ledger", signature.hex()[192:])
        print(signature.hex()[:192] == padded_ansx)
        print(signature.hex()[192:] == padded_ans)
        print(signature.hex()[:192] == signature.hex()[192:])

        # print "verified " + str(publicKey.ecdsa_verify(bytes(textToSign), signature))

except CommException as comm:
        if comm.sw == 0x6985:
                print('Aborted by user')
        else:
                print('Invalid status %x' % comm.sw)
