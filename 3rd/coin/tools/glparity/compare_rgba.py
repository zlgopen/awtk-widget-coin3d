#!/usr/bin/env python3

import argparse
import math
import pathlib
import sys


BACKGROUND = (31, 36, 46)
BACKGROUND_TOLERANCE = 8


def read_rgba(path: pathlib.Path, width: int, height: int) -> bytes:
    data = path.read_bytes()
    expected_size = width * height * 4
    if len(data) != expected_size:
        raise ValueError(
            f"{path}: expected {expected_size} bytes, got {len(data)}"
        )
    return data


def is_non_background(r: int, g: int, b: int) -> bool:
    return any(
        abs(channel - background) > BACKGROUND_TOLERANCE
        for channel, background in zip((r, g, b), BACKGROUND)
    )


def compare(reference: bytes, candidate: bytes, width: int, height: int) -> dict:
    expected_size = width * height * 4
    if len(reference) != expected_size or len(candidate) != expected_size:
        raise ValueError("RGBA buffers do not match the requested dimensions")

    pixel_count = width * height
    reference_non_background = 0
    candidate_non_background = 0
    squared_delta_sum = 0
    max_delta = 0

    for offset in range(0, expected_size, 4):
        reference_rgb = reference[offset:offset + 3]
        candidate_rgb = candidate[offset:offset + 3]
        if is_non_background(*reference_rgb):
            reference_non_background += 1
        if is_non_background(*candidate_rgb):
            candidate_non_background += 1
        for reference_channel, candidate_channel in zip(
                reference_rgb, candidate_rgb):
            delta = abs(reference_channel - candidate_channel)
            squared_delta_sum += delta * delta
            max_delta = max(max_delta, delta)

    return {
        "reference_coverage": reference_non_background / pixel_count,
        "candidate_coverage": candidate_non_background / pixel_count,
        "rmse_percent": (
            math.sqrt(squared_delta_sum / (pixel_count * 3)) / 255 * 100
        ),
        "max_delta": max_delta,
    }


def write_ppm_diff(path: pathlib.Path, reference: bytes, candidate: bytes,
                   width: int, height: int) -> None:
    expected_size = width * height * 4
    if len(reference) != expected_size or len(candidate) != expected_size:
        raise ValueError("RGBA buffers do not match the requested dimensions")

    pixels = bytearray()
    row_size = width * 4
    for row in range(height - 1, -1, -1):
        row_offset = row * row_size
        for column in range(width):
            offset = row_offset + column * 4
            pixels.extend(
                abs(reference[offset + channel] - candidate[offset + channel])
                for channel in range(3)
            )

    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_bytes(f"P6\n{width} {height}\n255\n".encode("ascii") + pixels)


def _positive_integer(value: str) -> int:
    parsed = int(value)
    if parsed <= 0:
        raise argparse.ArgumentTypeError("must be greater than zero")
    return parsed


def _non_negative_float(value: str) -> float:
    parsed = float(value)
    if not math.isfinite(parsed) or parsed < 0:
        raise argparse.ArgumentTypeError("must be a finite non-negative number")
    return parsed


def _coverage(value: str) -> float:
    parsed = float(value)
    if not math.isfinite(parsed) or not 0 <= parsed <= 1:
        raise argparse.ArgumentTypeError("must be between 0 and 1")
    return parsed


def _parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        description="Compare two bottom-row-first raw RGBA framebuffers."
    )
    parser.add_argument("--reference", required=True, type=pathlib.Path)
    parser.add_argument("--candidate", required=True, type=pathlib.Path)
    parser.add_argument("--width", required=True, type=_positive_integer)
    parser.add_argument("--height", required=True, type=_positive_integer)
    parser.add_argument("--rmse-limit", required=True, type=_non_negative_float)
    parser.add_argument("--coverage-floor", required=True, type=_coverage)
    parser.add_argument("--write-diff", type=pathlib.Path)
    return parser


def main(argv=None) -> int:
    args = _parser().parse_args(argv)
    try:
        reference = read_rgba(args.reference, args.width, args.height)
        candidate = read_rgba(args.candidate, args.width, args.height)
        metrics = compare(reference, candidate, args.width, args.height)
        if args.write_diff is not None:
            write_ppm_diff(
                args.write_diff, reference, candidate, args.width, args.height
            )
    except (OSError, ValueError) as error:
        print(f"error: {error}", file=sys.stderr)
        return 2

    passed = (
        metrics["reference_coverage"] >= args.coverage_floor
        and metrics["candidate_coverage"] >= args.coverage_floor
        and metrics["rmse_percent"] <= args.rmse_limit
    )
    print(
        f"reference_coverage={metrics['reference_coverage']:.6f} "
        f"candidate_coverage={metrics['candidate_coverage']:.6f} "
        f"rmse_percent={metrics['rmse_percent']:.6f} "
        f"max_delta={metrics['max_delta']} "
        f"status={'PASS' if passed else 'FAIL'}"
    )
    return 0 if passed else 1


if __name__ == "__main__":
    sys.exit(main())
