#!/usr/bin/env bash
# Build Coin via CMake for awtk-widget-coin3d.
# Profile comes from AWTK (SConstruct sets COIN_GL_PROFILE) or defaults to GL3.
# Output: 3rd/coin/build-<profile>/lib/libCoin.* and generated headers.
set -euo pipefail

ROOT="$(cd "$(dirname "$0")" && pwd)"
COIN_SRC="${ROOT}/coin"
PROFILE="$(printf '%s' "${COIN_GL_PROFILE:-GL3}" | tr '[:lower:]' '[:upper:]')"
case "${PROFILE}" in
  GL3|GLES3|GLES2) ;;
  *)
    echo "COIN_GL_PROFILE must be GL3 or GLES3 (got '${PROFILE}')" >&2
    exit 1
    ;;
esac
BUILD_DIR="${COIN_BUILD_DIR:-${COIN_SRC}/build-$(printf '%s' "${PROFILE}" | tr '[:upper:]' '[:lower:]')}"
JOBS="$(sysctl -n hw.ncpu 2>/dev/null || nproc 2>/dev/null || echo 4)"
BUILD_TYPE="${COIN_BUILD_TYPE:-Release}"

mkdir -p "${BUILD_DIR}"

if [[ ! -f "${BUILD_DIR}/CMakeCache.txt" ]]; then
  cmake -S "${COIN_SRC}" -B "${BUILD_DIR}" \
    -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" \
    -DCMAKE_CXX_STANDARD=17 \
    -DCOIN_GL_PROFILE="${PROFILE}" \
    -DCOIN_BUILD_SHARED_LIBS=ON \
    -DCOIN_BUILD_TESTS=OFF \
    -DCOIN_BUILD_DOCUMENTATION=OFF \
    -DCOIN_BUILD_AWESOME_DOCUMENTATION=OFF \
    -DCOIN_BUILD_EXAMPLES=OFF \
    -DCOIN_HAVE_JAVASCRIPT=OFF \
    -DHAVE_SOUND=OFF \
    -DHAVE_3DS_IMPORT_CAPABILITIES=OFF \
    -DUSE_EXTERNAL_EXPAT=OFF \
    -DUSE_EXCEPTIONS=OFF \
    -DUSE_SUPERGLU=OFF \
    -DCOIN_BUILD_SINGLE_LIB=ON
fi

cmake --build "${BUILD_DIR}" --target Coin -j"${JOBS}"

echo "Coin ${PROFILE} ready at ${BUILD_DIR}/lib"
