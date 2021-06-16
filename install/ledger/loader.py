#!/usr/bin/env python

"""
*******************************************************************************
*   Standalone Ledger Command Line Application Loader
*   (c) 2019 Kin Foundation
*
*  Licensed under the Apache License, Version 2.0 (the "License");
*  you may not use this file except in compliance with the License.
*  You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
*  Unless required by applicable law or agreed to in writing, software
*  distributed under the License is distributed on an "AS IS" BASIS,
*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*  See the License for the specific language governing permissions and
*  limitations under the License.
********************************************************************************
"""

import os
import runpy
import sys

from ledgerblue.comm import getDongle

# os.add_dll_directory(os.getcwd())

# reconstructing the following command line produced by `make load`:
# python -m ledgerblue.loadApp --appFlags 0x40 --path "44'/2017'" --curve ed25519 --tlv --targetId 0x31100004 --delete --fileName bin/app.hex \
# --appName Kin --appVersion 0.0.1 --dataSize `cat debug/app.map |grep _nvram_data_size | tr -s ' ' | cut -f2 -d' '` \
# `ICONHEX=\`python ../nanos-secure-sdk/icon.py icon_kin.gif hexbitmaponly 2>/dev/null\` ; [ ! -z "$ICONHEX" ] && echo "--icon $ICONHEX"`

appName = 'Minter'
appVersion = '0.1.0'
appFlags = '0x40'
bip32Path = "44'/60'"
appFile = './bin/app.hex'
iconHex = '0100000000ffffff00ffffffffffffe7ebcfe99fec37e667e7e7e7e7e7e7e7e7e7e7e7ffffffffffff'
dataSize = '0'

if hasattr(sys, '_MEIPASS'):  # in pyinstaller
    appFile = os.path.join(sys._MEIPASS, 'app.hex')

# get targetId for Nano S
targetId = '0x31100004'
dongle = getDongle(True)
a = dongle.exchange(bytearray([0xe0, 0x01, 0x00, 0x00, 0x00]))
dongle.close()
# targetId = '0x' + binascii.hexlify(bytes(a[:4])).decode('ascii')
print('device targetId is {}'.format(targetId))

# -apdu
# --appFlags 0x40
# --path "44'/60'"
# --curve secp256k1
# --tlv
# --targetId 0x31100004
# --delete
# --fileName app.hex
# --appName Minter
# --appVersion 0.1.0
# --dataSize 0
# --icon 0100000000ffffff00ffffffffffffe7ebcfe99fec37e667e7e7e7e7e7e7e7e7e7e7e7ffffffffffff
sys.argv = [sys.argv[0], '--apdu', '--appFlags', appFlags, '--path', bip32Path, '--curve', 'secp256k1', '--tlv',
            '--targetId', targetId, '--delete', '--fileName', appFile, '--appName', appName, '--appVersion', appVersion,
            '--dataSize', dataSize, '--icon', iconHex]

runpy.run_module('ledgerblue.loadApp', run_name='__main__')
