#!/usr/bin/env bash
set -uo pipefail

usage() {
  cat >&2 <<'EOF'
Usage: run_parity.sh --compat <renderer> --gl3 <renderer> \
  --scenes <manifest> --output <directory> [--width <pixels>] [--height <pixels>]
EOF
}

status=0

run_or_mark() {
  "$@" || status=1
}

run_for_scene() {
  "$@" || {
    scene_status=1
    return 1
  }
}

compat=""
gl3=""
scenes=""
output=""
width=400
height=300

while (($#)); do
  case "$1" in
    --compat|--gl3|--scenes|--output|--width|--height)
      if (($# < 2)); then
        echo "error: $1 requires a value" >&2
        usage
        exit 2
      fi
      case "$1" in
        --compat) compat="$2" ;;
        --gl3) gl3="$2" ;;
        --scenes) scenes="$2" ;;
        --output) output="$2" ;;
        --width) width="$2" ;;
        --height) height="$2" ;;
      esac
      shift 2
      ;;
    *)
      echo "error: unknown argument: $1" >&2
      usage
      exit 2
      ;;
  esac
done

if [[ -z "$compat" || -z "$gl3" || -z "$scenes" || -z "$output" ]]; then
  echo "error: --compat, --gl3, --scenes, and --output are required" >&2
  usage
  exit 2
fi

if [[ ! "$width" =~ ^[1-9][0-9]*$ || ! "$height" =~ ^[1-9][0-9]*$ ]]; then
  echo "error: --width and --height must be positive integers" >&2
  exit 2
fi

for renderer in "$compat" "$gl3"; do
  if [[ ! -x "$renderer" ]]; then
    echo "error: renderer is not executable: $renderer" >&2
    exit 2
  fi
done

if [[ ! -r "$scenes" ]]; then
  echo "error: scene manifest is not readable: $scenes" >&2
  exit 2
fi

ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
THRESHOLDS="$ROOT/tools/glparity/thresholds.json"
COMPARE="$ROOT/tools/glparity/compare_rgba.py"

if [[ ! -r "$THRESHOLDS" ]]; then
  echo "error: thresholds file is not readable: $THRESHOLDS" >&2
  exit 2
fi
if [[ ! -r "$COMPARE" ]]; then
  echo "error: comparator is not readable: $COMPARE" >&2
  exit 2
fi
if ! command -v python3 >/dev/null 2>&1; then
  echo "error: python3 is required" >&2
  exit 2
fi

run_or_mark mkdir -p "$output"

pass_count=0
fail_count=0
scene_count=0

while IFS= read -r scene_path || [[ -n "$scene_path" ]]; do
  scene_path="${scene_path%$'\r'}"
  [[ -z "${scene_path//[[:space:]]/}" ]] && continue
  [[ "$scene_path" =~ ^[[:space:]]*# ]] && continue

  scene_count=$((scene_count + 1))
  scene_key="$(basename "$scene_path")"
  scene_key="${scene_key%.iv}"
  if [[ "$scene_path" = /* ]]; then
    absolute_scene="$scene_path"
  else
    absolute_scene="$ROOT/$scene_path"
  fi

  compat_output="$output/compat/$scene_key"
  gl3_output="$output/gl3/$scene_key"
  diff_output="$output/diff/$scene_key"
  scene_status=0

  echo "scene: $scene_key"
  run_or_mark run_for_scene mkdir -p "$compat_output" "$gl3_output" "$diff_output"
  run_or_mark run_for_scene "$compat" \
    --scene "$absolute_scene" --output "$compat_output" \
    --width "$width" --height "$height"
  run_or_mark run_for_scene "$gl3" \
    --scene "$absolute_scene" --output "$gl3_output" \
    --width "$width" --height "$height"

  threshold=""
  if ! threshold="$(
    python3 - "$THRESHOLDS" "$scene_key" <<'PY'
import json, sys
cfg = json.load(open(sys.argv[1]))
key = sys.argv[2]
print(cfg.get("scenes", {}).get(key, {}).get("rmse_percent", cfg["default_rmse_percent"]))
PY
  )"; then
    echo "error: failed to load threshold for $scene_key" >&2
    status=1
    scene_status=1
  fi

  if [[ -n "$threshold" ]]; then
    run_or_mark run_for_scene python3 "$COMPARE" \
      --reference "$compat_output/frame0000.rgba" \
      --candidate "$gl3_output/frame0000.rgba" \
      --width "$width" --height "$height" \
      --rmse-limit "$threshold" --coverage-floor 0.01 \
      --write-diff "$diff_output/diff.ppm"
  fi

  if ((scene_status == 0)); then
    pass_count=$((pass_count + 1))
  else
    fail_count=$((fail_count + 1))
  fi
done < "$scenes"

echo "parity suite: $pass_count passed, $fail_count failed, $scene_count total"
exit "$status"
