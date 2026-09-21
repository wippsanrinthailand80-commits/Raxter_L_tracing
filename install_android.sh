#!/bin/bash
# Raxter L Tracing - Android NDK Build & Deploy
# Run on development machine with Android NDK

set -e

NDK_PATH="${ANDROID_NDK_HOME:-/opt/android-ndk}"
ABI="${1:-arm64-v8a}"
API="${2:-24}"

if [ ! -d "$NDK_PATH" ]; then
    echo "ERROR: Android NDK not found at $NDK_PATH"
    echo "Set ANDROID_NDK_HOME or pass as first argument"
    exit 1
fi

echo "Building for $ABI (API $API) using NDK at $NDK_PATH"

cd "$(dirname "$0")"

# Build with CMake
cmake -B build_android \
    -DCMAKE_TOOLCHAIN_FILE="$NDK_PATH/build/cmake/android.toolchain.cmake" \
    -DANDROID_ABI="$ABI" \
    -DANDROID_PLATFORM="android-$API" \
    -DCMAKE_BUILD_TYPE=Release

cmake --build build_android -- -j$(nproc)

# Output locations
echo ""
echo "Build complete! Outputs in build_android/:"
ls -la build_android/
echo ""
echo "To deploy to device:"
echo "  adb push build_android/libraxter.so /data/local/tmp/"
echo "  adb push raxter-demo /data/local/tmp/"
echo "  adb shell /data/local/tmp/raxter-demo"
