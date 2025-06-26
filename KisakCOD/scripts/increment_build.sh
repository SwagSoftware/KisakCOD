#!/bin/bash

# Check if run from KaisakCOD root (which must contain src/)
if [[ ! -d "src" ]]; then
    echo "This must be ran from the KaisakCOD root which contains CMakeLists.txt"
    exit 1
fi

BUILD_FILE="src/buildnumber.txt"
HEADER_FILE="src/buildnumber.h"

# Read existing build number or start at 0
if [[ -f "$BUILD_FILE" ]]; then
    BUILD_NUMBER=$(<"$BUILD_FILE")
else
    BUILD_NUMBER=0
fi

# Increment
BUILD_NUMBER=$((BUILD_NUMBER + 1))

# Write new number back
echo "$BUILD_NUMBER" > "$BUILD_FILE"

# Generate header
cat <<EOF > "$HEADER_FILE"
#pragma once
#define BUILD_NUMBER $BUILD_NUMBER
EOF

echo "Updated build number to $BUILD_NUMBER"
