#!/bin/sh
set -e
cd "$(dirname "$0")/../.."
docker run --rm -u "$(id -u):$(id -g)" -v "$PWD":/src -w /src emscripten/emsdk:3.1.74 sh -c   "emcmake cmake -S . -B build-wasm -DCMAKE_BUILD_TYPE=Release -DVIENNAROUTE_BUILD_TESTS=OFF && cmake --build build-wasm --target viennaroute_web"
mkdir -p web/src/wasm
cp build-wasm/viennaroute_web.mjs build-wasm/viennaroute_web.wasm web/src/wasm/
