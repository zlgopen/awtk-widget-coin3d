# GL Modern / COMPAT Parity Harness Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add a deterministic, headless-compatible COMPAT↔GL3 rendering regression harness that catches empty frames and material visual regressions in CI.

**Architecture:** A new `glparity-render` C++ executable renders a supplied Inventor scene into an SDL3 hidden window and saves raw RGBA plus JSON statistics. A Python-standard-library comparison tool validates non-background coverage and RMSE; a shell runner executes the same scene list against separately built COMPAT and GL3 renderers. The existing Ubuntu CI workflow builds both profiles, invokes the runner under Xvfb, and uploads failed artifacts.

**Tech Stack:** C++11, Coin3D scene graph APIs, SDL3/OpenGL, CMake, Python 3 `unittest` and stdlib, POSIX shell, GitHub Actions.

---

## File structure

| File | Responsibility |
| --- | --- |
| `CMakeLists.txt` | Add `tools/glparity` only while `COIN_BUILD_EXAMPLES=ON`. |
| `tools/glparity/CMakeLists.txt` | Define `glparity-render`, propagate Coin’s active GL profile macros, and link SDL3/Coin. |
| `tools/glparity/glparity_render.cpp` | Create profile-correct hidden SDL context, construct deterministic scene root, render, count pixels, and write RGBA/JSON output. |
| `tools/glparity/compare_rgba.py` | Validate raw buffers, compute pixel statistics/RMSE, write a PPM diff, and return a meaningful status. |
| `tools/glparity/tests/test_compare_rgba.py` | Synthetic unit tests for comparator pass/fail behavior and artifact generation. |
| `tools/glparity/scenes_p0.txt` | Versioned, deterministic P0 scene manifest. |
| `tools/glparity/thresholds.json` | Default/per-scene RMSE limits; `text2` override. |
| `tools/glparity/run_parity.sh` | Execute both renderers for every manifest item, apply per-scene threshold, aggregate failures. |
| `tools/glparity/README.md` | Local invocation, output format, row order, thresholds, and artifact investigation. |
| `.github/workflows/continuous-integration-workflow.yml` | Build COMPAT and GL3 harnesses, run parity under Xvfb, upload failures. |
| `docs/COIN_GL_PROFILE.md` | Link parity harness and document CI coverage boundaries. |

## Shared constants and CLI contract

- Fixed default render size: `400x300`.
- Fixed clear color: `0.12f, 0.14f, 0.18f`, quantized to `31, 36, 46`.
- A pixel is non-background when any RGB channel differs from clear by more than `8`; alpha is ignored for coverage and RMSE.
- `frame0000.rgba` is exactly `width * height * 4` bytes in unmodified `glReadPixels` bottom-row-first order.
- `glparity-render` CLI:

```text
glparity-render --scene <absolute-or-relative.iv> --output <directory>
                [--width 400] [--height 300] [--allow-empty]
```

- `compare_rgba.py` CLI:

```text
compare_rgba.py --reference <compat.rgba> --candidate <gl3.rgba>
                --width 400 --height 300 --rmse-limit 12
                --coverage-floor 0.01 --write-diff <diff.ppm>
```

## Task 1: Register a profile-aware parity renderer

**Files:**
- Modify: `CMakeLists.txt:1036-1038`
- Create: `tools/glparity/CMakeLists.txt`
- Create: `tools/glparity/glparity_render.cpp`

- [ ] **Step 1: Write the failing CMake acceptance check**

Configure a fresh GL3 examples build and try to build the missing target:

```bash
cmake -S . -B build-glparity-gl3 \
  -DCOIN_GL_PROFILE=GL3 -DCOIN_BUILD_EXAMPLES=ON -DCOIN_BUILD_TESTS=OFF
cmake --build build-glparity-gl3 --target glparity-render
```

Expected: target-not-found failure. This proves the check targets the new integration rather than an existing example.

- [ ] **Step 2: Add the target wiring**

Inside the existing `if (COIN_BUILD_EXAMPLES)` block in root `CMakeLists.txt`, add:

```cmake
add_subdirectory(tools/glparity)
```

Create `tools/glparity/CMakeLists.txt` modeled after `examples/sdl3/CMakeLists.txt`:

```cmake
find_package(SDL3 REQUIRED CONFIG)
add_executable(glparity-render glparity_render.cpp)
target_include_directories(glparity-render PRIVATE
  ${PROJECT_SOURCE_DIR}/include
  ${PROJECT_SOURCE_DIR}/include/Inventor/annex
  ${PROJECT_BINARY_DIR}/include
  ${COIN_TARGET_INCLUDE_DIRECTORIES})
target_link_libraries(glparity-render PRIVATE Coin ${COIN_TARGET_LINK_LIBRARIES_GL})
# Link SDL3::SDL3 or SDL3::SDL3-shared with the same fallback pattern as sdl3-example.
```

Copy only the profile compile definitions from `examples/sdl3/CMakeLists.txt:19-31`: `COIN_GL_MODERN`, and exactly one of `COIN_GL3_CORE`, `COIN_GLES3`, `COIN_GLES2`.

- [ ] **Step 3: Add a minimal renderer CLI that fails clearly**

Implement `main()` with `--scene` / `--output` argument parsing and a `usage()` helper. Until rendering exists, validate both required options and return `2` on bad arguments. Do not add output files yet.

- [ ] **Step 4: Verify the target now configures and runs**

```bash
cmake --build build-glparity-gl3 --target glparity-render -j"$(sysctl -n hw.ncpu)"
./build-glparity-gl3/bin/glparity-render
```

Expected: build succeeds; executable prints usage and exits `2`.

- [ ] **Step 5: Commit**

```bash
git add CMakeLists.txt tools/glparity/CMakeLists.txt tools/glparity/glparity_render.cpp
git commit -m "add glparity renderer target"
```

## Task 2: Render a deterministic scene and emit raw frame statistics

**Files:**
- Modify: `tools/glparity/glparity_render.cpp`
- Create: `tools/glparity/scenes_p0.txt`

- [ ] **Step 1: Add a failing manual renderer behavior check**

Build COMPAT and GL3 targets, then run the not-yet-implemented renderer:

```bash
cmake -S . -B build-glparity-compat \
  -DCOIN_GL_PROFILE=COMPAT -DCOIN_BUILD_EXAMPLES=ON -DCOIN_BUILD_TESTS=OFF
cmake --build build-glparity-compat --target glparity-render -j4
rm -rf /tmp/glparity-red
./build-glparity-compat/bin/glparity-render \
  --scene models/dead_simple/cube.iv --output /tmp/glparity-red
test -f /tmp/glparity-red/frame0000.rgba
```

Expected: `test` fails because output support has not been implemented.

- [ ] **Step 2: Create the P0 manifest**

Create `tools/glparity/scenes_p0.txt` with one repository-relative path per line:

```text
# Small deterministic scenes for COMPAT ↔ GL3 parity.
models/dead_simple/cube.iv
models/dead_simple/rotated_cube.iv
models/dead_simple/sphere.iv
models/dead_simple/cone.iv
models/dead_simple/material.iv
models/dead_simple/texture2.iv
models/dead_simple/text2.iv
models/dead_simple/indexedfaceset.iv
models/oiv_compliance/directionallight.iv
models/oiv_compliance/clipplane.iv
```

- [ ] **Step 3: Implement profile-correct SDL context setup**

Use the proven `examples/sdl3/sdl3.cpp` patterns, including the COMPAT fallback:

```cpp
#if defined(COIN_GLES3)
SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#elif defined(COIN_GLES2)
SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#elif defined(COIN_GL3_CORE)
SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
#else
SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
#endif
```

Always set double buffering and 24-bit depth. Create `SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN`, explicitly request zero multisample buffers/samples, make the context current, drain initial GL errors, and enable `GL_DEPTH_TEST`.

- [ ] **Step 4: Implement deterministic scene construction**

1. Call `SoDB::init()`.
2. Read the requested scene using `SoInput` plus `SoDB::readAll`; abort with a diagnostic on failure.
3. Create a harness `SoSeparator`. Prefer the same child order as `examples/sdl3/sdl3.cpp`: optional/inserted camera, then the default `SoDirectionalLight`, then the loaded scene root.
4. Create `SoSceneManager`, call `setBackgroundColor(SbColor(0.12f, 0.14f, 0.18f))`, call `setWindowSize(SbVec2s(width, height))` **before** any `viewAll`, activate, and `setSceneGraph(harness)`.
5. Detect an existing camera with `SoSearchAction` configured for `SoCamera`. If none exists, insert `SoPerspectiveCamera` early in the harness, then call:

```cpp
camera->viewAll(harness, scene_manager->getViewportRegion());
const float dist = fabsf(camera->position.getValue()[2]);
camera->nearDistance = dist * 0.1f;
camera->farDistance = dist * 10.0f;
```

Use the manager’s 400×300 viewport region — never a default `SbViewportRegion()`.
6. Render exactly once with `scene_manager->render()`.

Keep the harness root `ref()`/`unref()` balanced before destroying the GL context.

- [ ] **Step 5: Implement framebuffer capture and outputs**

After `scene_manager->render()`, call `glFinish()` and `glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, buffer.data())`.

Write `frame0000.rgba` in binary mode after creating the output directory with `mkdir`-style creation (the verification commands delete then recreate via the tool). Count coverage using:

```cpp
const unsigned char clear[3] = {31, 36, 46};
const bool nonbackground =
  abs(int(r) - clear[0]) > 8 ||
  abs(int(g) - clear[1]) > 8 ||
  abs(int(b) - clear[2]) > 8;
```

Write `stats.json` with exactly the fields in the design: width, height, `non_bg_pixels`, `non_bg_ratio`, center RGBA, GL version, profile, and `bg_rgb`. Avoid a JSON dependency; escape GL version text or emit it as a JSON string using a small local escaping helper.

Return nonzero if coverage is zero unless `--allow-empty` is supplied.

- [ ] **Step 6: Verify both profile renderers produce valid capture artifacts**

Rebuild both profiles after the renderer implementation lands, then capture:

```bash
cmake -S . -B build-glparity-compat \
  -DCOIN_GL_PROFILE=COMPAT -DCOIN_BUILD_EXAMPLES=ON -DCOIN_BUILD_TESTS=OFF
cmake -S . -B build-glparity-gl3 \
  -DCOIN_GL_PROFILE=GL3 -DCOIN_BUILD_EXAMPLES=ON -DCOIN_BUILD_TESTS=OFF
cmake --build build-glparity-compat --target glparity-render -j4
cmake --build build-glparity-gl3 --target glparity-render -j4
rm -rf /tmp/glparity-compat /tmp/glparity-gl3
./build-glparity-compat/bin/glparity-render \
  --scene "$PWD/models/dead_simple/cube.iv" --output /tmp/glparity-compat
./build-glparity-gl3/bin/glparity-render \
  --scene "$PWD/models/dead_simple/cube.iv" --output /tmp/glparity-gl3
python3 -c 'import json; print(json.load(open("/tmp/glparity-compat/stats.json"))["non_bg_ratio"])'
test -s /tmp/glparity-compat/frame0000.rgba
test -s /tmp/glparity-gl3/frame0000.rgba
```

Expected: both raw files are exactly `400 * 300 * 4` bytes and both stats report nonzero coverage.

On modern macOS, the COMPAT binary should request a legacy compatibility context (typically OpenGL 2.1). If local COMPAT context creation fails on a given machine, treat Ubuntu/Xvfb CI as the authoritative COMPAT↔GL3 gate and still keep the GL3 capture + Python unit tests green locally.

- [ ] **Step 7: Commit**

```bash
git add tools/glparity/glparity_render.cpp tools/glparity/scenes_p0.txt
git commit -m "render deterministic glparity frames"
```

## Task 3: Implement and test raw RGBA comparison

**Files:**
- Create: `tools/glparity/compare_rgba.py`
- Create: `tools/glparity/tests/test_compare_rgba.py`
- Create: `tools/glparity/thresholds.json`

- [ ] **Step 1: Write failing Python tests**

Use `unittest` and `tempfile.TemporaryDirectory`. Add three tests:

```python
def test_identical_non_background_frames_pass(self): ...
def test_empty_candidate_frame_fails_coverage_floor(self): ...
def test_large_pixel_difference_fails_and_writes_ppm(self): ...
```

Synthetic frames must be `2x2` RGBA buffers using background `(31, 36, 46, 255)` and an intentionally colored pixel. Import the module with `importlib.util.spec_from_file_location` so no packaging changes are needed.

- [ ] **Step 2: Verify RED**

```bash
python3 -m unittest tools/glparity/tests/test_compare_rgba.py -v
```

Expected: fail because `compare_rgba.py` does not exist.

- [ ] **Step 3: Implement comparison primitives**

Implement:

```python
def read_rgba(path: pathlib.Path, width: int, height: int) -> bytes: ...
def is_non_background(r: int, g: int, b: int) -> bool: ...
def compare(reference: bytes, candidate: bytes, width: int, height: int) -> dict: ...
def write_ppm_diff(path: pathlib.Path, reference: bytes, candidate: bytes,
                   width: int, height: int) -> None: ...
```

`read_rgba()` must reject a file whose size differs from `width * height * 4`. `compare()` computes coverage on RGB only, RMSE over RGB only (`sqrt(sum(delta²)/(pixel_count*3))/255*100`), and maximum absolute RGB delta. PPM output must use P6 and reverse the input row order so normal image viewers show top-up output.

CLI arguments must validate `--coverage-floor` and `--rmse-limit`, write a concise one-line metric report, and return `1` for coverage/RMSE failure, `2` for malformed input/usage, and `0` for pass. Coverage floor must fail if **either** reference or candidate is below the floor. When `--write-diff <path>` is supplied, always write the PPM (pass or fail) so the suite artifact layout is stable.

- [ ] **Step 4: Add threshold configuration**

Create:

```json
{
  "default_rmse_percent": 12.0,
  "scenes": {
    "text2": { "rmse_percent": 20.0 }
  }
}
```

Keys are **basenames without `.iv`** (for example `text2`, not `models/dead_simple/text2.iv`). The runner, not the Python comparator, will read it.

- [ ] **Step 5: Verify GREEN**

```bash
python3 -m unittest tools/glparity/tests/test_compare_rgba.py -v
```

Expected: all three tests pass.

- [ ] **Step 6: Commit**

```bash
git add tools/glparity/compare_rgba.py tools/glparity/tests/test_compare_rgba.py \
  tools/glparity/thresholds.json
git commit -m "add raw framebuffer parity comparator"
```

## Task 4: Add a reproducible COMPAT↔GL3 suite runner

**Files:**
- Create: `tools/glparity/run_parity.sh`
- Create: `tools/glparity/README.md`

- [ ] **Step 1: Write a failing runner invocation**

```bash
tools/glparity/run_parity.sh \
  --compat build-glparity-compat/bin/glparity-render \
  --gl3 build-glparity-gl3/bin/glparity-render \
  --scenes tools/glparity/scenes_p0.txt \
  --output /tmp/glparity-suite
```

Expected: command-not-found or usage failure because the runner does not exist.

- [ ] **Step 2: Implement shell argument handling with continue-on-failure**

Use `#!/usr/bin/env bash` and `set -uo pipefail` (do **not** enable global `set -e`). Keep a `status=0` accumulator:

```bash
run_or_mark() {
  "$@" || status=1
}
```

Parse exactly `--compat`, `--gl3`, `--scenes`, `--output`, and optional `--width`, `--height`. Validate executability/readability; resolve the repository root from the script directory; set:

```bash
THRESHOLDS="$ROOT/tools/glparity/thresholds.json"
COMPARE="$ROOT/tools/glparity/compare_rgba.py"
```

Create the output root. Threshold lookup must use Python only, for example:

```bash
python3 - "$THRESHOLDS" "$scene_key" <<'PY'
import json, sys
cfg = json.load(open(sys.argv[1]))
key = sys.argv[2]
print(cfg.get("scenes", {}).get(key, {}).get("rmse_percent", cfg["default_rmse_percent"]))
PY
```

Do not introduce a `jq` dependency.

- [ ] **Step 3: Execute every non-comment manifest line**

For each scene path:

1. Compute `scene_key` as the basename without `.iv` (for example `text2`).
2. Create the exact artifact layout from the design:
   - `$OUTPUT/compat/$scene_key/{frame0000.rgba,stats.json}`
   - `$OUTPUT/gl3/$scene_key/{frame0000.rgba,stats.json}`
   - `$OUTPUT/diff/$scene_key/diff.ppm`
3. Invoke each renderer with absolute `--scene` and the matching profile output directory, also via `run_or_mark`.
4. Load the per-scene threshold by looking up `scenes[scene_key].rmse_percent` in `thresholds.json`, falling back to `default_rmse_percent`.
5. Run `compare_rgba.py --write-diff ...` via `run_or_mark` so a comparison failure does not abort the remaining scenes.
6. Print a final pass/fail count and `exit "$status"`.

Use `400x300` defaults and coverage floor `0.01`.

Local platform note: the authoritative full COMPAT↔GL3 suite gate is Ubuntu/Xvfb CI. On macOS, still run the Python unit tests and GL3 capture; run the full dual-profile suite when the local COMPAT context succeeds.

- [ ] **Step 4: Write operator documentation**

`README.md` must cover:

- requirements: CMake, SDL3, glfw3 (because `COIN_BUILD_EXAMPLES=ON` also configures `examples/glfw`), Python 3, and an available OpenGL/Xvfb display;
- building the two profile binaries;
- running the suite;
- artifact tree layout;
- exact `.rgba` pixel format and bottom-row-first convention;
- metrics, thresholds, and what to inspect in `diff.ppm`;
- statement that GLES3 smoke is deliberately deferred to P1;
- note that `text2.iv` requests Arial and Coin may fall back to another available font on Linux; the raised RMSE threshold exists for that class of difference.

- [ ] **Step 5: Verify suite behavior**

```bash
rm -rf /tmp/glparity-suite
tools/glparity/run_parity.sh \
  --compat "$PWD/build-glparity-compat/bin/glparity-render" \
  --gl3 "$PWD/build-glparity-gl3/bin/glparity-render" \
  --scenes tools/glparity/scenes_p0.txt \
  --output /tmp/glparity-suite
```

Expected: all P0 scenes pass, or any known intentional per-scene discrepancy is reflected only through a reviewed `thresholds.json` override. Confirm each scene directory has both stats files and a `diff.ppm`.

- [ ] **Step 6: Commit**

```bash
git add tools/glparity/run_parity.sh tools/glparity/README.md
git commit -m "add gl3 compatibility parity runner"
```

## Task 5: Gate COMPAT↔GL3 parity in Ubuntu CI and document it

**Files:**
- Modify: `.github/workflows/continuous-integration-workflow.yml:11-51`
- Modify: `docs/COIN_GL_PROFILE.md`

- [ ] **Step 1: Write the failing CI-equivalent command locally**

Run the planned commands manually before editing CI:

```bash
xvfb-run -a tools/glparity/run_parity.sh \
  --compat "$PWD/build-glparity-compat/bin/glparity-render" \
  --gl3 "$PWD/build-glparity-gl3/bin/glparity-render" \
  --scenes tools/glparity/scenes_p0.txt \
  --output /tmp/glparity-ci
```

Expected before CI dependency work: it either succeeds on Linux with Xvfb installed, or explains the missing `xvfb-run`. Do not silently replace it with a visible SDL window.

- [ ] **Step 2: Add a dedicated Ubuntu parity job**

Create `ubuntu-glparity` rather than coupling the normal unit-test job to GPU requirements:

1. Install system packages exactly (SDL3 source build + Xvfb/OpenGL + fonts):

```text
freeglut3-dev libboost-dev libglfw3-dev xvfb mesa-utils fonts-dejavu-core pkg-config
libx11-dev libxext-dev libxrandr-dev libxcursor-dev libxfixes-dev libxi-dev
libxinerama-dev libxss-dev libgl1-mesa-dev libgl1-mesa-dri
```

   Install `libglfw3-dev` because `COIN_BUILD_EXAMPLES=ON` also configures `examples/glfw`, which does `find_package(glfw3 3.3 REQUIRED)`.
   Do **not** rely on `libsdl3-dev`; Ubuntu 24.04 (`ubuntu-latest`) does not ship it.
2. Build and install SDL3 from the official release into `$HOME/sdl3-prefix` before configuring Coin:

```bash
curl -L https://github.com/libsdl-org/SDL/releases/download/release-3.2.10/SDL3-3.2.10.tar.gz | tar xz
cmake -S SDL3-3.2.10 -B sdl3-build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$HOME/sdl3-prefix"
cmake --build sdl3-build -j4
cmake --install sdl3-build
export CMAKE_PREFIX_PATH="$HOME/sdl3-prefix${CMAKE_PREFIX_PATH:+:$CMAKE_PREFIX_PATH}"
```

3. Configure/build `cmake_build_compat` with `COMPAT`, examples on, tests off; build `glparity-render`.
4. Configure/build `cmake_build_gl3` with `GL3`, examples on, tests off; build `glparity-render`.
5. Execute:

```bash
xvfb-run -a tools/glparity/run_parity.sh \
  --compat "$PWD/cmake_build_compat/bin/glparity-render" \
  --gl3 "$PWD/cmake_build_gl3/bin/glparity-render" \
  --scenes tools/glparity/scenes_p0.txt \
  --output glparity-out
```

6. Upload `glparity-out/` only when failure occurs:

```yaml
if: failure()
```

Do not modify the existing GLES3/GLES2 compile-only job for phase 1. Ubuntu/Xvfb is the authoritative COMPAT↔GL3 gate.

- [ ] **Step 3: Document the coverage contract**

Append a short `## GL parity regression` section to `docs/COIN_GL_PROFILE.md`:

```markdown
The Ubuntu CI parity job renders `tools/glparity/scenes_p0.txt` with both
`COMPAT` and `GL3`. It rejects empty frames and compares RGB RMSE against
per-scene limits. It intentionally does not provide pixel-perfect parity or
GLES3 rendering coverage; GLES3 smoke is planned separately.
```

Link to `tools/glparity/README.md`.

- [ ] **Step 4: Verify configuration and focused tests**

```bash
python3 -m unittest tools/glparity/tests/test_compare_rgba.py -v
cmake -S . -B build-glparity-compat \
  -DCOIN_GL_PROFILE=COMPAT -DCOIN_BUILD_EXAMPLES=ON -DCOIN_BUILD_TESTS=OFF
cmake -S . -B build-glparity-gl3 \
  -DCOIN_GL_PROFILE=GL3 -DCOIN_BUILD_EXAMPLES=ON -DCOIN_BUILD_TESTS=OFF
cmake --build build-glparity-compat --target glparity-render -j4
cmake --build build-glparity-gl3 --target glparity-render -j4
tools/glparity/run_parity.sh \
  --compat "$PWD/build-glparity-compat/bin/glparity-render" \
  --gl3 "$PWD/build-glparity-gl3/bin/glparity-render" \
  --scenes tools/glparity/scenes_p0.txt \
  --output /tmp/glparity-final
```

Expected: Python tests and both renderer builds pass; the local parity suite succeeds on a machine with a valid desktop GL context.

- [ ] **Step 5: Commit**

```bash
git add .github/workflows/continuous-integration-workflow.yml \
  docs/COIN_GL_PROFILE.md
git commit -m "gate gl3 rendering parity in ci"
```

## Final verification

- [ ] Run `git diff --check`.
- [ ] Run the comparator unit tests.
- [ ] Run the full local P0 suite with both profile binaries.
- [ ] Check all failure diagnostics are actionable: malformed RGBA names a file, empty frame identifies profile/scene, and comparison failure names its diff artifact.
- [ ] Verify `git status --short`; do not commit pre-existing `docs/superpowers/specs/*` or unrelated user changes without explicit approval.
