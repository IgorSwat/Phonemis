#!/bin/bash

# Exit on error
set -e

# --- Configuration ---
PROJECT_NAME="phonemis"
SOURCE_DIR=$(pwd)
BUILD_ROOT="build_ios"
OUTPUT_DIR="output_ios"

# Option handling
if [ "$1" == "--clean" ]; then
    # Clean build and output directories
    rm -rf "$BUILD_ROOT" "$OUTPUT_DIR"
    echo "Cleaned $BUILD_ROOT and $OUTPUT_DIR"
    exit 0
fi

# Check if running on macOS
if [[ "$OSTYPE" != "darwin"* ]]; then
    echo "Error: iOS builds require macOS and Xcode."
    exit 1
fi

echo "Starting static library build for iOS..."

# Clean previous builds
rm -rf "$OUTPUT_DIR"
mkdir -p "$OUTPUT_DIR"

# Function to build for a specific SDK/Arch
build_ios_target() {
    SDK=$1      # iphoneos or iphonesimulator
    ARCH=$2     # arm64 or x86_64
    
    echo "------------------------------------------------"
    echo "Targeting: $SDK ($ARCH)"
    echo "------------------------------------------------"
    
    TARGET_BUILD_DIR="$BUILD_ROOT/${SDK}_${ARCH}"
    mkdir -p "$TARGET_BUILD_DIR"

    cmake -S "$SOURCE_DIR" -B "$TARGET_BUILD_DIR" -G Xcode \
        -DCMAKE_SYSTEM_NAME=iOS \
        -DCMAKE_OSX_SYSROOT="$SDK" \
        -DCMAKE_OSX_ARCHITECTURES="$ARCH" \
        -DCMAKE_XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH=NO \
        -DCMAKE_INSTALL_PREFIX="$OUTPUT_DIR/$SDK/$ARCH"

    # Build the project (Xcode generator)
    cmake --build "$TARGET_BUILD_DIR" --config Release

    # Move built .a files from Release-* to OUTPUT_DIR
    # Find the Release directory (e.g., Release-iphoneos or Release-iphonesimulator)
    release_dir=$(find "$TARGET_BUILD_DIR" -type d -name "Release-*" | head -n 1)
    if [ -d "$release_dir" ]; then
        mkdir -p "$OUTPUT_DIR/$SDK/$ARCH/lib"
        # Move all .a files to OUTPUT_DIR
        find "$release_dir" -name "*.a" -exec mv {} "$OUTPUT_DIR/$SDK/$ARCH/lib/" \;
    fi

    # Install using CMake's install command (avoids requiring an Xcode 'install' target)
    cmake --install "$TARGET_BUILD_DIR" --config Release --prefix "$OUTPUT_DIR/$SDK/$ARCH"
}

# 1. Build for Physical Device (arm64)
build_ios_target "iphoneos" "arm64"

# 2. Build for Simulator (x86_64 for Intel Macs)
build_ios_target "iphonesimulator" "x86_64"

# 3. Build for Simulator (arm64 for Apple Silicon Macs)
build_ios_target "iphonesimulator" "arm64"

echo "------------------------------------------------"
echo "Build complete. Your .a files are located in:"
echo "  - $OUTPUT_DIR/iphoneos/arm64/lib"
echo "  - $OUTPUT_DIR/iphonesimulator/x86_64/lib"
echo "  - $OUTPUT_DIR/iphonesimulator/arm64/lib"