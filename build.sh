#!/bin/bash
# Linux build script for Raxter L Tracing using Meson

set -e

BUILD_DIR="build_linux"
BUILD_TYPE="debugoptimized"

usage() {
    echo "Usage: $0 [options]"
    echo "Options:"
    echo "  --build-dir DIR    Build directory (default: build_linux)"
    echo "  --type TYPE        Build type: debug, debugoptimized, release (default: debugoptimized)"
    echo "  --clean            Clean build directory"
    echo "  --no-validation    Disable Vulkan validation layers"
    echo "  --max-bounces N    Max light bounces 1-8 (default: 8)"
    exit 1
}

MESON_ARGS=()

while [[ $# -gt 0 ]]; do
    case $1 in
        --build-dir) BUILD_DIR="$2"; shift 2 ;;
        --type) BUILD_TYPE="$2"; shift 2 ;;
        --clean) CLEAN=1; shift ;;
        --no-validation) MESON_ARGS+=("-Denable-validation=false"); shift ;;
        --max-bounces) MESON_ARGS+=("-Dmax-light-bounces=$2"); shift 2 ;;
        *) usage ;;
    esac
done

if [[ $CLEAN == 1 ]]; then
    rm -rf "$BUILD_DIR"
fi

meson setup "$BUILD_DIR" \
    --buildtype="$BUILD_TYPE" \
    "${MESON_ARGS[@]}"

meson compile -C "$BUILD_DIR"

echo "Build complete: $BUILD_DIR/madel-demo"