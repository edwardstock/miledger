#!/usr/bin/env bash

@PYTHON_BIN@ -m ledgerblue.loadApp \
  --apdu \
  --appFlags 0x40 \
  --path "44'/60'" \
  --curve secp256k1 \
  --tlv \
  --targetId 0x31100004 \
  --delete \
  --fileName app.hex \
  --appName Minter \
  --appVersion @LEDGER_APP_VERSION@ \
  --dataSize 0 \
  --icon 0100000000ffffff00ffffffffffffe7ebcfe99fec37e667e7e7e7e7e7e7e7e7e7e7e7ffffffffffff