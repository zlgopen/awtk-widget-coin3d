#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import unittest

from coin_gl_profile import (
    CoinGlProfileError,
    resolve_coin_gl_profile,
    resolve_from_awtk,
)


class TestResolveCoinGlProfile(unittest.TestCase):
    def test_backend_gl3(self):
        self.assertEqual(resolve_coin_gl_profile('GL3'), ('GL3', 'build-gl3'))

    def test_backend_gles3(self):
        self.assertEqual(resolve_coin_gl_profile('GLES3'), ('GLES3', 'build-gles3'))

    def test_backend_is_case_insensitive(self):
        self.assertEqual(resolve_coin_gl_profile('gles3'), ('GLES3', 'build-gles3'))

    def test_empty_defaults_to_gl3(self):
        self.assertEqual(resolve_coin_gl_profile(None), ('GL3', 'build-gl3'))
        self.assertEqual(resolve_coin_gl_profile(''), ('GL3', 'build-gl3'))

    def test_ccflags_with_nanovg_gles3(self):
        flags = '-DWITH_NANOVG_GLES3 -DWITH_NANOVG_GL -DWITH_NANOVG_GPU'
        self.assertEqual(resolve_coin_gl_profile(None, flags), ('GLES3', 'build-gles3'))

    def test_ccflags_with_gpu_gles3(self):
        self.assertEqual(
            resolve_coin_gl_profile('', '-DWITH_GPU_GLES3'),
            ('GLES3', 'build-gles3'))

    def test_ccflags_nvgp_gles3(self):
        self.assertEqual(
            resolve_coin_gl_profile(None, '-DWITH_NANOVG_PLUS_GPU -DNVGP_GLES3'),
            ('GLES3', 'build-gles3'))

    def test_ccflags_nanovg_gles3_macro(self):
        self.assertEqual(
            resolve_coin_gl_profile(None, '-DNANOVG_GLES3'),
            ('GLES3', 'build-gles3'))

    def test_msvc_slash_d_gles3(self):
        self.assertEqual(
            resolve_coin_gl_profile(None, '/DWITH_NANOVG_GLES3 /DWITH_NANOVG_GPU'),
            ('GLES3', 'build-gles3'))

    def test_ccflags_gl3(self):
        flags = '-DWITH_NANOVG_GL3 -DWITH_NANOVG_GL -DWITH_NANOVG_GPU'
        self.assertEqual(resolve_coin_gl_profile(None, flags), ('GL3', 'build-gl3'))

    def test_explicit_backend_wins_over_ccflags(self):
        self.assertEqual(
            resolve_coin_gl_profile('GLES3', '-DWITH_NANOVG_GL3'),
            ('GLES3', 'build-gles3'))

    def test_backend_gles2(self):
        self.assertEqual(resolve_coin_gl_profile('GLES2'), ('GLES2', 'build-gles2'))

    def test_ccflags_gles2(self):
        self.assertEqual(
            resolve_coin_gl_profile(None, '-DWITH_NANOVG_GLES2 -DWITH_GPU_GLES2'),
            ('GLES2', 'build-gles2'))

    def test_resolve_from_awtk_gles2(self):
        class Awtk(object):
            NANOVG_BACKEND = 'GLES2'
            CCFLAGS = '-DWITH_NANOVG_GLES2'

        class Helper(object):
            awtk = Awtk()

        self.assertEqual(resolve_from_awtk(Helper(), {}), ('GLES2', 'build-gles2'))

    def test_gl2_is_unsupported(self):
        with self.assertRaises(CoinGlProfileError):
            resolve_coin_gl_profile('GL2')

    def test_agge_is_unsupported(self):
        with self.assertRaises(CoinGlProfileError):
            resolve_coin_gl_profile('AGGE')

    def test_resolve_from_awtk_backend(self):
        class Awtk(object):
            NANOVG_BACKEND = 'GLES3'
            CCFLAGS = ''

        class Helper(object):
            awtk = Awtk()

        self.assertEqual(resolve_from_awtk(Helper(), {}), ('GLES3', 'build-gles3'))

    def test_resolve_from_awtk_ccflags(self):
        class Awtk(object):
            NANOVG_BACKEND = ''
            CCFLAGS = '-DWITH_NANOVG_GLES3 -DWITH_NANOVG_GPU'

        class Helper(object):
            awtk = Awtk()
            AWTK_CCFLAGS = ''

        self.assertEqual(resolve_from_awtk(Helper(), {}), ('GLES3', 'build-gles3'))

    def test_resolve_from_environ_backend(self):
        self.assertEqual(
            resolve_from_awtk(None, {'NANOVG_BACKEND': 'GLES3'}),
            ('GLES3', 'build-gles3'))


if __name__ == '__main__':
    unittest.main()
