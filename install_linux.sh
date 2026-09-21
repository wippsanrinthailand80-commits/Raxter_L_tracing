#!/bin/bash
# Raxter L Tracing - Linux Installer
set -e

PREFIX="${PREFIX:-/usr/local}"
BUILD_DIR="$(dirname "$0")"

echo "Installing Raxter L Tracing to $PREFIX..."

# Install library
install -Dm644 libraxter.a "$PREFIX/lib/libraxter.a"

# Install headers
install -d "$PREFIX/include/raxter"
cp -r ../src/core/*.h "$PREFIX/include/raxter/" 2>/dev/null || true
cp -r ../src/renderer/*.h "$PREFIX/include/raxter/" 2>/dev/null || true
cp -r ../src/compute/*.h "$PREFIX/include/raxter/" 2>/dev/null || true
cp -r ../src/math/*.h "$PREFIX/include/raxter/" 2>/dev/null || true
cp -r ../src/platform/*.h "$PREFIX/include/raxter/" 2>/dev/null || true

# Install shaders
install -d "$PREFIX/share/raxter/shaders"
cp *.spv "$PREFIX/share/raxter/shaders/"

# Install demo
install -Dm755 raxter-demo "$PREFIX/bin/raxter-demo"

# Update library cache
ldconfig 2>/dev/null || true

echo "Installation complete!"
echo "Run: raxter-demo"
