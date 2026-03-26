#!/usr/bin/env bash

set -e

# Function to check parameters
CheckParams() {
    local allowed=("generate" "clean" "clean_all" "build" "clean_build" "fw_update" ) 
    local found=false

    # Loop through all script arguments
    for arg in "$@"; do
        for cmd in "${allowed[@]}"; do
            if [[ "$arg" == "$cmd" ]]; then
                echo "Parameter '$arg' is used."
                found=true
            fi
        done
    done

    if [[ "$found" == false ]]; then
        echo "No valid parameters (${allowed[@]}) were used."
        echo "Usage: $0 (${allowed[@]})"
        return 1
    fi
}

# Example usage with all script arguments
CheckParams "$@"

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"
APPS_DIR="${ROOT_DIR}/apps"

processed_count=0
skipped_count=0

generate() {
  python3 ../../scripts/EspConfigGenerator.py \
      EspConfigDefinition.yml \
      ../../myesp32/include/EspConfig.h \
      src/EspConfig.cpp
}

clean_all() {
  echo "CLEAN '$1'"
  rm -rf .pio .vscode || true
}

clean() {
  echo "CLEAN '$1'"
  pio run -e myesp32app -t clean
}

build() {
  pio run -e myesp32app 
}

clean_build() {
  clean
  pio run -e myesp32app 
}

fw_update() {
  local path="$1"
  local name=$(basename "$path")
  echo "fw_update '$path' '$name'"

  set +e

  python3 ../../scripts/OTAFirmwareUpdate.py http://$name/ota/upload $path/firmware.bin
}

for app_dir in "${APPS_DIR}"/*; do
    echo "${app_dir}"
    # Ensure it's a directory
    [[ -d "${app_dir}" ]] || continue

    # Check if it's a valid app
    if [[ -f "${app_dir}/EspConfigDefinition.yml" ]]; then
        echo "----------------------------------------"
        echo "Processing app: ${app_dir}"
        pushd "${app_dir}" 

        $@ ${app_dir}

        popd 

        ((processed_count++)) || true
    fi
done

echo "========================================"
echo "Processed apps: ${processed_count}"
