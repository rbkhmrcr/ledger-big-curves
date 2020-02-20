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

b = 7
p = 5543634365110765627805495722742127385843376434033820803590214255538854698464778703795540858859767700241957783601153
n = 5543634365110765627805495722742127385843376434033820803592568747918351978899288491582778380528407187068941959692289

g = [1, 1587713460471950740217388326193312024737041813752165827005856534245539019723616944862168333942330219466268138558982]
gg = [3984487199923362794985200050720904058574926811961808702580466496168551814521559693353044992305458034548907156963327, 4440460323056502728140008727514031409702167863188492196781275939597847013104288050916106332325943645372246775229397]

def is_on_curve(P1):
    lhs = (pow(P1[0], 3, p) + b) % p
    rhs = pow(P1[1], 2, p)
    return lhs == rhs

def point_add(P1, P2):
    if (P1 is None):
        return P2
    if (P2 is None):
        return P1
    if (P1[0] == P2[0] and P1[1] != P2[1]):
        return None
    if (P1 == P2):
        lam = ((3 * P1[0] * P1[0]) * pow(2 * P1[1], p - 2, p)) % p
    else:
        lam = ((P2[1] - P1[1]) * pow(P2[0] - P1[0], p - 2, p)) % p
    x3 = (lam * lam - P1[0] - P2[0]) % p
    return [x3, (lam * (P1[0] - x3) - P1[1]) % p]

N = 382

def point_mul(P, n):
    R = None
    for i in range(N):
        if ((n >> i) & 1):
            R = point_add(R, P)
        P = point_add(P, P)
    return R

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
                received = dongle.exchange(apdu)
                offset += len(chunk)
        print('received ', received.hex())

except CommException as comm:
        if comm.sw == 0x6985:
                print('Aborted by user')
        else:
                print('Invalid status %x' % comm.sw)
