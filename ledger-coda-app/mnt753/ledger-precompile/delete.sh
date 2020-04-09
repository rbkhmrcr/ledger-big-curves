#!/bin/bash

TARGET_ID="0x31100004"
TARGET_VERSION="1.5.5" 
APP_NAME="Coda"
APP_VERSION="0.0.1"
ICON_NAME="nanos_app_coda.gif"
BIP_PATH="44'/49370'"

APP_DELETE_PARAMS="--targetId $TARGET_ID --appName $APP_NAME"

python3 -m ledgerblue.deleteApp $APP_DELETE_PARAMS
