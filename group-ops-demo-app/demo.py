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
import msgpack
import sys
import os
import struct

a = 0x00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000b
b = 0x00007da285e70863c79d56446237ce2e1468d14ae9bb64b2bb01b10e60a5d5dfe0a25714b7985993f62f03b22a9a3c737a1a1e0fcf2c43d7bf847957c34cca1e3585f9a80a95f401867c4e80f4747fde5aba7505ba6fcf2485540b13dfc8468a
mnt6_g1 = [0x0000255f8e876e831147412cfb1002284f30338088131c2437e884c4997fd1dcb409367d0c0d5fc5e818771b931f1d5bdd069ce5e3c57b6df120cee3cd9d867e66d11acbf7da60895b8b3d9d442c4c4123329a6fefa9a1f3f7a1fbd93a7bffb8, 0x000128c02fff6e2eb3fca70dc1063bac34551801202a3585bdd6d7722c6c07d7873bb02d4c7a18ed9c4bd3c7ed0ffb31c57e610dc7a593cce5a792e94d0020c335b74d9992f5cbf4b2cc4c42eff9a5a6c4521df9855687139f0c51754c0ccc49]
mnt6_g1_old = [0xb0d6e141836d261dbe17959758b33a19987126cb808dfa411854cf0a44c0f4962eca2a213ffeaa770dad44f59f260ac64c9fcb46da65cbc9eebe1ce9b83f91a64b685106d5f1e4a05ddfae9b2e1a567e0e74c1b7ff94cc3f361fb1f064aa, 0x30bd0dcb53b85bd013043029438966ffec9438150ad06f59b4cc8dda8bff0fe5d3f4f63e46ac91576d1b4a15076774feb51ba730f83fc9eb56e9bcc9233e031577a744c336e1edff5513bf5c9a4d234bcc4ad6d9f1b3fdf00e16446a8268]

mnt6_q = [16364236387491689444759057944334173579070747473738339749093487337644739228935268157504218078126401066954815152892688541654726829424326599038522503517302466226143788988217410842672857564665527806044250003808514184274233938437290, 4510127914410645922431074687553594593336087066778984214797709122300210966076979927285161950203037801392624582544098750667549188549761032654706830225743998064330900301346566408501390638273322467173741629353517809979540986561128]

p = 0x0001c4c62d92c41110229022eee2cdadb7f997505b8fafed5eb7e8f96c97d87307fdb925e8a0ed8d99d124d9a15af79db26c5c28c859a99b3eebca9429212636b9dff97634993aa4d6c381bc3f0057974ea099170fa13a4fd90776e240000001
group_order = 0x0001c4c62d92c41110229022eee2cdadb7f997505b8fafed5eb7e8f96c97d87307fdb925e8a0ed8d99d124d9a15af79db117e776f218059db80f0da5cb537e38685acce9767254a4638810719ac425f0e39d54522cdd119f5e9063de245e8001

bigk = 0x30bd0dcb53b85bd013043029438966ffec9438150ad06f59b4cc8dda8bff0fe5d3f4f63e46ac91576d1b4a15076774feb51ba730f83fc9eb56e9bcc9233e031577a744c336e1edff5513bf5c9a4d234bcc4ad6d9f1b3fdf00e16446a8268

new_one = 0xe41e93acde012cab875fe72705e435e027785d45960369e5fbf2578299e00e2de683a82dfae41694f5eed605a51b55e3a5a6cca7b875f8f277ef6f9bb3ae7d568583fdd9c0d34d9dccaabbbe8725fb7a429d98f1742fef934c9dafd5400e
new_two = 0x531f1dea3304278bd3bf14f82d05c38342e0fbe8a2971b1d3e8957878dc9395d9aded081483c066b5df957ee2a891507815d3349aa2fedbee49a107bedbfb40d19f81d073730493840e07595008fa081f616c7012879b957f591aec3164c
new_three = 0x1bbed033fb3b720957f478ac81c7145f59e8aea129c363f6c3fb4f91fe576efe095ed75213f6cf8b3a5943ec69ebf71f35dceca15e3048aab8b16989c78f76af8210182ec0af13583e866689611eb6bb0d1f340c4dc92608eae1659afee82
new_four = 0x771e8f98bf7d84c98691db1e340d2f383e6d1069a6c74924a1c2f68e9b4d6a65fea5c8db95532aea3cfa6d6a485c10d8a89d4686f4eb8891210b483ecd3a8f9d27606db75f90a112db1ea3908bbb8744f2c1adf8e1f5258af202bf1c6355
new_five = 0x166e9626c93320502c19c21abfc08d7d883306a7f85bd12721b43b987da67893d836a00c9c90a84fed959d9884ab1ac85e25c034ecc4792e294501c03ed9ced34f6b30309e83cea655ed9ef0f785fa4336265af7c3115b4063425b92eea71

def is_on_curve(P1):
    lhs = (pow(P1[0], 3, p) + P1[0] * a + b) % p
    rhs = pow(P1[1], 2, p)
    print("x^3 + ax + b", lhs)
    print("y^2", rhs)
    return lhs == rhs

def point_add(P1, P2):
    if (P1 is None):
        return P2
    if (P2 is None):
        return P1
    if (P1[0] == P2[0] and P1[1] != P2[1]):
        return None
    if (P1 == P2):
        lam = ((3 * P1[0] * P1[0] + a) * pow(2 * P1[1], p - 2, p)) % p
    else:
        lam = ((P2[1] - P1[1]) * pow(P2[0] - P1[0], p - 2, p)) % p
    x3 = (lam * lam - P1[0] - P2[0]) % p
    return [x3, (lam * (P1[0] - x3) - P1[1]) % p]

N = 753

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
        print('comp mul', hex(point_mul(mnt6_g1, new_five)[0]))

except CommException as comm:
        if comm.sw == 0x6985:
                print('Aborted by user')
        else:
                print('Invalid status %x' % comm.sw)
