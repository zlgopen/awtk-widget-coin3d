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
# MSVC shared builds: Coin4d.lib (Debug) / Coin4.lib (Release).
if platform.system() == 'Windows':
  COIN_LINK_CANDIDATES = ['Coin4d', 'Coin4'] if COIN_IS_DEBUG else ['Coin4', 'Coin']
else:
  COIN_LINK_CANDIDATES = ['Coin', 'libCoin']

def find_coin_link_name():
  for name in COIN_LINK_CANDIDATES:
    for fname in (name + '.lib', 'lib' + name + '.dylib', 'lib' + name + '.so', 'lib' + name + '.a'):
      if os.path.exists(os.path.join(COIN_LIB_DIR, fname)):
        return name
  if os.path.isdir(COIN_LIB_DIR):
    for fname in os.listdir(COIN_LIB_DIR):
      if fname.startswith('libCoin.') and ('.so' in fname or fname.endswith('.dylib') or fname.endswith('.a')):
        return 'Coin'
  return None

def coin_lib_exists():
  return find_coin_link_name() is not None

def ensure_coin_built():
  print('Building Coin via CMake (%s, %s)...' % (COIN_GL_PROFILE, COIN_CMAKE_CONFIG))
  if platform.system() == 'Windows':
    glad_cmake = AWTK_GLAD_DIR.replace('\\', '/')
    subprocess.check_call([
      'cmake', '-S', COIN_SRC, '-B', COIN_BUILD,
      '-DCMAKE_BUILD_TYPE=' + COIN_CMAKE_CONFIG,
      '-DCOIN_GL_PROFILE=' + COIN_GL_PROFILE,
      '-DCOIN_BUILD_SHARED_LIBS=ON',
      '-DCOIN_BUILD_TESTS=OFF',
      '-DCOIN_BUILD_DOCUMENTATION=OFF',
      '-DCOIN_BUILD_EXAMPLES=OFF',
      '-DAWTK_GLAD_DIR=' + glad_cmake,
    ])
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

COIN_LINK_NAME = find_coin_link_name() or ('Coin4d' if COIN_IS_DEBUG else 'Coin4')
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
  # Windows: copy only the CRT-matching DLL (Coin4d.dll vs Coin4.dll).
  if os.path.isdir(COIN_BIN_DIR):
    wanted = COIN_LINK_NAME + '.dll'
    for name in os.listdir(COIN_BIN_DIR):
      if name.lower() != wanted.lower():
        # Remove mismatched Coin DLL from bin to avoid loader picking the wrong one.
        if name.startswith('Coin') and name.lower().endswith('.dll'):
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
