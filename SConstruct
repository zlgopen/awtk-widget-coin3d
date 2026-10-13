import os
import platform
import subprocess
import shutil
import scripts.app_helper as app
import scripts.awtk_locator as awtk_locator
from scripts.coin_gl_profile import CoinGlProfileError, resolve_from_awtk

helper = app.Helper(ARGUMENTS)

APP_ROOT = helper.APP_ROOT
COIN_SRC = os.path.join(APP_ROOT, '3rd', 'coin')
try:
  COIN_GL_PROFILE, COIN_BUILD_NAME = resolve_from_awtk(helper)
except CoinGlProfileError as exc:
  raise Exception(str(exc))
COIN_BUILD = os.path.join(COIN_SRC, COIN_BUILD_NAME)
COIN_LIB_DIR = os.path.join(COIN_BUILD, 'lib')
COIN_BIN_DIR = os.path.join(COIN_BUILD, 'bin')
COIN_BUILD_SCRIPT = os.path.join(APP_ROOT, '3rd', 'build_coin.sh')
print('Coin GL profile: %s (AWTK NANOVG_BACKEND=%s) -> %s' % (
  COIN_GL_PROFILE,
  getattr(helper.awtk, 'NANOVG_BACKEND', os.environ.get('NANOVG_BACKEND', '')),
  COIN_BUILD_NAME))
_tk_root = os.environ.get('TK_ROOT') or os.environ.get('AWTK_ROOT') or awtk_locator.getAwtkRoot()
if not _tk_root or not os.path.isdir(_tk_root):
  _tk_root = os.path.abspath(os.path.join(APP_ROOT, '..', 'awtk'))
AWTK_GLAD_DIR = os.path.abspath(os.path.join(_tk_root, '3rd', 'glad'))
if not os.path.isfile(os.path.join(AWTK_GLAD_DIR, 'glad.c')):
  raise Exception('AWTK glad not found: %s' % AWTK_GLAD_DIR)
os.environ['AWTK_GLAD_DIR'] = AWTK_GLAD_DIR
print('AWTK glad: %s' % AWTK_GLAD_DIR)

# Keep Coin CRT in sync with AWTK/app (MSVC Debug=/MDd, Release=/MD).
COIN_IS_DEBUG = bool(getattr(helper, 'DEBUG', True))
COIN_CMAKE_CONFIG = 'Debug' if COIN_IS_DEBUG else 'Release'
_IS_MINGW = platform.system() == 'Windows' and getattr(helper.awtk, 'TOOLS_NAME', '') == 'mingw'
# Coin shared-library link names:
#   MSVC   Debug Coin4d.lib      Release Coin4.lib
#   MinGW  Debug libCoind.dll.a  Release libCoin.dll.a
#   Linux/macOS  libCoin.so / libCoin.dylib
if _IS_MINGW:
  COIN_LINK_CANDIDATES = ['Coind'] if COIN_IS_DEBUG else ['Coin']
elif platform.system() == 'Windows': # MSVC
  COIN_LINK_CANDIDATES = ['Coin4d'] if COIN_IS_DEBUG else ['Coin4']
else: # Linux / macOS
  COIN_LINK_CANDIDATES = ['Coin']

def find_coin_link_name():
  for name in COIN_LINK_CANDIDATES:
    for fname in (name + '.lib', 'lib' + name + '.dll.a', 'lib' + name + '.dylib', 'lib' + name + '.so', 'lib' + name + '.a'):
      if os.path.exists(os.path.join(COIN_LIB_DIR, fname)):
        return name
  if os.path.isdir(COIN_LIB_DIR):
    for fname in os.listdir(COIN_LIB_DIR):
      if fname.startswith('libCoin.') and ('.so' in fname or fname.endswith('.dylib') or fname.endswith('.a')):
        return 'Coin'
  return None

def coin_lib_exists():
  return find_coin_link_name() is not None

def msvc_cmake_generator():
  # SCons MSVC_VERSION (no Exp suffix) -> CMake -G. SCons 4.3 has no
  # msvc_default_version and no MSVC.Config; get_installed_vcs() is shared.
  _MSVC_CMAKE_GENERATOR = {
    '14.5': 'Visual Studio 18 2026',
    '14.3': 'Visual Studio 17 2022',
    '14.2': 'Visual Studio 16 2019',
    '14.1': 'Visual Studio 15 2017',
    '14.0': 'Visual Studio 14 2015',
  }
  # Newest installed VC in _VCVER order (vswhere/registry, cl.exe must exist).
  # Same list get_default_version() uses when MSVC_VERSION is unset.
  import SCons.Tool.MSCommon.vc as vc
  installed = vc.get_installed_vcs()
  if not installed:
    raise Exception('No MSVC compiler found (SCons get_installed_vcs)')
  msvc_version = installed[0]
  version_key = msvc_version[:-3] if msvc_version.endswith('Exp') else msvc_version
  generator = _MSVC_CMAKE_GENERATOR.get(version_key)
  if generator is None:
    raise Exception('No CMake generator for SCons MSVC version %s' % msvc_version)
  print('MSVC %s -> CMake -G "%s"' % (msvc_version, generator))
  return generator

def ensure_coin_built():
  print('Building Coin via CMake (%s, %s)...' % (COIN_GL_PROFILE, COIN_CMAKE_CONFIG))
  if platform.system() == 'Windows':
    glad_cmake = AWTK_GLAD_DIR.replace('\\', '/')
    cmake_args = [
      'cmake', '-S', COIN_SRC, '-B', COIN_BUILD,
    ]
    if _IS_MINGW:
      cmake_args.extend(['-G', 'MinGW Makefiles'])
    else:
      cmake_args.extend(['-G', msvc_cmake_generator()])
    cmake_args.extend([
      '-DCMAKE_BUILD_TYPE=' + COIN_CMAKE_CONFIG,
      '-DCOIN_GL_PROFILE=' + COIN_GL_PROFILE,
      '-DCOIN_BUILD_SHARED_LIBS=ON',
      '-DCOIN_BUILD_TESTS=OFF',
      '-DCOIN_BUILD_DOCUMENTATION=OFF',
      '-DCOIN_BUILD_EXAMPLES=OFF',
      '-DAWTK_GLAD_DIR=' + glad_cmake,
    ])
    subprocess.check_call(cmake_args)
    # Visual Studio is multi-config; CMAKE_BUILD_TYPE is ignored without --config.
    subprocess.check_call([
      'cmake', '--build', COIN_BUILD, '--target', 'Coin',
      '--config', COIN_CMAKE_CONFIG, '-j', '8'
    ])
  else:
    env = os.environ.copy()
    env['COIN_BUILD_TYPE'] = COIN_CMAKE_CONFIG
    env['COIN_GL_PROFILE'] = COIN_GL_PROFILE
    env['COIN_BUILD_DIR'] = COIN_BUILD
    subprocess.check_call(['bash', COIN_BUILD_SCRIPT], env=env)

ensure_coin_built()

COIN_LINK_NAME = find_coin_link_name()
if COIN_LINK_NAME is None:
  raise Exception(
    'Coin library not found in %s (expected %s.*, %s build). ' % (
      COIN_LIB_DIR, COIN_LINK_CANDIDATES[0], COIN_CMAKE_CONFIG))
print('Linking Coin as %s (app %s)' % (COIN_LINK_NAME, COIN_CMAKE_CONFIG))

APP_CPPPATH = [
  os.path.join(APP_ROOT, 'src'),
  os.path.join(COIN_SRC, 'include'),
  os.path.join(COIN_BUILD, 'include'),
  AWTK_GLAD_DIR,
]

APP_LIBPATH = [COIN_LIB_DIR]
APP_LIBS = ['coin3d', COIN_LINK_NAME]

APP_CXXFLAGS = '  '
if platform.system() == 'Windows' and helper.awtk.TOOLS_NAME != 'mingw':
  APP_CXXFLAGS += ' /std:c++17 /DGLAD_GLAPI_EXPORT '
else:
  APP_CXXFLAGS += ' -std=gnu++17 '
  if _IS_MINGW:
    # MSVC auto-defines COIN_DLL in Inventor/C/basic.h; mingw must be told.
    APP_CXXFLAGS += ' -DCOIN_DLL '

os.environ['COIN_LINK_NAME'] = COIN_LINK_NAME

helper.add_cxxflags(APP_CXXFLAGS).add_cpppath(APP_CPPPATH).add_libpath(APP_LIBPATH)
helper.set_dll_def('src/coin3d.def').set_libs(APP_LIBS).call(DefaultEnvironment)

BIN_DIR = os.path.join(APP_ROOT, 'bin')
if not os.path.exists(BIN_DIR):
  os.makedirs(BIN_DIR)

def copy_coin_runtime():
  # Unix: shared objects under lib/
  if os.path.isdir(COIN_LIB_DIR):
    for name in os.listdir(COIN_LIB_DIR):
      if not (name.startswith('libCoin') and (name.endswith('.dylib') or '.so' in name)):
        continue
      src = os.path.join(COIN_LIB_DIR, name)
      dst = os.path.join(BIN_DIR, name)
      try:
        if os.path.lexists(dst):
          os.remove(dst)
        if os.path.islink(src):
          os.symlink(os.readlink(src), dst)
        else:
          shutil.copy2(src, dst)
      except Exception as e:
        print('warn: copy Coin lib failed:', e)
  # Windows: copy only the matching DLL
  if os.path.isdir(COIN_BIN_DIR):
    if _IS_MINGW:
      # MinGW runtime is libCoin-80d.dll / libCoin-80.dll
      wanted = None
      for name in sorted(os.listdir(COIN_BIN_DIR)):
        if (name.startswith('libCoin') and name.lower().endswith('.dll')
            and name[:-4].endswith('d') == COIN_IS_DEBUG):
          wanted = name
          break
      if wanted is None:
        return
    else:
      wanted = COIN_LINK_NAME + '.dll'
    for name in os.listdir(COIN_BIN_DIR):
      if name.lower() != wanted.lower():
        # Remove mismatched Coin DLL from bin to avoid loader picking the wrong one.
        if (name.startswith('Coin') or name.startswith('libCoin')) and name.lower().endswith('.dll'):
          stale = os.path.join(BIN_DIR, name)
          if os.path.exists(stale) and name.lower() != wanted.lower():
            try:
              os.remove(stale)
            except Exception:
              pass
        continue
      src = os.path.join(COIN_BIN_DIR, name)
      dst = os.path.join(BIN_DIR, name)
      try:
        shutil.copy2(src, dst)
      except Exception as e:
        print('warn: copy Coin dll failed:', e)

copy_coin_runtime()

SConscriptFiles = ['src/SConscript', 'demos/SConscript', 'tests/SConscript']
helper.SConscript(SConscriptFiles)
