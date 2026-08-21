import importlib.util
import pathlib
import tempfile
import unittest


MODULE_PATH = pathlib.Path(__file__).parents[1] / "compare_rgba.py"
SPEC = importlib.util.spec_from_file_location("compare_rgba", MODULE_PATH)
compare_rgba = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(compare_rgba)

BACKGROUND = (31, 36, 46, 255)


def frame(colored_pixel):
    pixels = [BACKGROUND] * 4
    pixels[0] = colored_pixel
    return bytes(channel for pixel in pixels for channel in pixel)


class CompareRgbaTest(unittest.TestCase):
    def test_identical_non_background_frames_pass(self):
        with tempfile.TemporaryDirectory() as directory:
            root = pathlib.Path(directory)
            reference = root / "reference.rgba"
            candidate = root / "candidate.rgba"
            pixels = frame((255, 36, 46, 255))
            reference.write_bytes(pixels)
            candidate.write_bytes(pixels)

            status = compare_rgba.main([
                "--reference", str(reference),
                "--candidate", str(candidate),
                "--width", "2",
                "--height", "2",
                "--rmse-limit", "0",
                "--coverage-floor", "0.25",
            ])

            self.assertEqual(status, 0)

    def test_empty_candidate_frame_fails_coverage_floor(self):
        with tempfile.TemporaryDirectory() as directory:
            root = pathlib.Path(directory)
            reference = root / "reference.rgba"
            candidate = root / "candidate.rgba"
            reference.write_bytes(frame((255, 36, 46, 255)))
            candidate.write_bytes(bytes(BACKGROUND * 4))

            status = compare_rgba.main([
                "--reference", str(reference),
                "--candidate", str(candidate),
                "--width", "2",
                "--height", "2",
                "--rmse-limit", "100",
                "--coverage-floor", "0.25",
            ])

            self.assertEqual(status, 1)

    def test_large_pixel_difference_fails_and_writes_ppm(self):
        with tempfile.TemporaryDirectory() as directory:
            root = pathlib.Path(directory)
            reference = root / "reference.rgba"
            candidate = root / "candidate.rgba"
            diff = root / "diff.ppm"
            reference_pixels = frame((255, 36, 46, 255))
            candidate_pixels = frame((31, 255, 46, 255))
            reference.write_bytes(reference_pixels)
            candidate.write_bytes(candidate_pixels)

            status = compare_rgba.main([
                "--reference", str(reference),
                "--candidate", str(candidate),
                "--width", "2",
                "--height", "2",
                "--rmse-limit", "1",
                "--coverage-floor", "0.25",
                "--write-diff", str(diff),
            ])
            metrics = compare_rgba.compare(
                reference_pixels, candidate_pixels, width=2, height=2
            )

            self.assertEqual(status, 1)
            self.assertAlmostEqual(metrics["rmse_percent"], 35.463826, places=5)
            self.assertEqual(metrics["max_delta"], 224)
            self.assertEqual(diff.read_bytes(), (
                b"P6\n2 2\n255\n"
                + bytes((0, 0, 0) * 2)
                + bytes((224, 219, 0))
                + bytes((0, 0, 0))
            ))


if __name__ == "__main__":
    unittest.main()
