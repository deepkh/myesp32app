#!/bin/bash

set -e

host=$1

echo "Update FW to $1"
python3 ./scripts/OTAFirmwareUpdate.py http://$host/ota/upload apps/$host/firmware.bin

