#!/bin/bash
# Android NDK build script for Raxter L Tracing

set -e

NDK_PATH="${ANDROID_NDK_HOME:-/opt/android-ndk}"
API_LEVEL=24
ABI=arm64-v8a
BUILD_TYPE=Release

usage() {
    echo "Usage: $0 [options]"
    echo "Options:"
    echo "  --ndk PATH       Android NDK path (default: \$ANDROID_NDK_HOME or /opt/android-ndk)"
    echo "  --api LEVEL      Android API level (default: 24)"
    echo "  --abi ABI        Target ABI (default: arm64-v8a)"
    echo "  --debug          Build debug version"
    echo "  --clean          Clean build directory"
    exit 1
}

while [[ $# -gt 0 ]]; do
    case $1 in
        --ndk) NDK_PATH="$2"; shift 2 ;;
        --api) API_LEVEL="$2"; shift 2 ;;
        --abi) ABI="$2"; shift 2 ;;
        --debug) BUILD_TYPE=Debug; shift ;;
        --clean) CLEAN=1; shift ;;
        *) usage ;;
    esac
done

if [[ ! -d "$NDK_PATH" ]]; then
    echo "Error: NDK not found at $NDK_PATH"
    echo "Set ANDROID_NDK_HOME or use --ndk"
    exit 1
fi

BUILD_DIR="build_android_${ABI}_${BUILD_TYPE}"

if [[ $CLEAN == 1 ]]; then
    rm -rf "$BUILD_DIR"
fi

mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

cmake .. \
    -DCMAKE_TOOLCHAIN_FILE="$NDK_PATH/build/cmake/android.toolchain.cmake" \
    -DANDROID_ABI="$ABI" \
    -DANDROID_PLATFORM="android-$API_LEVEL" \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
    -G Ninja

ninja

echo "Build complete: $BUILD_DIR/libmadel.so"