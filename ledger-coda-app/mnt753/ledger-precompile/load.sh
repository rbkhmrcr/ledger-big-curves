#!/bin/bash

# cat $(BOLOS_SDK)/include/bolos_target.h | grep TARGET_ID | cut -f3 -d' '
TARGET_ID="0x31100004"
# cat $(BOLOS_SDK)/include/bolos_version.h | grep define | cut -f2 -d'"'
TARGET_VERSION="1.6.0" 
APP_NAME="Coda"
APP_VERSION="0.0.1"
ICON_NAME="nanos_app_coda.gif"
BIP_PATH="44'/49370'"
# this is generated with 
# python3 $(BOLOS_SDK)/icon3.py --hexbitmaponly nanos_app_coda.gif
ICON_HEX="010000000000ffffffffffffffffffffff0ff00ff0cff3cff3cff3cff30ff00ff0ffffffffffffffff"

APP_LOAD_PARAMS="--appFlags 0x00 \
         --tlv --targetId $TARGET_ID \
         --targetVersion=$TARGET_VERSION \
         --delete --fileName bin/app.hex \
         --appName $APP_NAME \
         --appVersion $APP_VERSION \
         --icon $ICON_HEX \
         --path $BIP_PATH"

python3 -m ledgerblue.loadApp $APP_LOAD_PARAMS
