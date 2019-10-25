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

textToSign = ""
while True:
        data = raw_input("Enter text to sign, end with an empty line : ")
        if len(data) == 0:
                break
        textToSign += data + "\n"

""" 
we send E0 (is this handled for us?) || CLA || INS || P1 || P2 || payload
        INS_VERSION     = 0x01
        INS_PUBLICKEY   = 0x02
        INS_SIGN        = 0x04
        INS_HASH        = 0x08

        P1_FIRST        = 0x00
        P1_MORE         = 0x80
        P2_DISPLAY_HASH = 0x00
        P2_SIGN_HASH    = 0x01
    // exception codes
    #define SW_DEVELOPER_ERR 0x6B00
    #define SW_INVALID_PARAM 0x6B01
    #define SW_IMPROPER_INIT 0x6B02
    #define SW_USER_REJECTED 0x6985
    #define SW_OK            0x9000
"""

dongle = getDongle(True)
publicKey = dongle.exchange(bytes("8004000000".decode('hex')))
print "publicKey " + str(publicKey).encode('hex')
try:
        offset = 0
        signature = b''
        while offset <> len(textToSign):
                if (len(textToSign) - offset) > 255:
                        chunk = textToSign[offset : offset + 255]
                else:
                        chunk = textToSign[offset:]
                if (offset + len(chunk)) == len(textToSign):
                        p1 = 0x80
                else:
                        p1 = 0x00
                apdu = bytes("8002".decode('hex')) + chr(p1) + chr(0x00) + chr(len(chunk)) + bytes(chunk)
                signature += dongle.exchange(apdu)
                offset += len(chunk)
        print "signature " + str(signature).encode('hex')
        publicKey = PublicKey(bytes(publicKey), raw=True)
        signature = publicKey.ecdsa_deserialize(bytes(signature))
        print "verified " + str(publicKey.ecdsa_verify(bytes(textToSign), signature))
except CommException as comm:
        if comm.sw == 0x6985:
                print "Aborted by user"
        else:
                print "Invalid status " + comm.sw
