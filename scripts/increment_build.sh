#!/bin/bash

# Check if path is provided
if [[ -z "$1" ]]; then
    echo "Usage: $0 <path-to-src-folder>"
    exit 1
fi

BUILD_DIR="$1"
BUILD_FILE="$BUILD_DIR/buildnumber.txt"
HEADER_FILE="$BUILD_DIR/buildnumber.h"

# Check if a build number was not provided
if [[ -z "$2" ]]; then
    # Read existing build number or start at 0
    if [[ -f "$BUILD_FILE" ]]; then
        BUILD_NUMBER=$(<"$BUILD_FILE")
    else
        BUILD_NUMBER=0
    fi

    # Increment
    BUILD_NUMBER=$((BUILD_NUMBER + 1))
else
    BUILD_NUMBER="$2"
fi


# Write new number back
echo "$BUILD_NUMBER" > "$BUILD_FILE"

# Generate header
cat <<EOF > "$HEADER_FILE"
#pragma once
#define BUILD_NUMBER $BUILD_NUMBER

char* getBuildNumber();
int getBuildNumberAsInt();
EOF

echo "Updated build number to $BUILD_NUMBER"
