# -*- coding: utf-8 -*-
"""Map AWTK NanoVG / GPU backend to Coin COIN_GL_PROFILE."""

import os

COIN_PROFILE_GL3 = 'GL3'
COIN_PROFILE_GLES3 = 'GLES3'
COIN_PROFILE_GLES2 = 'GLES2'

_GLES3_TOKENS = (
    'GLES3',
    'WITH_NANOVG_GLES3',
    'WITH_GPU_GLES3',
    'NVGP_GLES3',
    'NANOVG_GLES3',
)

_GLES2_TOKENS = (
    'GLES2',
    'WITH_NANOVG_GLES2',
    'WITH_GPU_GLES2',
    'NVGP_GLES2',
    'NANOVG_GLES2',
)

_GL2_TOKENS = (
    'GL2',
    'WITH_NANOVG_GL2',
    'WITH_GPU_GL2',
)

_SOFT_TOKENS = (
    'AGGE',
    'CAIRO',
    'WITH_NANOVG_AGGE',
    'WITH_NANOVG_SOFT',
    'WITH_VGCANVAS_CAIRO',
)


class CoinGlProfileError(Exception):
    """AWTK backend has no supported Coin GL profile."""


def _normalize_token(value):
    if value is None:
        return ''
    text = str(value).strip()
    if text.startswith('-D') or text.startswith('/D'):
        text = text[2:]
    text = text.upper()
    if '=' in text:
        text = text.split('=', 1)[0]
    return text


def _tokens_from_ccflags(ccflags):
    if not ccflags:
        return []
    if isinstance(ccflags, (list, tuple)):
        parts = ccflags
    else:
        parts = str(ccflags).split()
    return [_normalize_token(part) for part in parts if _normalize_token(part)]


def _unsupported(name):
    raise CoinGlProfileError(
        'AWTK backend %s has no usable Coin GL profile. '
        'Use NANOVG_BACKEND=GL3, GLES3, or GLES2.' % name)


def _has_any(tokens, names):
    token_set = set(tokens)
    return any(name in token_set for name in names)


def detect_awtk_inputs(helper=None, environ=None):
    """Read NANOVG_BACKEND and compiler flags from an AWTK app helper."""
    if environ is None:
        environ = os.environ

    backend = None
    awtk = getattr(helper, 'awtk', None) if helper is not None else None
    if awtk is not None:
        backend = getattr(awtk, 'NANOVG_BACKEND', None)
    if not backend and helper is not None:
        backend = getattr(helper, 'NANOVG_BACKEND', None)
    if not backend:
        backend = environ.get('NANOVG_BACKEND')

    parts = []
    if helper is not None:
        for attr in ('AWTK_CCFLAGS', 'APP_CCFLAGS'):
            value = getattr(helper, attr, None)
            if value:
                parts.append(str(value))
    if awtk is not None:
        for attr in ('CCFLAGS', 'AWTK_CCFLAGS', 'COMMON_CCFLAGS'):
            value = getattr(awtk, attr, None)
            if value:
                parts.append(str(value))
    return backend, ' '.join(parts)


def resolve_from_awtk(helper=None, environ=None):
    backend, ccflags = detect_awtk_inputs(helper, environ)
    return resolve_coin_gl_profile(backend, ccflags)


def resolve_coin_gl_profile(backend=None, ccflags=None):
    """Return (COIN_GL_PROFILE, cmake-build-dir-name) for the AWTK GPU backend.

    ``backend`` is AWTK ``NANOVG_BACKEND`` (GL3 / GLES3 / GLES2 / GL2 / AGGE).
    ``ccflags`` may include WITH_NANOVG_*, WITH_GPU_*, NVGP_*, or NANOVG_*.
    An explicit backend wins over compiler flags. Empty input defaults to GL3.
    """
    name = _normalize_token(backend)
    flags = _tokens_from_ccflags(ccflags)

    if name in _SOFT_TOKENS or (not name and _has_any(flags, _SOFT_TOKENS)):
        _unsupported(name or 'AGGE/CAIRO')
    if name in _GL2_TOKENS or (not name and _has_any(flags, _GL2_TOKENS)):
        _unsupported('GL2')

    if name in _GLES2_TOKENS or (not name and _has_any(flags, _GLES2_TOKENS)):
        return (COIN_PROFILE_GLES2, 'build-gles2')
    if name in _GLES3_TOKENS or (not name and _has_any(flags, _GLES3_TOKENS)):
        return (COIN_PROFILE_GLES3, 'build-gles3')

    return (COIN_PROFILE_GL3, 'build-gl3')
