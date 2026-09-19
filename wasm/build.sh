#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
CORE_DIR="${SCRIPT_DIR}/../core"
DIST_DIR="${SCRIPT_DIR}/dist"

mkdir -p "${DIST_DIR}"

if ! command -v emcc &> /dev/null; then
    echo "Error: emcc (Emscripten compiler) is not installed or not in PATH."
    echo "To install Emscripten SDK: https://emscripten.org/docs/getting_started/downloads.html"
    exit 1
fi

echo "Compiling C++ Core to WebAssembly..."

emcc -O3 -flto \
    -std=c++20 \
    -I"${CORE_DIR}/include" \
    "${CORE_DIR}/src/BitBuffer.cpp" \
    "${CORE_DIR}/src/QrSegment.cpp" \
    "${CORE_DIR}/src/QrCode.cpp" \
    "${CORE_DIR}/src/QrTopology.cpp" \
    "${CORE_DIR}/src/QrShapeRenderer.cpp" \
    "${CORE_DIR}/src/QrEyeRenderer.cpp" \
    "${CORE_DIR}/src/QrSvgBuilder.cpp" \
    "${CORE_DIR}/src/QrImageProcessor.cpp" \
    "${SCRIPT_DIR}/bindings.cpp" \
    --bind \
    -s WASM=1 \
    -s ALLOW_MEMORY_GROWTH=1 \
    -s MODULARIZE=1 \
    -s EXPORT_NAME="createQrEngine" \
    -s EXPORTED_RUNTIME_METHODS='["ccall", "cwrap"]' \
    -s ENVIRONMENT="web,webview,worker,node" \
    -o "${DIST_DIR}/qr_engine.js"

echo "WebAssembly compilation completed successfully -> ${DIST_DIR}/qr_engine.wasm"
