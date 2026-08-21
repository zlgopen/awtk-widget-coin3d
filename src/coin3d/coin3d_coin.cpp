/**
 * File:   coin3d_coin.cpp
 * Author: AWTK Develop Team
 * Brief:  Coin3D backend for coin3d widget
 *
 * Copyright (c) 2026 - 2026 Guangzhou ZHIYUAN Electronics Co.,Ltd.
 *
 */

/**
 * History:
 * ================================================================
 * 2026-08-17 Li XianJing <xianjimli@hotmail.com> created
 * 2026-08-18 Li XianJing <xianjimli@hotmail.com> node find/move/rotate/resize
 * 2026-08-19 Li XianJing <xianjimli@hotmail.com> File node include other .iv
 * 2026-08-19 Li XianJing <xianjimli@hotmail.com> runtime load .stl models
 * 2026-08-19 Li XianJing <xianjimli@hotmail.com> File node include .stl via temp .iv
 * 2026-08-20 Li XianJing <xianjimli@hotmail.com> get_background_rgb; render without Coin alpha-0 clear
 * 2026-08-20 Li XianJing <xianjimli@hotmail.com> viewAll then zoom out so models do not fill the viewport
 *
 */

#include "coin3d_coin.h"
#include "coin3d_view.hpp"

#include "tkc/mem.h"
#include "tkc/utils.h"
#include "tkc/fs.h"
#include "tkc/path.h"
#include "tkc/color_parser.h"
#include "base/assets_manager.h"
#include "base/system_info.h"

#include <Inventor/SoDB.h>
#include <Inventor/SoInput.h>
#include <Inventor/SoOutput.h>
#include <Inventor/SoPath.h>
#include <Inventor/SoSceneManager.h>
#include <Inventor/SbColor.h>
#include <Inventor/actions/SoSearchAction.h>
#include <Inventor/actions/SoWriteAction.h>
#include <Inventor/nodes/SoIndexedFaceSet.h>
#include <Inventor/nodes/SoCamera.h>
#include <Inventor/nodes/SoCube.h>
#include <Inventor/nodes/SoDirectionalLight.h>
#include <Inventor/nodes/SoFile.h>
#include <Inventor/nodes/SoGroup.h>
#include <Inventor/nodes/SoLight.h>
#include <Inventor/nodes/SoMaterial.h>
#include <Inventor/nodes/SoPerspectiveCamera.h>
#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/nodes/SoTransform.h>
#include <Inventor/sensors/SoSensorManager.h>
#include <Inventor/nodekits/SoNodeKit.h>
#include <Inventor/annex/ForeignFiles/SoSTLFileKit.h>

#include <cmath>
#include <cstring>

#define COIN3D_PI 3.14159265f
#define COIN3D_DEG_TO_RAD (COIN3D_PI / 180.0f)
#define COIN3D_RAD_TO_DEG (180.0f / COIN3D_PI)
#define COIN3D_ASIN_LIMIT 1.0f
#define COIN3D_DEFAULT_SCALE 1.0f
/* Coin viewAll 贴包围球，物体铺满视口；拉开距离与 XML scale=8 的立方体观感接近。 */
#define COIN3D_VIEWALL_DISTANCE_SCALE 1.5f
#define COIN3D_PATH_PARENT_FROM_TAIL 1
#define COIN3D_MIN_PATH_WITH_PARENT 2

#define COIN3D_DEFAULT_BG_R 0.12f
#define COIN3D_DEFAULT_BG_G 0.14f
#define COIN3D_DEFAULT_BG_B 0.18f
#define COIN3D_COLOR_SCALE (1.0f / 255.0f)
#define COIN3D_MAX_TMP_IV 16
#define COIN3D_IV_DATA_DIR "design/default/data"
#define COIN3D_MODEL_EXT_STL ".stl"
#define COIN3D_MODEL_EXT_IV ".iv"
#define COIN3D_MODEL_ALIAS_STL "_stl"

struct _coin3d_coin_t {
  SoSceneManager* scene_manager;
  SoSeparator* root;
  SoCamera* camera;
  Coin3dView view;
  bool_t sodb_ready;
  bool_t need_redraw;
  char* background;

  _coin3d_coin_t()
      : scene_manager(NULL),
        root(NULL),
        camera(NULL),
        sodb_ready(FALSE),
        need_redraw(FALSE),
        background(NULL) {
  }
};

static bool_t s_coin3d_sodb_inited = FALSE;

static void coin3d_coin_render_cb(void* user, SoSceneManager* manager) {
  coin3d_coin_t* coin = (coin3d_coin_t*)user;
  (void)manager;
  if (coin != NULL) {
    coin->need_redraw = TRUE;
  }
}

static void coin3d_coin_apply_background(coin3d_coin_t* coin) {
  float r = COIN3D_DEFAULT_BG_R;
  float g = COIN3D_DEFAULT_BG_G;
  float b = COIN3D_DEFAULT_BG_B;

  if (coin->background != NULL && coin->background[0] != '\0') {
    color_t c = color_parse(coin->background);
    r = c.rgba.r * COIN3D_COLOR_SCALE;
    g = c.rgba.g * COIN3D_COLOR_SCALE;
    b = c.rgba.b * COIN3D_COLOR_SCALE;
  }

  if (coin->scene_manager != NULL) {
    coin->scene_manager->setBackgroundColor(SbColor(r, g, b));
  }
}

static SoSeparator* coin3d_coin_create_builtin_scene(SoCamera** out_camera) {
  SoSeparator* root = new SoSeparator;
  root->ref();

  SoPerspectiveCamera* camera = new SoPerspectiveCamera;
  camera->nearDistance = 0.01f;
  camera->farDistance = 100.0f;
  root->addChild(camera);

  SoDirectionalLight* light = new SoDirectionalLight;
  light->direction = SbVec3f(0.2f, -1.0f, -0.5f);
  root->addChild(light);

  SoMaterial* mat = new SoMaterial;
  mat->diffuseColor = SbColor(1.0f, 0.8f, 0.2f);
  root->addChild(mat);
  root->addChild(new SoCube);

  if (out_camera != NULL) {
    *out_camera = camera;
  }
  return root;
}

static SoNode* coin3d_coin_find_node_of_type(SoNode* node, SoType type) {
  int32_t i = 0;
  SoGroup* group = NULL;

  if (node == NULL) {
    return NULL;
  }
  if (node->isOfType(type)) {
    return node;
  }
  if (!node->isOfType(SoGroup::getClassTypeId())) {
    return NULL;
  }

  group = (SoGroup*)node;
  for (i = 0; i < group->getNumChildren(); i++) {
    SoNode* found = coin3d_coin_find_node_of_type(group->getChild(i), type);
    if (found != NULL) {
      return found;
    }
  }
  return NULL;
}

static SoCamera* coin3d_coin_find_camera(SoSeparator* root) {
  return (SoCamera*)coin3d_coin_find_node_of_type(root, SoCamera::getClassTypeId());
}

static bool_t coin3d_coin_has_light(SoSeparator* root) {
  return coin3d_coin_find_node_of_type(root, SoLight::getClassTypeId()) != NULL ? TRUE : FALSE;
}

static SoSeparator* coin3d_coin_wrap_scene(SoSeparator* scene, SoCamera** out_camera) {
  SoSeparator* harness = new SoSeparator;
  harness->ref();

  SoCamera* camera = coin3d_coin_find_camera(scene);
  if (camera == NULL) {
    SoPerspectiveCamera* perspective = new SoPerspectiveCamera;
    perspective->nearDistance = 0.01f;
    perspective->farDistance = 100.0f;
    harness->addChild(perspective);
    camera = perspective;
  }

  if (!coin3d_coin_has_light(scene)) {
    SoDirectionalLight* light = new SoDirectionalLight;
    light->direction = SbVec3f(0.2f, -1.0f, -0.5f);
    harness->addChild(light);
  }

  harness->addChild(scene);
  if (out_camera != NULL) {
    *out_camera = camera;
  }
  return harness;
}

static const asset_info_t* coin3d_coin_ref_model_asset(const char* model) {
  assets_manager_t* am = assets_manager();
  const asset_info_t* info = NULL;
  char name[MAX_PATH + 1];
  uint32_t i = 0;

  return_value_if_fail(am != NULL && model != NULL && model[0] != '\0', NULL);

  info = assets_manager_ref(am, ASSET_TYPE_DATA, model);
  if (info != NULL) {
    return info;
  }

  /* design 资源打包后，cube.iv 通常登记为 cube_iv */
  tk_strncpy(name, model, sizeof(name) - 1);
  for (i = 0; name[i] != '\0'; i++) {
    if (name[i] == '.' || name[i] == '/' || name[i] == '\\') {
      name[i] = '_';
    }
  }
  if (!tk_str_eq(name, model)) {
    info = assets_manager_ref(am, ASSET_TYPE_DATA, name);
  }
  return info;
}

static ret_t coin3d_coin_resolve_model_path(const char* filename, char* out, uint32_t size);

static ret_t coin3d_coin_try_iv_path(char* out, uint32_t size, const char* dir, const char* name) {
  return_value_if_fail(out != NULL && size > 0 && name != NULL && name[0] != '\0', RET_BAD_PARAMS);
  if (dir == NULL || dir[0] == '\0') {
    return RET_BAD_PARAMS;
  }
  if (path_build(out, size, dir, name, NULL) != RET_OK) {
    return RET_FAIL;
  }
  return file_exist(out) ? RET_OK : RET_NOT_FOUND;
}

static bool_t coin3d_coin_is_user_iv_file(const char* path) {
  const char* base = NULL;

  if (path == NULL || path[0] == '\0') {
    return FALSE;
  }
  if (!path_extname_is(path, COIN3D_MODEL_EXT_IV)) {
    return FALSE;
  }
  base = strrchr(path, '/');
  base = base != NULL ? base + 1 : path;
  return strncmp(base, "coin3d_stl_", 11) != 0;
}

static ret_t coin3d_coin_resolve_stl_path(const char* filename, const char* cur_file, char* out,
                                          uint32_t size) {
  char dir[MAX_PATH + 1];

  memset(dir, 0, sizeof(dir));
  return_value_if_fail(filename != NULL && filename[0] != '\0', RET_BAD_PARAMS);
  return_value_if_fail(out != NULL && size > 0, RET_BAD_PARAMS);
  memset(out, 0, size);

  if (file_exist(filename)) {
    tk_strncpy(out, filename, size - 1);
    return RET_OK;
  }

  if (coin3d_coin_is_user_iv_file(cur_file)) {
    if (path_dirname(cur_file, dir, sizeof(dir)) == RET_OK &&
        coin3d_coin_try_iv_path(out, size, dir, filename) == RET_OK) {
      return RET_OK;
    }
  }

  return coin3d_coin_resolve_model_path(filename, out, size);
}

static ret_t coin3d_coin_resolve_model_path(const char* model, char* out, uint32_t size) {
  char base[MAX_PATH + 1];

  memset(base, 0, sizeof(base));
  return_value_if_fail(model != NULL && model[0] != '\0', RET_BAD_PARAMS);
  return_value_if_fail(out != NULL && size > 0, RET_BAD_PARAMS);
  memset(out, 0, size);

  if (file_exist(model)) {
    tk_strncpy(out, model, size - 1);
    return RET_OK;
  }

  if (path_basename(model, base, sizeof(base)) != RET_OK || base[0] == '\0') {
    return RET_NOT_FOUND;
  }

#ifdef APP_ROOT
  {
    char data_dir[MAX_PATH + 1];
    memset(data_dir, 0, sizeof(data_dir));
    if (path_build(data_dir, sizeof(data_dir), APP_ROOT, "design", "default", "data", NULL) ==
            RET_OK &&
        coin3d_coin_try_iv_path(out, size, data_dir, base) == RET_OK) {
      return RET_OK;
    }
  }
#endif

  if (coin3d_coin_try_iv_path(out, size, COIN3D_IV_DATA_DIR, base) == RET_OK) {
    return RET_OK;
  }

  return RET_NOT_FOUND;
}

static bool_t coin3d_coin_model_is_stl(const char* model) {
  uint32_t len = 0;

  return_value_if_fail(model != NULL && model[0] != '\0', FALSE);
  if (path_extname_is(model, COIN3D_MODEL_EXT_STL)) {
    return TRUE;
  }

  len = (uint32_t)strlen(model);
  if (len > strlen(COIN3D_MODEL_ALIAS_STL) &&
      tk_str_ieq(model + len - strlen(COIN3D_MODEL_ALIAS_STL), COIN3D_MODEL_ALIAS_STL)) {
    return TRUE;
  }
  return FALSE;
}

static bool_t coin3d_coin_name_has_stl_ext(const char* name) {
  return_value_if_fail(name != NULL && name[0] != '\0', FALSE);
  return path_extname_is(name, COIN3D_MODEL_EXT_STL);
}

static ret_t coin3d_coin_materialize_asset_file(const char* name, bool_t force_stl_ext, char* out,
                                                  uint32_t size) {
  const asset_info_t* info = NULL;
  char dir[MAX_PATH + 1];
  char base[MAX_PATH + 1];
  char tmp_name[MAX_PATH + 1];
  static uint32_t s_tmp_seq = 0;

  memset(dir, 0, sizeof(dir));
  memset(base, 0, sizeof(base));
  memset(tmp_name, 0, sizeof(tmp_name));
  return_value_if_fail(name != NULL && name[0] != '\0', RET_BAD_PARAMS);
  return_value_if_fail(out != NULL && size > 0, RET_BAD_PARAMS);

  info = coin3d_coin_ref_model_asset(name);
  return_value_if_fail(info != NULL, RET_NOT_FOUND);
  if (fs_get_temp_path(os_fs(), dir) != RET_OK || path_basename(name, base, sizeof(base)) != RET_OK ||
      base[0] == '\0') {
    assets_manager_unref(assets_manager(), info);
    return RET_FAIL;
  }

  if (force_stl_ext && !coin3d_coin_name_has_stl_ext(base)) {
    tk_snprintf(tmp_name, sizeof(tmp_name), "coin3d_inc_%u_%s%s", s_tmp_seq++, base,
                COIN3D_MODEL_EXT_STL);
  } else {
    tk_snprintf(tmp_name, sizeof(tmp_name), "coin3d_inc_%u_%s", s_tmp_seq++, base);
  }
  if (path_build(out, size, dir, tmp_name, NULL) != RET_OK ||
      file_write(out, info->data, info->size) != RET_OK) {
    assets_manager_unref(assets_manager(), info);
    return RET_FAIL;
  }

  assets_manager_unref(assets_manager(), info);
  return RET_OK;
}

static SoSeparator* coin3d_coin_read_stl_file(const char* path);

static void coin3d_coin_reset_input_dirs(void) {
  char data_dir[MAX_PATH + 1];

  memset(data_dir, 0, sizeof(data_dir));
  SoInput::clearDirectories();
#ifdef APP_ROOT
  if (path_build(data_dir, sizeof(data_dir), APP_ROOT, "design", "default", "data", NULL) ==
          RET_OK &&
      path_exist(data_dir)) {
    SoInput::addDirectoryLast(data_dir);
  }
#endif
  if (path_exist(COIN3D_IV_DATA_DIR)) {
    SoInput::addDirectoryLast(COIN3D_IV_DATA_DIR);
  }
}

static ret_t coin3d_coin_stage_stl_for_read(const char* src, char* out, uint32_t size) {
  char dir[MAX_PATH + 1];
  char base[MAX_PATH + 1];
  char tmp_name[MAX_PATH + 1];
  void* data = NULL;
  uint32_t data_size = 0;
  static uint32_t s_tmp_seq = 0;

  memset(dir, 0, sizeof(dir));
  memset(base, 0, sizeof(base));
  memset(tmp_name, 0, sizeof(tmp_name));
  return_value_if_fail(src != NULL && src[0] != '\0', RET_BAD_PARAMS);
  return_value_if_fail(out != NULL && size > 0, RET_BAD_PARAMS);

  data = file_read(src, &data_size);
  if (data == NULL || data_size == 0) {
    TKMEM_FREE(data);
    return RET_FAIL;
  }

  if (fs_get_temp_path(os_fs(), dir) != RET_OK ||
      path_basename(src, base, sizeof(base)) != RET_OK || base[0] == '\0') {
    TKMEM_FREE(data);
    return RET_FAIL;
  }

  tk_snprintf(tmp_name, sizeof(tmp_name), "coin3d_src_%u_%s", s_tmp_seq++, base);
  if (path_build(out, size, dir, tmp_name, NULL) != RET_OK ||
      file_write(out, data, data_size) != RET_OK) {
    TKMEM_FREE(data);
    return RET_FAIL;
  }

  TKMEM_FREE(data);
  return RET_OK;
}

static ret_t coin3d_coin_materialize_stl_as_iv(const char* stl_path, char* out, uint32_t size) {
  SoSeparator* scene = NULL;
  SoOutput output;
  char dir[MAX_PATH + 1];
  char base[MAX_PATH + 1];
  char tmp_name[MAX_PATH + 1];
  char staged_stl[MAX_PATH + 1];
  static uint32_t s_tmp_seq = 0;

  memset(dir, 0, sizeof(dir));
  memset(base, 0, sizeof(base));
  memset(tmp_name, 0, sizeof(tmp_name));
  memset(staged_stl, 0, sizeof(staged_stl));
  return_value_if_fail(stl_path != NULL && stl_path[0] != '\0', RET_BAD_PARAMS);
  return_value_if_fail(out != NULL && size > 0, RET_BAD_PARAMS);

  scene = coin3d_coin_read_stl_file(stl_path);
  if (scene == NULL) {
    if (coin3d_coin_stage_stl_for_read(stl_path, staged_stl, sizeof(staged_stl)) != RET_OK) {
      return RET_FAIL;
    }

    scene = coin3d_coin_read_stl_file(staged_stl);
    file_remove(staged_stl);
    if (scene == NULL) {
      return RET_FAIL;
    }
  }

  if (fs_get_temp_path(os_fs(), dir) != RET_OK ||
      path_basename(stl_path, base, sizeof(base)) != RET_OK || base[0] == '\0') {
    scene->unref();
    return RET_FAIL;
  }

  tk_snprintf(tmp_name, sizeof(tmp_name), "coin3d_stl_%u_%s%s", s_tmp_seq++, base,
              COIN3D_MODEL_EXT_IV);
  if (path_build(out, size, dir, tmp_name, NULL) != RET_OK || !output.openFile(out)) {
    scene->unref();
    return RET_FAIL;
  }

  {
    SoWriteAction writer(&output);
    writer.apply(scene);
  }
  output.closeFile();
  scene->unref();
  return RET_OK;
}

class Coin3dSoInput : public SoInput {
 public:
  Coin3dSoInput(void) : tmp_count(0) {
    memset(tmp_paths, 0, sizeof(tmp_paths));
    context_iv_path[0] = '\0';
  }

  void set_context_iv_path(const char* path) {
    context_iv_path[0] = '\0';
    if (path != NULL && path[0] != '\0') {
      tk_strncpy(context_iv_path, path, MAX_PATH);
    }
  }

  virtual ~Coin3dSoInput() {
    uint32_t i = 0;
    for (i = 0; i < tmp_count; i++) {
      if (tmp_paths[i][0] != '\0') {
        file_remove(tmp_paths[i]);
      }
    }
  }

  virtual SbBool pushFile(const char* filename) {
    char path[MAX_PATH + 1];

    memset(path, 0, sizeof(path));
    if (filename == NULL || filename[0] == '\0') {
      return FALSE;
    }

    if (coin3d_coin_model_is_stl(filename)) {
      if (tmp_count >= COIN3D_MAX_TMP_IV || push_stl_include(filename, path, sizeof(path)) != RET_OK) {
        return FALSE;
      }
      tk_strncpy(tmp_paths[tmp_count], path, MAX_PATH);
      tmp_count++;
      return SoInput::pushFile(path);
    }

    if (coin3d_coin_resolve_model_path(filename, path, sizeof(path)) == RET_OK) {
      return SoInput::pushFile(path);
    }

    if (tmp_count < COIN3D_MAX_TMP_IV &&
        coin3d_coin_materialize_asset_file(filename, coin3d_coin_model_is_stl(filename), path,
                                          sizeof(path)) == RET_OK) {
      tk_strncpy(tmp_paths[tmp_count], path, MAX_PATH);
      tmp_count++;
      return SoInput::pushFile(path);
    }

    return SoInput::pushFile(filename);
  }

 private:
  ret_t push_stl_include(const char* filename, char* out, uint32_t size) {
    char stl_path[MAX_PATH + 1];
    char asset_stl[MAX_PATH + 1];
    const char* cur_file = NULL;
    bool_t from_asset = FALSE;

    memset(stl_path, 0, sizeof(stl_path));
    memset(asset_stl, 0, sizeof(asset_stl));
    return_value_if_fail(filename != NULL && filename[0] != '\0', RET_BAD_PARAMS);
    return_value_if_fail(out != NULL && size > 0, RET_BAD_PARAMS);

    cur_file = getCurFileName();
    if (cur_file == NULL || cur_file[0] == '\0') {
      cur_file = context_iv_path[0] != '\0' ? context_iv_path : NULL;
    }
    if (coin3d_coin_resolve_stl_path(filename, cur_file, stl_path, sizeof(stl_path)) != RET_OK) {
      if (coin3d_coin_materialize_asset_file(filename, TRUE, asset_stl, sizeof(asset_stl)) !=
          RET_OK) {
        if (!file_exist(filename)) {
          return RET_NOT_FOUND;
        }
        tk_strncpy(stl_path, filename, sizeof(stl_path) - 1);
      } else {
        from_asset = TRUE;
        tk_strncpy(stl_path, asset_stl, sizeof(stl_path) - 1);
      }
    }

    if (coin3d_coin_materialize_stl_as_iv(stl_path, out, size) != RET_OK) {
      if (from_asset) {
        file_remove(asset_stl);
      }
      return RET_FAIL;
    }
    if (from_asset) {
      file_remove(asset_stl);
    }
    return RET_OK;
  }

  char context_iv_path[MAX_PATH + 1];
  char tmp_paths[COIN3D_MAX_TMP_IV][MAX_PATH + 1];
  uint32_t tmp_count;
};

static SoSeparator* coin3d_coin_read_model_file(const char* path) {
  void* data = NULL;
  uint32_t size = 0;
  char dir[MAX_PATH + 1];
  char data_dir[MAX_PATH + 1];
  SoSeparator* scene = NULL;
  Coin3dSoInput input;

  memset(dir, 0, sizeof(dir));
  memset(data_dir, 0, sizeof(data_dir));
  return_value_if_fail(path != NULL && path[0] != '\0', NULL);
  coin3d_coin_reset_input_dirs();
  input.set_context_iv_path(path);
  data = file_read(path, &size);
  return_value_if_fail(data != NULL && size > 0, NULL);

#ifdef APP_ROOT
  if (path_build(data_dir, sizeof(data_dir), APP_ROOT, "design", "default", "data", NULL) ==
      RET_OK) {
    input.addDirectoryFirst(data_dir);
  }
#endif
  input.addDirectoryFirst(COIN3D_IV_DATA_DIR);
  if (path_dirname(path, dir, sizeof(dir)) == RET_OK && dir[0] != '\0') {
    input.addDirectoryFirst(dir);
  }
  input.setBuffer(data, size);
  scene = SoDB::readAll(&input);
  TKMEM_FREE(data);
  coin3d_coin_reset_input_dirs();
  return scene;
}

static SoSeparator* coin3d_coin_read_iv_buffer(const void* data, uint32_t size,
                                                const char* context_iv_path) {
  Coin3dSoInput input;
  char data_dir[MAX_PATH + 1];
  SoSeparator* scene = NULL;

  memset(data_dir, 0, sizeof(data_dir));
  if (data == NULL || size == 0) {
    return NULL;
  }

  coin3d_coin_reset_input_dirs();
  input.set_context_iv_path(context_iv_path);
#ifdef APP_ROOT
  if (path_build(data_dir, sizeof(data_dir), APP_ROOT, "design", "default", "data", NULL) ==
      RET_OK) {
    input.addDirectoryFirst(data_dir);
  }
#endif
  input.addDirectoryFirst(COIN3D_IV_DATA_DIR);

  input.setBuffer((void*)data, size);
  scene = SoDB::readAll(&input);
  coin3d_coin_reset_input_dirs();
  return scene;
}

static SoSeparator* coin3d_coin_read_stl_file(const char* path) {
  SoSeparator* scene = NULL;
  SoSTLFileKit* kit = NULL;

  return_value_if_fail(path != NULL && path[0] != '\0', NULL);
  if (!SoSTLFileKit::identify(path)) {
    return NULL;
  }

  kit = new SoSTLFileKit;
  kit->ref();
  if (!kit->readFile(path)) {
    kit->unref();
    return NULL;
  }

  scene = kit->convert();
  kit->unref();
  if (scene == NULL) {
    return NULL;
  }
  scene->ref();
  return scene;
}

static SoSeparator* coin3d_coin_load_stl_model(const char* model) {
  char path[MAX_PATH + 1];
  SoSeparator* scene = NULL;

  memset(path, 0, sizeof(path));
  return_value_if_fail(model != NULL && model[0] != '\0', NULL);

  if (coin3d_coin_resolve_model_path(model, path, sizeof(path)) == RET_OK) {
    return coin3d_coin_read_stl_file(path);
  }

  if (coin3d_coin_materialize_asset_file(model, TRUE, path, sizeof(path)) == RET_OK) {
    scene = coin3d_coin_read_stl_file(path);
    file_remove(path);
    return scene;
  }

  if (file_exist(model)) {
    return coin3d_coin_read_stl_file(model);
  }

  return NULL;
}

static void coin3d_coin_attach_root(coin3d_coin_t* coin, SoSeparator* root, SoCamera* camera) {
  if (coin->root != NULL) {
    coin->root->unref();
    coin->root = NULL;
  }
  coin->root = root;
  coin->camera = camera;

  if (coin->scene_manager == NULL) {
    coin->scene_manager = new SoSceneManager;
    coin->scene_manager->setRenderCallback(coin3d_coin_render_cb, coin);
    coin->scene_manager->activate();
  }

  coin->scene_manager->setSceneGraph(root);
  coin3d_coin_apply_background(coin);

  if (camera != NULL) {
    camera->viewAll(root, coin->scene_manager->getViewportRegion());
    /* 避免旋转后近裁切过紧 */
    if (camera->isOfType(SoPerspectiveCamera::getClassTypeId())) {
      SoPerspectiveCamera* pc = (SoPerspectiveCamera*)camera;
      if (pc->nearDistance.getValue() < 0.01f) {
        pc->nearDistance = 0.01f;
      }
      if (pc->farDistance.getValue() < 100.0f) {
        pc->farDistance = 100.0f;
      }
    }
  }

  coin->view.init(camera, coin->scene_manager);
  if (camera != NULL) {
    coin->view.setDistance(coin->view.getDistance() * COIN3D_VIEWALL_DISTANCE_SCALE);
  }
  coin->need_redraw = TRUE;
}

static ret_t coin3d_coin_load_builtin(coin3d_coin_t* coin) {
  SoCamera* camera = NULL;
  SoSeparator* root = coin3d_coin_create_builtin_scene(&camera);
  return_value_if_fail(root != NULL, RET_FAIL);
  coin3d_coin_attach_root(coin, root, camera);
  return RET_OK;
}

coin3d_coin_t* coin3d_coin_create(void) {
  return new coin3d_coin_t();
}

ret_t coin3d_coin_destroy(coin3d_coin_t* coin) {
  return_value_if_fail(coin != NULL, RET_BAD_PARAMS);

  coin->view.reset();
  if (coin->scene_manager != NULL) {
    coin->scene_manager->setSceneGraph(NULL);
    coin->scene_manager->setRenderCallback(NULL, NULL);
    delete coin->scene_manager;
    coin->scene_manager = NULL;
  }
  if (coin->root != NULL) {
    coin->root->unref();
    coin->root = NULL;
  }
  coin->camera = NULL;
  TKMEM_FREE(coin->background);
  delete coin;
  return RET_OK;
}

ret_t coin3d_coin_ensure_ready(coin3d_coin_t* coin) {
  return_value_if_fail(coin != NULL, RET_BAD_PARAMS);

  if (!s_coin3d_sodb_inited) {
    SoDB::init();
    SoNodeKit::init();
    SoFile::setSearchOK(TRUE);
#ifdef APP_ROOT
    {
      char data_dir[MAX_PATH + 1];
      memset(data_dir, 0, sizeof(data_dir));
      if (path_build(data_dir, sizeof(data_dir), APP_ROOT, "design", "default", "data", NULL) ==
              RET_OK &&
          path_exist(data_dir)) {
        SoInput::addDirectoryLast(data_dir);
      }
    }
#endif
    if (path_exist(COIN3D_IV_DATA_DIR)) {
      SoInput::addDirectoryLast(COIN3D_IV_DATA_DIR);
    }
    s_coin3d_sodb_inited = TRUE;
  }
  coin->sodb_ready = TRUE;

  if (coin->scene_manager == NULL || coin->root == NULL) {
    return coin3d_coin_load_builtin(coin);
  }
  return RET_OK;
}

ret_t coin3d_coin_load_model(coin3d_coin_t* coin, const char* model) {
  const asset_info_t* info = NULL;
  SoSeparator* scene = NULL;
  SoSeparator* root = NULL;
  SoCamera* camera = NULL;
  char path[MAX_PATH + 1];

  memset(path, 0, sizeof(path));
  return_value_if_fail(coin != NULL, RET_BAD_PARAMS);
  return_value_if_fail(coin3d_coin_ensure_ready(coin) == RET_OK, RET_FAIL);
  coin3d_coin_reset_input_dirs();

  if (model == NULL || model[0] == '\0') {
    return coin3d_coin_load_builtin(coin);
  }

  if (coin3d_coin_model_is_stl(model)) {
    scene = coin3d_coin_load_stl_model(model);
  } else {
    if (coin3d_coin_resolve_model_path(model, path, sizeof(path)) == RET_OK) {
      scene = coin3d_coin_read_model_file(path);
    }

    if (scene == NULL) {
      info = coin3d_coin_ref_model_asset(model);
      if (info != NULL) {
        char ctx[MAX_PATH + 1];
        memset(ctx, 0, sizeof(ctx));
        if (coin3d_coin_resolve_model_path(model, ctx, sizeof(ctx)) != RET_OK) {
          ctx[0] = '\0';
        }
        scene = coin3d_coin_read_iv_buffer(info->data, info->size, ctx);
        assets_manager_unref(assets_manager(), info);
      }
    }
  }

  if (scene == NULL) {
    log_warn("coin3d: load model \"%s\" failed, fallback to builtin cube\n", model);
    return coin3d_coin_load_builtin(coin);
  }

  root = coin3d_coin_wrap_scene(scene, &camera);
  if (root == NULL) {
    scene->unref();
    return coin3d_coin_load_builtin(coin);
  }

  coin3d_coin_attach_root(coin, root, camera);
  return RET_OK;
}

ret_t coin3d_coin_set_background(coin3d_coin_t* coin, const char* background) {
  return_value_if_fail(coin != NULL, RET_BAD_PARAMS);
  TKMEM_FREE(coin->background);
  coin->background = tk_strdup(background != NULL ? background : "");
  coin3d_coin_apply_background(coin);
  coin->need_redraw = TRUE;
  return RET_OK;
}

ret_t coin3d_coin_get_background_rgb(coin3d_coin_t* coin, float* r, float* g, float* b) {
  return_value_if_fail(coin != NULL && r != NULL && g != NULL && b != NULL, RET_BAD_PARAMS);

  *r = COIN3D_DEFAULT_BG_R;
  *g = COIN3D_DEFAULT_BG_G;
  *b = COIN3D_DEFAULT_BG_B;
  if (coin->scene_manager != NULL) {
    const SbColor& bg = coin->scene_manager->getBackgroundColor();
    *r = bg[0];
    *g = bg[1];
    *b = bg[2];
  } else if (coin->background != NULL && coin->background[0] != '\0') {
    color_t c = color_parse(coin->background);
    *r = c.rgba.r * COIN3D_COLOR_SCALE;
    *g = c.rgba.g * COIN3D_COLOR_SCALE;
    *b = c.rgba.b * COIN3D_COLOR_SCALE;
  }

  return RET_OK;
}

ret_t coin3d_coin_set_viewport(coin3d_coin_t* coin, widget_t* widget) {
  float_t ratio = 1.0f;
  system_info_t* si = system_info();
  int32_t w = 0;
  int32_t h = 0;

  return_value_if_fail(coin != NULL && widget != NULL, RET_BAD_PARAMS);
  return_value_if_fail(coin3d_coin_ensure_ready(coin) == RET_OK, RET_FAIL);

  if (si != NULL) {
    ratio = si->device_pixel_ratio;
  }
  w = (int32_t)(widget->w * ratio);
  h = (int32_t)(widget->h * ratio);
  if (w < 1) {
    w = 1;
  }
  if (h < 1) {
    h = 1;
  }

  coin->scene_manager->setWindowSize(SbVec2s((short)w, (short)h));
  coin->scene_manager->setSize(SbVec2s((short)w, (short)h));
  coin->scene_manager->setViewportRegion(SbViewportRegion((short)w, (short)h));
  return RET_OK;
}

ret_t coin3d_coin_render(coin3d_coin_t* coin) {
  return_value_if_fail(coin != NULL, RET_BAD_PARAMS);
  return_value_if_fail(coin3d_coin_ensure_ready(coin) == RET_OK, RET_FAIL);

  /* 调用方已按不透明背景清过 color/depth；Coin 默认清屏 alpha=0，会让 snapshot FBO 变全透明 */
  coin->scene_manager->render(FALSE, FALSE);
  coin->need_redraw = FALSE;
  return RET_OK;
}

bool_t coin3d_coin_process_sensors(coin3d_coin_t* coin) {
  return_value_if_fail(coin != NULL, FALSE);
  if (!coin->sodb_ready || !s_coin3d_sodb_inited) {
    return FALSE;
  }

  SoDB::getSensorManager()->processTimerQueue();
  SoDB::getSensorManager()->processDelayQueue(TRUE);
  if (coin->need_redraw) {
    coin->need_redraw = FALSE;
    return TRUE;
  }
  return FALSE;
}

SoCamera* coin3d_coin_get_camera(coin3d_coin_t* coin) {
  return coin != NULL ? coin->camera : NULL;
}

Coin3dView* coin3d_coin_get_view(coin3d_coin_t* coin) {
  return coin != NULL ? &coin->view : NULL;
}

ret_t coin3d_coin_handle_pointer(coin3d_coin_t* coin, widget_t* widget, event_t* e) {
  return_value_if_fail(coin != NULL && widget != NULL && e != NULL, RET_BAD_PARAMS);
  return_value_if_fail(coin3d_coin_ensure_ready(coin) == RET_OK, RET_FAIL);

  if (coin->view.onEvent(widget, e)) {
    coin->need_redraw = TRUE;
    return RET_OK;
  }
  return RET_NOT_FOUND;
}

static SoPath* coin3d_coin_search_name(SoNode* root, const char* name, SoSearchAction* action) {
  if (root == NULL || name == NULL || name[0] == '\0' || action == NULL) {
    return NULL;
  }

  action->setSearchingAll(TRUE);
  action->setInterest(SoSearchAction::FIRST);
  action->setName(SbName(name));
  action->apply(root);
  return action->getPath();
}

static SoTransform* coin3d_coin_ensure_transform(SoNode* root, const char* name, bool_t create) {
  SoSearchAction action;
  SoPath* path = NULL;
  SoNode* node = NULL;
  SoGroup* group = NULL;
  int32_t i = 0;
  int32_t index = 0;

  path = coin3d_coin_search_name(root, name, &action);
  if (path == NULL) {
    return NULL;
  }

  node = path->getTail();
  if (node->isOfType(SoTransform::getClassTypeId())) {
    return (SoTransform*)node;
  }

  if (node->isOfType(SoGroup::getClassTypeId())) {
    group = (SoGroup*)node;
    for (i = 0; i < group->getNumChildren(); i++) {
      SoNode* child = group->getChild(i);
      if (child->isOfType(SoTransform::getClassTypeId())) {
        return (SoTransform*)child;
      }
    }
    if (!create) {
      return NULL;
    }
    SoTransform* xf = new SoTransform;
    group->insertChild(xf, 0);
    return xf;
  }

  if (path->getLength() < COIN3D_MIN_PATH_WITH_PARENT) {
    return NULL;
  }

  node = path->getTail();
  SoNode* parent_node = path->getNodeFromTail(COIN3D_PATH_PARENT_FROM_TAIL);
  if (!parent_node->isOfType(SoGroup::getClassTypeId())) {
    return NULL;
  }

  group = (SoGroup*)parent_node;
  index = group->findChild(node);
  if (index > 0) {
    SoNode* prev = group->getChild(index - 1);
    if (prev->isOfType(SoTransform::getClassTypeId())) {
      return (SoTransform*)prev;
    }
  }
  if (!create || index < 0) {
    return NULL;
  }

  SoTransform* xf = new SoTransform;
  group->insertChild(xf, index);
  return xf;
}

static SbRotation coin3d_coin_xyz_deg_to_rotation(float x, float y, float z) {
  SbRotation rx(SbVec3f(1.0f, 0.0f, 0.0f), x * COIN3D_DEG_TO_RAD);
  SbRotation ry(SbVec3f(0.0f, 1.0f, 0.0f), y * COIN3D_DEG_TO_RAD);
  SbRotation rz(SbVec3f(0.0f, 0.0f, 1.0f), z * COIN3D_DEG_TO_RAD);
  return rx * ry * rz;
}

static void coin3d_coin_rotation_to_xyz_deg(const SbRotation& rot, float* x, float* y, float* z) {
  SbMatrix m;
  float s = 0.0f;

  rot.getValue(m);
  s = m[2][0];
  if (s > COIN3D_ASIN_LIMIT) {
    s = COIN3D_ASIN_LIMIT;
  } else if (s < -COIN3D_ASIN_LIMIT) {
    s = -COIN3D_ASIN_LIMIT;
  }

  *y = asinf(s) * COIN3D_RAD_TO_DEG;
  *x = atan2f(-m[2][1], m[2][2]) * COIN3D_RAD_TO_DEG;
  *z = atan2f(-m[1][0], m[0][0]) * COIN3D_RAD_TO_DEG;
}

uint32_t coin3d_coin_count_indexed_face_sets(coin3d_coin_t* coin) {
  SoSearchAction action;
  return_value_if_fail(coin != NULL && coin->root != NULL, 0);

  action.setType(SoIndexedFaceSet::getClassTypeId());
  action.setInterest(SoSearchAction::ALL);
  action.setSearchingAll(TRUE);
  action.apply(coin->root);
  return (uint32_t)action.getPaths().getLength();
}

void* coin3d_coin_find_node(coin3d_coin_t* coin, const char* name) {
  SoSearchAction action;
  SoPath* path = NULL;

  return_value_if_fail(coin != NULL && coin->root != NULL, NULL);
  return_value_if_fail(name != NULL && name[0] != '\0', NULL);

  path = coin3d_coin_search_name(coin->root, name, &action);
  return path != NULL ? path->getTail() : NULL;
}

static SoTransform* coin3d_coin_require_transform(coin3d_coin_t* coin, const char* name) {
  return_value_if_fail(coin != NULL && coin->root != NULL, NULL);
  return_value_if_fail(name != NULL && name[0] != '\0', NULL);
  return coin3d_coin_ensure_transform(coin->root, name, TRUE);
}

ret_t coin3d_coin_node_move(coin3d_coin_t* coin, const char* name, float x, float y, float z) {
  SoTransform* xf = NULL;
  return_value_if_fail(coin != NULL && name != NULL && name[0] != '\0', RET_BAD_PARAMS);
  xf = coin3d_coin_require_transform(coin, name);
  return_value_if_fail(xf != NULL, RET_NOT_FOUND);

  xf->translation.setValue(x, y, z);
  coin->need_redraw = TRUE;
  return RET_OK;
}

ret_t coin3d_coin_node_rotate(coin3d_coin_t* coin, const char* name, float x, float y, float z) {
  SoTransform* xf = NULL;
  return_value_if_fail(coin != NULL && name != NULL && name[0] != '\0', RET_BAD_PARAMS);
  xf = coin3d_coin_require_transform(coin, name);
  return_value_if_fail(xf != NULL, RET_NOT_FOUND);

  xf->rotation = coin3d_coin_xyz_deg_to_rotation(x, y, z);
  coin->need_redraw = TRUE;
  return RET_OK;
}

ret_t coin3d_coin_node_resize(coin3d_coin_t* coin, const char* name, float x, float y, float z) {
  SoTransform* xf = NULL;
  return_value_if_fail(coin != NULL && name != NULL && name[0] != '\0', RET_BAD_PARAMS);
  xf = coin3d_coin_require_transform(coin, name);
  return_value_if_fail(xf != NULL, RET_NOT_FOUND);

  xf->scaleFactor.setValue(x, y, z);
  coin->need_redraw = TRUE;
  return RET_OK;
}

ret_t coin3d_coin_node_get_translation(coin3d_coin_t* coin, const char* name, float* x, float* y,
                                      float* z) {
  SoTransform* xf = NULL;
  SbVec3f v;
  return_value_if_fail(coin != NULL && coin->root != NULL, RET_BAD_PARAMS);
  return_value_if_fail(name != NULL && name[0] != '\0', RET_BAD_PARAMS);
  return_value_if_fail(x != NULL && y != NULL && z != NULL, RET_BAD_PARAMS);
  return_value_if_fail(coin3d_coin_find_node(coin, name) != NULL, RET_NOT_FOUND);

  xf = coin3d_coin_ensure_transform(coin->root, name, FALSE);
  if (xf == NULL) {
    *x = 0.0f;
    *y = 0.0f;
    *z = 0.0f;
    return RET_OK;
  }

  v = xf->translation.getValue();
  *x = v[0];
  *y = v[1];
  *z = v[2];
  return RET_OK;
}

ret_t coin3d_coin_node_get_rotation(coin3d_coin_t* coin, const char* name, float* x, float* y,
                                   float* z) {
  SoTransform* xf = NULL;
  return_value_if_fail(coin != NULL && coin->root != NULL, RET_BAD_PARAMS);
  return_value_if_fail(name != NULL && name[0] != '\0', RET_BAD_PARAMS);
  return_value_if_fail(x != NULL && y != NULL && z != NULL, RET_BAD_PARAMS);
  return_value_if_fail(coin3d_coin_find_node(coin, name) != NULL, RET_NOT_FOUND);

  xf = coin3d_coin_ensure_transform(coin->root, name, FALSE);
  if (xf == NULL) {
    *x = 0.0f;
    *y = 0.0f;
    *z = 0.0f;
    return RET_OK;
  }

  coin3d_coin_rotation_to_xyz_deg(xf->rotation.getValue(), x, y, z);
  return RET_OK;
}

ret_t coin3d_coin_node_get_scale(coin3d_coin_t* coin, const char* name, float* x, float* y,
                                float* z) {
  SoTransform* xf = NULL;
  SbVec3f v;
  return_value_if_fail(coin != NULL && coin->root != NULL, RET_BAD_PARAMS);
  return_value_if_fail(name != NULL && name[0] != '\0', RET_BAD_PARAMS);
  return_value_if_fail(x != NULL && y != NULL && z != NULL, RET_BAD_PARAMS);
  return_value_if_fail(coin3d_coin_find_node(coin, name) != NULL, RET_NOT_FOUND);

  xf = coin3d_coin_ensure_transform(coin->root, name, FALSE);
  if (xf == NULL) {
    *x = COIN3D_DEFAULT_SCALE;
    *y = COIN3D_DEFAULT_SCALE;
    *z = COIN3D_DEFAULT_SCALE;
    return RET_OK;
  }

  v = xf->scaleFactor.getValue();
  *x = v[0];
  *y = v[1];
  *z = v[2];
  return RET_OK;
}
