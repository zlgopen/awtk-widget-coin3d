#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")" && pwd)"
BUILD_DIR="${ROOT}/build-gl3"
JOBS="$(sysctl -n hw.ncpu 2>/dev/null || nproc 2>/dev/null || echo 4)"

# 配置并编译库 + SDL3 旋转 Cube 示例
cmake -S "${ROOT}" -B "${BUILD_DIR}" \
  -DCOIN_GL_PROFILE=GL3 \
  -DCOIN_BUILD_EXAMPLES=ON

cmake --build "${BUILD_DIR}" --target Coin sdl3-example -j"${JOBS}"

# 运行
exec "${BUILD_DIR}/bin/sdl3-example"
