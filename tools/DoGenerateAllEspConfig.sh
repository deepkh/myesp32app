#!/usr/bin/env bash

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"
APPS_DIR="${ROOT_DIR}/apps"

processed_count=0
skipped_count=0

for app_dir in "${APPS_DIR}"/*; do
    # Ensure it's a directory
    [[ -d "${app_dir}" ]] || continue

    # Check if it's a valid app
    if [[ -f "${app_dir}/EspConfigDefinition.yml" ]]; then
        echo "----------------------------------------"
        echo "Processing app: ${app_dir}"

        pushd "${app_dir}" > /dev/null

        python3 ../../tools/EspConfigGenerator.py \
            EspConfigDefinition.yml \
            ../../myesp32/include/EspConfig.h \
            src/EspConfig.cpp

        popd > /dev/null

        ((processed_count++))
    else
        echo "Skipping (no config): ${app_dir}"
        ((skipped_count++))
    fi
done

echo "========================================"
echo "Processed apps: ${processed_count}"
echo "Skipped apps: ${skipped_count}"
