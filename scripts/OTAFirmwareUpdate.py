#!/usr/bin/env python3
import sys
import os
import requests

def ota_firmware_update(url, firmware_path):
    if not os.path.isfile(firmware_path):
        print(f"Error: Firmware file '{firmware_path}' does not exist.")
        return 1

    try:
        with open(firmware_path, 'rb') as f:
            files = {'file': (os.path.basename(firmware_path), f, 'application/octet-stream')}
            print(f"Uploading '{firmware_path}' to '{url}'...\n")
            
            # Create a session to inspect headers
            with requests.Session() as session:
                req = requests.Request('POST', url, files=files)
                prepped = session.prepare_request(req)

                # Dump request headers
                print("=== Request Headers ===")
                for k, v in prepped.headers.items():
                    print(f"{k}: {v}")
                print("\n=== Sending Request ===\n")

                # Send the request
                response = session.send(prepped)

                # Dump response headers and body
                print("=== Response Headers ===")
                for k, v in response.headers.items():
                    print(f"{k}: {v}")
                print("\n=== Response Body ===")
                print(response.text)
    except requests.exceptions.RequestException as e:
        print(f"Error: Failed to upload firmware - {e}")
        return 1

    if response.status_code == 200 and response.text.strip() == "OK":
        print("\nFirmware upload successful.")
        return 0
    else:
        print(f"\nError: Upload failed. Status code: {response.status_code}")
        return 1

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print(f"Usage: {sys.argv[0]} <OTA_URL> <FIRMWARE_PATH>")
        sys.exit(1)

    ota_url = sys.argv[1]
    firmware_file = sys.argv[2]
    sys.exit(ota_firmware_update(ota_url, firmware_file))
