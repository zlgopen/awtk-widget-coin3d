#include <string.h>
#include <vector>
#include "awtk.h"
#include "coin3d/coin3d.h"
#include "coin3d/coin3d_coin.h"
#include "coin3d/coin3d_gl.h"
#include "coin3d/coin3d_gizmo.h"
#include "coin3d/coin3d_gizmo.hpp"
#include "coin3d/coin3d_view.hpp"
#include "gtest/gtest.h"
#include "../demos/demo_model_arg.h"
#include "../demos/demo_screenshot.h"
#include "../demos/demo_set_model.h"
#include "../demos/demo_transform.h"

#include <Inventor/SoDB.h>
#include <Inventor/SoInput.h>
#include <Inventor/SoPrimitiveVertex.h>
#include <Inventor/actions/SoCallbackAction.h>
#include <Inventor/nodes/SoCamera.h>
#include <Inventor/nodes/SoPerspectiveCamera.h>
#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/nodes/SoShape.h>

TEST(coin3d, basic) {
  widget_t* w = coin3d_create(NULL, 10, 20, 30, 40);
  ASSERT_EQ(w != NULL, true);
  coin3d_t* coin3d = COIN3D(w);
  ASSERT_EQ(coin3d != NULL, true);
  ASSERT_EQ(coin3d->coin != NULL, true);
  ASSERT_EQ(widget_destroy(w), RET_OK);
}

TEST(coin3d, model_and_background_props) {
  widget_t* w = coin3d_create(NULL, 0, 0, 100, 100);
  value_t v;
  float r = 0.0f;
  float g = 0.0f;
  float b = 0.0f;
  ASSERT_EQ(w != NULL, true);

  ASSERT_EQ(coin3d_set_background(w, "#112233"), RET_OK);
  ASSERT_EQ(widget_get_prop(w, COIN3D_PROP_BACKGROUND, &v), RET_OK);
  ASSERT_STREQ(value_str(&v), "#112233");
  ASSERT_EQ(coin3d_coin_get_background_rgb(NULL, &r, &g, &b), RET_BAD_PARAMS);
  ASSERT_EQ(coin3d_coin_get_background_rgb(COIN3D(w)->coin, NULL, &g, &b), RET_BAD_PARAMS);
  ASSERT_EQ(coin3d_coin_get_background_rgb(COIN3D(w)->coin, &r, &g, &b), RET_OK);
  ASSERT_NEAR(r, 0x11 / 255.0f, 0.01f);
  ASSERT_NEAR(g, 0x22 / 255.0f, 0.01f);
  ASSERT_NEAR(b, 0x33 / 255.0f, 0.01f);

  ASSERT_EQ(coin3d_set_model(w, ""), RET_OK);
  ASSERT_EQ(widget_get_prop(w, COIN3D_PROP_MODEL, &v), RET_OK);
  ASSERT_STREQ(value_str(&v), "");

  ASSERT_EQ(widget_destroy(w), RET_OK);
}

static const char* coin3d_ivexamples_models[] = {"bird.iv",
                                                 "desk.iv",
                                                 "dogDish.iv",
                                                 "duck.iv",
                                                 "eatAtJosies.iv",
                                                 "flower.iv",
                                                 "flowerPath.iv",
                                                 "jumpyMan.iv",
                                                 "luxo.iv",
                                                 "monitor.iv",
                                                 "parkbench.iv",
                                                 "star.iv",
                                                 "temple.iv",
                                                 "windmillTower.iv",
                                                 "windmillVanes.iv"};

TEST(coin3d, load_model_from_asset) {
  static const char* models[] = {"cube.iv",
                                 "cone.iv",
                                 "sphere.iv",
                                 "cylinder.iv",
                                 "rotating_cube.iv",
                                 "primitives.iv",
                                 "materials.iv",
                                 "color_cube.iv",
                                 "textured_cube.iv",
                                 "wireframe.iv",
                                 "shuttle.iv",
                                 "pendulum.iv",
                                 "blinker.iv",
                                 "solar.iv",
                                 "robot.iv",
                                 "include_files.iv",
                                 "include_stl.iv",
                                 "ground_grid.iv"};
  widget_t* w = coin3d_create(NULL, 0, 0, 100, 100);
  value_t v;
  uint32_t i = 0;
  ASSERT_EQ(w != NULL, true);

  for (i = 0; i < ARRAY_SIZE(models); i++) {
    ASSERT_EQ(coin3d_set_model(w, models[i]), RET_OK);
    ASSERT_EQ(widget_get_prop(w, COIN3D_PROP_MODEL, &v), RET_OK);
    ASSERT_STREQ(value_str(&v), models[i]);
  }
  for (i = 0; i < ARRAY_SIZE(coin3d_ivexamples_models); i++) {
    ASSERT_EQ(coin3d_set_model(w, coin3d_ivexamples_models[i]), RET_OK);
    ASSERT_EQ(widget_get_prop(w, COIN3D_PROP_MODEL, &v), RET_OK);
    ASSERT_STREQ(value_str(&v), coin3d_ivexamples_models[i]);
  }

  ASSERT_EQ(widget_destroy(w), RET_OK);
}

TEST(coin3d, close_button_is_hit_target) {
  widget_t* win = window_create(NULL, 0, 0, 800, 480);
  widget_t* view = coin3d_create(win, 0, 0, 0, 0);
  widget_t* models = combo_box_create(win, 0, 0, 0, 0);
  widget_t* close = button_create(win, 0, 0, 0, 0);
  widget_t* target = NULL;
  ASSERT_EQ(win != NULL && view != NULL && models != NULL && close != NULL, true);

  widget_set_name(models, "models");
  widget_set_name(close, "close");
  widget_set_self_layout_params(view, "c", "10", "90%", "-100");
  widget_set_self_layout_params(models, "10", "b:50", "280", "30");
  widget_set_self_layout_params(close, "r:10", "b:50", "80", "30");
  widget_layout(win);

  target = widget_find_target(win, close->x + close->w / 2, close->y + close->h / 2);
  ASSERT_EQ(target, close);

  target = widget_find_target(win, models->x + models->w / 2, models->y + models->h / 2);
  ASSERT_EQ(target, models);

  widget_destroy(win);
}

TEST(coin3d, find_model_arg) {
  char* none[] = {(char*)"demo"};
  char* size_only[] = {(char*)"demo", (char*)"800", (char*)"480"};
  char* asset[] = {(char*)"demo", (char*)"robot.iv"};
  char* path[] = {(char*)"demo", (char*)"800", (char*)"design/default/data/solar.iv"};
  char* upper[] = {(char*)"demo", (char*)"Foo.IV"};
  char* stl[] = {(char*)"demo", (char*)"pyramid.stl"};

  ASSERT_EQ(demo_find_model_arg(0, NULL), (const char*)NULL);
  ASSERT_EQ(demo_find_model_arg(1, none), (const char*)NULL);
  ASSERT_EQ(demo_find_model_arg(3, size_only), (const char*)NULL);
  ASSERT_STREQ(demo_find_model_arg(2, asset), "robot.iv");
  ASSERT_STREQ(demo_find_model_arg(3, path), "design/default/data/solar.iv");
  ASSERT_STREQ(demo_find_model_arg(2, upper), "Foo.IV");
  ASSERT_STREQ(demo_find_model_arg(2, stl), "pyramid.stl");
}

TEST(coin3d, demo_pan_buttons_move_object_on_screen) {
  float dx = 0.0f;
  float dy = 0.0f;
  const float step = 0.2f;

  ASSERT_EQ(demo_pan_delta("pan_left", step, &dx, &dy), RET_OK);
  ASSERT_GT(dx, 0.0f);
  ASSERT_FLOAT_EQ(dy, 0.0f);

  ASSERT_EQ(demo_pan_delta("pan_right", step, &dx, &dy), RET_OK);
  ASSERT_LT(dx, 0.0f);
  ASSERT_FLOAT_EQ(dy, 0.0f);

  ASSERT_EQ(demo_pan_delta("pan_up", step, &dx, &dy), RET_OK);
  ASSERT_FLOAT_EQ(dx, 0.0f);
  ASSERT_LT(dy, 0.0f);

  ASSERT_EQ(demo_pan_delta("pan_down", step, &dx, &dy), RET_OK);
  ASSERT_FLOAT_EQ(dx, 0.0f);
  ASSERT_GT(dy, 0.0f);
}

TEST(coin3d, load_model_from_file) {
  char path[MAX_PATH + 1];
  widget_t* w = coin3d_create(NULL, 0, 0, 100, 100);
  value_t v;
  ASSERT_EQ(w != NULL, true);

  tk_snprintf(path, sizeof(path), "%s/design/default/data/cube.iv", APP_ROOT);
  ASSERT_EQ(file_exist(path), true);
  ASSERT_EQ(coin3d_set_model(w, path), RET_OK);
  ASSERT_EQ(widget_get_prop(w, COIN3D_PROP_MODEL, &v), RET_OK);
  ASSERT_STREQ(value_str(&v), path);

  tk_snprintf(path, sizeof(path), "%s/design/default/data/pyramid.stl", APP_ROOT);
  ASSERT_EQ(file_exist(path), true);
  ASSERT_EQ(coin3d_set_model(w, path), RET_OK);
  ASSERT_EQ(widget_get_prop(w, COIN3D_PROP_MODEL, &v), RET_OK);
  ASSERT_STREQ(value_str(&v), path);

  tk_snprintf(path, sizeof(path), "%s/design/default/data/pyramid_binary.stl", APP_ROOT);
  ASSERT_EQ(file_exist(path), true);
  ASSERT_EQ(coin3d_set_model(w, path), RET_OK);

  tk_snprintf(path, sizeof(path), "%s/design/default/data/color_pyramid.stl", APP_ROOT);
  ASSERT_EQ(file_exist(path), true);
  ASSERT_EQ(coin3d_set_model(w, path), RET_OK);

  tk_snprintf(path, sizeof(path), "%s/design/default/data/viscam_pyramid.stl", APP_ROOT);
  ASSERT_EQ(file_exist(path), true);
  ASSERT_EQ(coin3d_set_model(w, path), RET_OK);

  ASSERT_EQ(widget_destroy(w), RET_OK);
}

TEST(coin3d, load_stl_from_asset) {
  static const char* models[] = {"pyramid.stl", "pyramid_binary.stl", "color_pyramid.stl",
                                 "viscam_pyramid.stl"};
  widget_t* w = coin3d_create(NULL, 0, 0, 100, 100);
  value_t v;
  uint32_t i = 0;
  ASSERT_EQ(w != NULL, true);

  for (i = 0; i < ARRAY_SIZE(models); i++) {
    ASSERT_EQ(coin3d_set_model(w, models[i]), RET_OK);
    ASSERT_EQ(widget_get_prop(w, COIN3D_PROP_MODEL, &v), RET_OK);
    ASSERT_STREQ(value_str(&v), models[i]);
  }

  ASSERT_EQ(widget_destroy(w), RET_OK);
}

TEST(coin3d, load_invalid_stl_fallback) {
  widget_t* w = coin3d_create(NULL, 0, 0, 100, 100);
  ASSERT_EQ(w != NULL, true);
  ASSERT_EQ(coin3d_set_model(w, "not_a_valid_model.stl"), RET_OK);
  ASSERT_EQ(widget_destroy(w), RET_OK);
}

TEST(coin3d, load_ivexamples_models_from_file) {
  widget_t* w = coin3d_create(NULL, 0, 0, 100, 100);
  char path[MAX_PATH + 1];
  value_t v;
  uint32_t i = 0;
  ASSERT_EQ(w != NULL, true);

  for (i = 0; i < ARRAY_SIZE(coin3d_ivexamples_models); i++) {
    memset(path, 0, sizeof(path));
    tk_snprintf(path, sizeof(path), "%s/design/default/data/%s", APP_ROOT,
                coin3d_ivexamples_models[i]);
    ASSERT_EQ(file_exist(path), true);
    ASSERT_EQ(coin3d_set_model(w, path), RET_OK);
    ASSERT_EQ(widget_get_prop(w, COIN3D_PROP_MODEL, &v), RET_OK);
    ASSERT_STREQ(value_str(&v), path);
  }

  ASSERT_EQ(widget_destroy(w), RET_OK);
}

static uint32_t coin3d_test_count_face_sets(widget_t* w) {
  coin3d_t* coin3d = COIN3D(w);
  if (coin3d == NULL || coin3d->coin == NULL) {
    return 0;
  }
  return coin3d_coin_count_indexed_face_sets(coin3d->coin);
}

TEST(coin3d, load_include_stl_from_asset) {
  widget_t* w = coin3d_create(NULL, 0, 0, 100, 100);
  value_t v;
  ASSERT_EQ(w != NULL, true);
  ASSERT_EQ(coin3d_set_model(w, "include_stl.iv"), RET_OK);
  ASSERT_EQ(widget_get_prop(w, COIN3D_PROP_MODEL, &v), RET_OK);
  ASSERT_STREQ(value_str(&v), "include_stl.iv");
  ASSERT_GE(coin3d_test_count_face_sets(w), 3u);
  ASSERT_EQ(widget_destroy(w), RET_OK);
}

TEST(coin3d, load_include_stl_resolves_file_nodes) {
  widget_t* w = coin3d_create(NULL, 0, 0, 100, 100);
  char path[MAX_PATH + 1];
  value_t v;
  ASSERT_EQ(w != NULL, true);

  memset(path, 0, sizeof(path));
  tk_snprintf(path, sizeof(path), "%s/design/default/data/include_stl.iv", APP_ROOT);
  ASSERT_EQ(file_exist(path), true);
  ASSERT_EQ(coin3d_set_model(w, path), RET_OK);
  ASSERT_EQ(widget_get_prop(w, COIN3D_PROP_MODEL, &v), RET_OK);
  ASSERT_STREQ(value_str(&v), path);
  ASSERT_GE(coin3d_test_count_face_sets(w), 3u);
  ASSERT_EQ(widget_destroy(w), RET_OK);

  w = coin3d_create(NULL, 0, 0, 100, 100);
  ASSERT_EQ(w != NULL, true);
  ASSERT_EQ(coin3d_set_model(w, "include_stl.iv"), RET_OK);
  ASSERT_EQ(widget_get_prop(w, COIN3D_PROP_MODEL, &v), RET_OK);
  ASSERT_STREQ(value_str(&v), "include_stl.iv");
  ASSERT_GE(coin3d_test_count_face_sets(w), 3u);

  ASSERT_EQ(widget_destroy(w), RET_OK);
}

TEST(coin3d, load_include_files_resolves_file_nodes) {
  widget_t* w = coin3d_create(NULL, 0, 0, 100, 100);
  char path[MAX_PATH + 1];
  ASSERT_EQ(w != NULL, true);

  memset(path, 0, sizeof(path));
  tk_snprintf(path, sizeof(path), "%s/design/default/data/include_files.iv", APP_ROOT);
  ASSERT_EQ(file_exist(path), true);
  ASSERT_EQ(coin3d_set_model(w, path), RET_OK);
  ASSERT_TRUE(coin3d_find_node(w, "included_part") != NULL);

  ASSERT_EQ(coin3d_set_model(w, "include_files.iv"), RET_OK);
  ASSERT_TRUE(coin3d_find_node(w, "included_part") != NULL);

  ASSERT_EQ(widget_destroy(w), RET_OK);
}

TEST(coin3d, load_ground_grid_has_named_nodes) {
  widget_t* w = coin3d_create(NULL, 0, 0, 100, 100);
  char path[MAX_PATH + 1];
  ASSERT_EQ(w != NULL, true);

  memset(path, 0, sizeof(path));
  tk_snprintf(path, sizeof(path), "%s/design/default/data/ground_grid.iv", APP_ROOT);
  ASSERT_EQ(file_exist(path), true);
  ASSERT_EQ(coin3d_set_model(w, path), RET_OK);
  ASSERT_TRUE(coin3d_find_node(w, "ground") != NULL);
  ASSERT_TRUE(coin3d_find_node(w, "box") != NULL);
  ASSERT_TRUE(coin3d_find_node(w, "ball") != NULL);
  ASSERT_TRUE(coin3d_find_node(w, "cone") != NULL);

  ASSERT_EQ(widget_destroy(w), RET_OK);
}

TEST(coin3d, gl_rect_is_widget_not_window) {
  widget_t* win = window_create(NULL, 0, 0, 800, 480);
  widget_t* view = coin3d_create(win, 40, 10, 720, 410);
  int32_t x = 0;
  int32_t y = 0;
  int32_t w = 0;
  int32_t h = 0;
  ASSERT_EQ(win != NULL && view != NULL, true);
  widget_move_resize(win, 0, 0, 800, 480);
  widget_move_resize(view, 40, 10, 720, 410);

  ASSERT_EQ(coin3d_gl_get_widget_gl_rect(NULL, &x, &y, &w, &h), RET_BAD_PARAMS);
  ASSERT_EQ(coin3d_gl_get_widget_gl_rect(view, NULL, &y, &w, &h), RET_BAD_PARAMS);
  ASSERT_EQ(coin3d_gl_get_widget_gl_rect(view, &x, &y, &w, &h), RET_OK);
  ASSERT_EQ(x, 40);
  ASSERT_EQ(y, 60);
  ASSERT_EQ(w, 720);
  ASSERT_EQ(h, 410);
  ASSERT_LT(h, win->h);
  ASSERT_GT(y, 0);

  widget_destroy(win);
}

static void coin3d_view_reset_look_at_origin(SoPerspectiveCamera* cam) {
  cam->position = SbVec3f(0.0f, 0.0f, 5.0f);
  cam->orientation = SbRotation::identity();
  cam->focalDistance = 5.0f;
}

TEST(coin3d, gizmo_prop_default_off) {
  widget_t* w = coin3d_create(NULL, 0, 0, 100, 100);
  value_t v;
  ASSERT_EQ(w != NULL, true);
  ASSERT_EQ(widget_get_prop(w, "gizmo", &v), RET_OK);
  ASSERT_EQ(value_bool(&v), FALSE);
  ASSERT_EQ(widget_destroy(w), RET_OK);
}

TEST(coin3d, gizmo_prop_set_get) {
  widget_t* w = coin3d_create(NULL, 0, 0, 100, 100);
  value_t v;
  ASSERT_EQ(w != NULL, true);

  ASSERT_EQ(widget_set_prop_bool(w, "gizmo", TRUE), RET_OK);
  ASSERT_EQ(widget_get_prop(w, "gizmo", &v), RET_OK);
  ASSERT_EQ(value_bool(&v), TRUE);

  ASSERT_EQ(widget_set_prop_bool(w, "gizmo", FALSE), RET_OK);
  ASSERT_EQ(widget_get_prop(w, "gizmo", &v), RET_OK);
  ASSERT_EQ(value_bool(&v), FALSE);

  ASSERT_EQ(widget_destroy(w), RET_OK);
}

TEST(coin3d, gizmo_calc_rect_top_right) {
  rect_t r;
  ASSERT_EQ(coin3d_gizmo_calc_rect(200, 180, &r), RET_OK);
  ASSERT_EQ(r.x, 200 - COIN3D_GIZMO_SIZE - COIN3D_GIZMO_MARGIN);
  ASSERT_EQ(r.y, COIN3D_GIZMO_MARGIN);
  ASSERT_EQ(r.w, COIN3D_GIZMO_SIZE);
  ASSERT_EQ(r.h, COIN3D_GIZMO_SIZE);
}

TEST(coin3d, gizmo_pick_front_axis_and_disk) {
  widget_t* w = coin3d_create(NULL, 0, 0, 200, 200);
  SoPerspectiveCamera* cam = new SoPerspectiveCamera;
  rect_t r;
  float points_x[6];
  float points_y[6];
  float depth[6];
  float cx = 0;
  float cy = 0;
  ASSERT_EQ(w != NULL, true);

  cam->ref();
  coin3d_view_reset_look_at_origin(cam);
  coin3d_gizmo_calc_rect(200, 200, &r);
  ASSERT_EQ(coin3d_gizmo_project_axes(cam, &r, points_x, points_y, depth), RET_OK);
  cx = (float)r.x + (float)r.w * 0.5f;
  cy = (float)r.y + (float)r.h * 0.5f;
  ASSERT_EQ(coin3d_gizmo_pick_at(&r, points_x, points_y, depth, cx, cy), COIN3D_GIZMO_PICK_POS_Z);
  ASSERT_EQ(coin3d_gizmo_pick_at(&r, points_x, points_y, depth, cx + 20.0f, cy + 20.0f),
            COIN3D_GIZMO_PICK_DISK);

  cam->unref();
  ASSERT_EQ(widget_destroy(w), RET_OK);
}

TEST(coin3d, gizmo_snap_pos_y) {
  widget_t* w = coin3d_create(NULL, 0, 0, 200, 200);
  SoPerspectiveCamera* cam = new SoPerspectiveCamera;
  SbVec3f pos;
  ASSERT_EQ(w != NULL, true);

  cam->ref();
  coin3d_view_reset_look_at_origin(cam);
  ASSERT_EQ(coin3d_gizmo_snap_camera(cam, COIN3D_GIZMO_PICK_POS_Y), RET_OK);
  pos = cam->position.getValue();
  ASSERT_GT(pos[1], 4.0f);
  ASSERT_NEAR(pos[0], 0.0f, 0.05f);
  ASSERT_NEAR(pos[2], 0.0f, 0.05f);

  cam->unref();
  ASSERT_EQ(widget_destroy(w), RET_OK);
}

TEST(coin3d, gl_bad_params) {
  coin3d_gl_t gl;
  memset(&gl, 0, sizeof(gl));

  ASSERT_EQ(coin3d_gl_init(NULL), RET_BAD_PARAMS);
  ASSERT_EQ(coin3d_gl_paint(NULL, NULL, NULL, NULL), RET_BAD_PARAMS);
  ASSERT_EQ(coin3d_gl_deinit(NULL), RET_BAD_PARAMS);
  ASSERT_EQ(coin3d_gl_deinit(&gl), RET_OK);
}

TEST(coin3d, left_drag_orbit_matches_urdf_view) {
  widget_t* w = coin3d_create(NULL, 0, 0, 200, 200);
  SoPerspectiveCamera* cam = new SoPerspectiveCamera;
  Coin3dView view;
  pointer_event_t down;
  pointer_event_t move;
  SbVec3f pos;
  ASSERT_EQ(w != NULL, true);

  cam->ref();
  coin3d_view_reset_look_at_origin(cam);
  view.init(cam, NULL);

  pointer_event_init(&down, EVT_POINTER_DOWN, w, 100, 100);
  down.button = 1;
  ASSERT_EQ(view.onEvent(w, (event_t*)&down), true);
  pointer_event_init(&move, EVT_POINTER_MOVE, w, 100, 140);
  ASSERT_EQ(view.onEvent(w, (event_t*)&move), true);
  pos = cam->position.getValue();
  ASSERT_GT(pos[1], 0.0f);

  coin3d_view_reset_look_at_origin(cam);
  view.init(cam, NULL);
  pointer_event_init(&down, EVT_POINTER_DOWN, w, 100, 100);
  down.button = 1;
  ASSERT_EQ(view.onEvent(w, (event_t*)&down), true);
  pointer_event_init(&move, EVT_POINTER_MOVE, w, 140, 100);
  ASSERT_EQ(view.onEvent(w, (event_t*)&move), true);
  pos = cam->position.getValue();
  ASSERT_LT(pos[0], 0.0f);

  cam->unref();
  ASSERT_EQ(widget_destroy(w), RET_OK);
}

TEST(coin3d, middle_wheel_key_pans) {
  widget_t* w = coin3d_create(NULL, 0, 0, 200, 200);
  key_event_t ke;
  pointer_event_t pe;
  ASSERT_EQ(w != NULL, true);

  key_event_init(&ke, EVT_KEY_DOWN, w, TK_KEY_WHEEL);
  ASSERT_EQ(widget_dispatch(w, (event_t*)&ke), RET_STOP);

  pointer_event_init(&pe, EVT_POINTER_MOVE, w, 80, 40);
  ASSERT_EQ(widget_dispatch(w, (event_t*)&pe), RET_STOP);

  key_event_init(&ke, EVT_KEY_UP, w, TK_KEY_WHEEL);
  ASSERT_EQ(widget_dispatch(w, (event_t*)&ke), RET_STOP);

  pointer_event_init(&pe, EVT_POINTER_MOVE, w, 100, 50);
  ASSERT_NE(widget_dispatch(w, (event_t*)&pe), RET_STOP);

  ASSERT_EQ(widget_destroy(w), RET_OK);
}

static void coin3d_test_parse_xy(const char* str, float* x, float* y) {
  ASSERT_EQ(sscanf(str, "%f,%f", x, y), 2);
}

static void coin3d_test_parse_xyz(const char* str, float* x, float* y, float* z) {
  ASSERT_EQ(sscanf(str, "%f,%f,%f", x, y, z), 3);
}

TEST(coin3d, camera_props_set_get_roundtrip) {
  widget_t* w = coin3d_create(NULL, 0, 0, 100, 100);
  value_t v;
  float x = 0;
  float y = 0;
  float z = 0;
  float pitch = 0;
  float yaw = 0;
  ASSERT_EQ(w != NULL, true);

  ASSERT_EQ(coin3d_set_translation(w, "1.5,2.5,3.5"), RET_OK);
  ASSERT_EQ(widget_get_prop(w, COIN3D_PROP_TRANSLATION, &v), RET_OK);
  coin3d_test_parse_xyz(value_str(&v), &x, &y, &z);
  ASSERT_NEAR(x, 1.5f, 0.01f);
  ASSERT_NEAR(y, 2.5f, 0.01f);
  ASSERT_NEAR(z, 3.5f, 0.01f);

  ASSERT_EQ(widget_set_prop_str(w, COIN3D_PROP_ROTATION, "20,30"), RET_OK);
  ASSERT_EQ(widget_get_prop(w, COIN3D_PROP_ROTATION, &v), RET_OK);
  coin3d_test_parse_xy(value_str(&v), &pitch, &yaw);
  ASSERT_NEAR(pitch, 20.0f, 0.05f);
  ASSERT_NEAR(yaw, 30.0f, 0.05f);

  ASSERT_EQ(coin3d_set_scale(w, 8.0f), RET_OK);
  ASSERT_EQ(widget_get_prop(w, COIN3D_PROP_SCALE, &v), RET_OK);
  ASSERT_NEAR(value_float(&v), 8.0f, 0.01f);

  ASSERT_EQ(widget_destroy(w), RET_OK);
}

TEST(coin3d, camera_props_apply_after_set_model) {
  widget_t* w = coin3d_create(NULL, 0, 0, 100, 100);
  value_t v;
  float x = 0;
  float y = 0;
  float z = 0;
  float pitch = 0;
  float yaw = 0;
  ASSERT_EQ(w != NULL, true);

  ASSERT_EQ(coin3d_set_model(w, "cube.iv"), RET_OK);
  ASSERT_EQ(coin3d_set_translation(w, "1,2,3"), RET_OK);
  ASSERT_EQ(coin3d_set_rotation(w, "15,25"), RET_OK);
  ASSERT_EQ(coin3d_set_scale(w, 7.0f), RET_OK);

  ASSERT_EQ(widget_get_prop(w, COIN3D_PROP_TRANSLATION, &v), RET_OK);
  coin3d_test_parse_xyz(value_str(&v), &x, &y, &z);
  ASSERT_NEAR(x, 1.0f, 0.05f);
  ASSERT_NEAR(y, 2.0f, 0.05f);
  ASSERT_NEAR(z, 3.0f, 0.05f);

  ASSERT_EQ(widget_get_prop(w, COIN3D_PROP_ROTATION, &v), RET_OK);
  coin3d_test_parse_xy(value_str(&v), &pitch, &yaw);
  ASSERT_NEAR(pitch, 15.0f, 0.1f);
  ASSERT_NEAR(yaw, 25.0f, 0.1f);

  ASSERT_EQ(widget_get_prop(w, COIN3D_PROP_SCALE, &v), RET_OK);
  ASSERT_NEAR(value_float(&v), 7.0f, 0.05f);

  ASSERT_EQ(widget_destroy(w), RET_OK);
}

/* Inventor Cube 边长 2；tight viewAll 焦距约 4.2，物体铺满视口。留边后应接近 XML 的 scale=8。 */
#define COIN3D_TEST_CUBE_FRAMED_SCALE_MIN 6.0f
#define COIN3D_TEST_CUBE_FRAMED_SCALE_MAX 12.0f

TEST(coin3d, set_model_frames_cube_with_margin) {
  widget_t* w = coin3d_create(NULL, 0, 0, 400, 300);
  value_t v;
  ASSERT_EQ(w != NULL, true);

  ASSERT_EQ(coin3d_set_model(w, "cube.iv"), RET_OK);
  ASSERT_EQ(widget_get_prop(w, COIN3D_PROP_SCALE, &v), RET_OK);
  ASSERT_GT(value_float(&v), COIN3D_TEST_CUBE_FRAMED_SCALE_MIN);
  ASSERT_LT(value_float(&v), COIN3D_TEST_CUBE_FRAMED_SCALE_MAX);

  ASSERT_EQ(coin3d_set_rotation(w, "20,30"), RET_OK);
  ASSERT_EQ(demo_set_model_keep_orbit(w, "cube.iv"), RET_OK);
  ASSERT_EQ(widget_get_prop(w, COIN3D_PROP_SCALE, &v), RET_OK);
  ASSERT_GT(value_float(&v), COIN3D_TEST_CUBE_FRAMED_SCALE_MIN);
  ASSERT_LT(value_float(&v), COIN3D_TEST_CUBE_FRAMED_SCALE_MAX);

  ASSERT_EQ(widget_destroy(w), RET_OK);
}

TEST(coin3d, demo_set_model_keep_orbit) {
  widget_t* w = coin3d_create(NULL, 0, 0, 400, 300);
  value_t v;
  float pitch = 0.0f;
  float yaw = 0.0f;
  ASSERT_EQ(w != NULL, true);

  ASSERT_EQ(demo_set_model_keep_orbit(NULL, "cube.iv"), RET_BAD_PARAMS);
  ASSERT_EQ(demo_set_model_keep_orbit(w, NULL), RET_BAD_PARAMS);
  ASSERT_EQ(demo_set_model_keep_orbit(w, ""), RET_BAD_PARAMS);

  ASSERT_EQ(coin3d_set_model(w, "cube.iv"), RET_OK);
  ASSERT_EQ(coin3d_set_rotation(w, "20,30"), RET_OK);
  ASSERT_EQ(coin3d_set_scale(w, 8.0f), RET_OK);
  ASSERT_EQ(demo_set_model_keep_orbit(w, "desk.iv"), RET_OK);

  ASSERT_EQ(widget_get_prop(w, COIN3D_PROP_MODEL, &v), RET_OK);
  ASSERT_STREQ(value_str(&v), "desk.iv");
  ASSERT_EQ(widget_get_prop(w, COIN3D_PROP_ROTATION, &v), RET_OK);
  coin3d_test_parse_xy(value_str(&v), &pitch, &yaw);
  ASSERT_NEAR(pitch, 20.0f, 0.1f);
  ASSERT_NEAR(yaw, 30.0f, 0.1f);
  ASSERT_EQ(widget_get_prop(w, COIN3D_PROP_SCALE, &v), RET_OK);
  ASSERT_GT(value_float(&v), 20.0f);

  ASSERT_EQ(widget_destroy(w), RET_OK);
}

TEST(coin3d, demo_capture_displayed_window_bad_params) {
  ASSERT_EQ(demo_capture_displayed_window(NULL), (bitmap_t*)NULL);
}

static void coin3d_test_camera_clip(widget_t* w, float* near_d, float* far_d, float* focal) {
  SoCamera* cam = coin3d_coin_get_camera(COIN3D(w)->coin);
  ASSERT_TRUE(cam != NULL);
  *near_d = cam->nearDistance.getValue();
  *far_d = cam->farDistance.getValue();
  *focal = cam->focalDistance.getValue();
}

TEST(coin3d, desk_iv_is_framed_despite_cube_camera_props) {
  widget_t* w = coin3d_create(NULL, 0, 0, 400, 300);
  float near_d = 0.0f;
  float far_d = 0.0f;
  float focal = 0.0f;
  ASSERT_EQ(w != NULL, true);

  ASSERT_EQ(coin3d_set_translation(w, "0,0,0"), RET_OK);
  ASSERT_EQ(coin3d_set_rotation(w, "20,30"), RET_OK);
  ASSERT_EQ(coin3d_set_scale(w, 8.0f), RET_OK);
  ASSERT_EQ(coin3d_set_model(w, "desk.iv"), RET_OK);

  coin3d_test_camera_clip(w, &near_d, &far_d, &focal);
  ASSERT_GT(focal, 20.0f);
  ASSERT_LT(near_d, focal);
  ASSERT_GT(far_d, focal);

  ASSERT_EQ(widget_destroy(w), RET_OK);
}

TEST(coin3d, desk_iv_zoom_in_does_not_clip) {
  widget_t* w = coin3d_create(NULL, 0, 0, 400, 300);
  float near_d = 0.0f;
  float far_d = 0.0f;
  float focal = 0.0f;
  ASSERT_EQ(w != NULL, true);

  ASSERT_EQ(coin3d_set_model(w, "desk.iv"), RET_OK);
  ASSERT_EQ(coin3d_set_scale(w, 8.0f), RET_OK);

  coin3d_test_camera_clip(w, &near_d, &far_d, &focal);
  ASSERT_NEAR(focal, 8.0f, 0.05f);
  ASSERT_LT(near_d, focal);
  ASSERT_GT(far_d, focal);

  ASSERT_EQ(widget_destroy(w), RET_OK);
}

TEST(coin3d, pan_rotate_zoom_update_props) {
  widget_t* w = coin3d_create(NULL, 0, 0, 100, 100);
  value_t v;
  float x0 = 0;
  float y0 = 0;
  float z0 = 0;
  float x1 = 0;
  float y1 = 0;
  float z1 = 0;
  float pitch0 = 0;
  float yaw0 = 0;
  float pitch1 = 0;
  float yaw1 = 0;
  float scale0 = 0;
  float scale1 = 0;
  ASSERT_EQ(w != NULL, true);

  ASSERT_EQ(coin3d_set_translation(w, "0,0,0"), RET_OK);
  ASSERT_EQ(coin3d_set_rotation(w, "0,0"), RET_OK);
  ASSERT_EQ(coin3d_set_scale(w, 5.0f), RET_OK);

  ASSERT_EQ(widget_get_prop(w, COIN3D_PROP_TRANSLATION, &v), RET_OK);
  coin3d_test_parse_xyz(value_str(&v), &x0, &y0, &z0);
  ASSERT_EQ(coin3d_pan(w, 1.0f, 0.0f), RET_OK);
  ASSERT_EQ(widget_get_prop(w, COIN3D_PROP_TRANSLATION, &v), RET_OK);
  coin3d_test_parse_xyz(value_str(&v), &x1, &y1, &z1);
  ASSERT_GT(x1, x0);

  ASSERT_EQ(coin3d_pan(w, 0.0f, 1.0f), RET_OK);
  ASSERT_EQ(widget_get_prop(w, COIN3D_PROP_TRANSLATION, &v), RET_OK);
  coin3d_test_parse_xyz(value_str(&v), &x1, &y1, &z1);
  ASSERT_GT(y1, y0);

  ASSERT_EQ(widget_get_prop(w, COIN3D_PROP_ROTATION, &v), RET_OK);
  coin3d_test_parse_xy(value_str(&v), &pitch0, &yaw0);
  ASSERT_EQ(coin3d_rotate(w, 10.0f, 0.0f), RET_OK);
  ASSERT_EQ(widget_get_prop(w, COIN3D_PROP_ROTATION, &v), RET_OK);
  coin3d_test_parse_xy(value_str(&v), &pitch1, &yaw1);
  ASSERT_GT(pitch1, pitch0);

  ASSERT_EQ(coin3d_rotate(w, 0.0f, 15.0f), RET_OK);
  ASSERT_EQ(widget_get_prop(w, COIN3D_PROP_ROTATION, &v), RET_OK);
  coin3d_test_parse_xy(value_str(&v), &pitch1, &yaw1);
  ASSERT_GT(yaw1, yaw0);

  ASSERT_EQ(widget_get_prop(w, COIN3D_PROP_SCALE, &v), RET_OK);
  scale0 = value_float(&v);
  ASSERT_EQ(coin3d_zoom(w, 1.0f), RET_OK);
  ASSERT_EQ(widget_get_prop(w, COIN3D_PROP_SCALE, &v), RET_OK);
  scale1 = value_float(&v);
  ASSERT_NEAR(scale1, scale0 + 1.0f, 0.01f);

  ASSERT_EQ(coin3d_zoom(w, -1000.0f), RET_OK);
  ASSERT_EQ(widget_get_prop(w, COIN3D_PROP_SCALE, &v), RET_OK);
  ASSERT_NEAR(value_float(&v), COIN3D_VIEW_MIN_DISTANCE, 0.0001f);

  ASSERT_EQ(widget_destroy(w), RET_OK);
}

TEST(coin3d, camera_api_bad_params) {
  widget_t* w = coin3d_create(NULL, 0, 0, 100, 100);
  ASSERT_EQ(w != NULL, true);

  ASSERT_EQ(coin3d_pan(NULL, 1.0f, 0.0f), RET_BAD_PARAMS);
  ASSERT_EQ(coin3d_rotate(NULL, 1.0f, 0.0f), RET_BAD_PARAMS);
  ASSERT_EQ(coin3d_zoom(NULL, 1.0f), RET_BAD_PARAMS);
  ASSERT_EQ(coin3d_set_translation(NULL, "0,0,0"), RET_BAD_PARAMS);
  ASSERT_EQ(coin3d_set_translation(w, NULL), RET_BAD_PARAMS);
  ASSERT_EQ(coin3d_set_translation(w, "1,2"), RET_BAD_PARAMS);
  ASSERT_EQ(coin3d_set_rotation(NULL, "0,0"), RET_BAD_PARAMS);
  ASSERT_EQ(coin3d_set_rotation(w, NULL), RET_BAD_PARAMS);
  ASSERT_EQ(coin3d_set_rotation(w, "10"), RET_BAD_PARAMS);
  ASSERT_EQ(coin3d_set_scale(NULL, 1.0f), RET_BAD_PARAMS);

  ASSERT_EQ(widget_destroy(w), RET_OK);
}

TEST(coin3d, transform_buttons_are_hit_targets) {
  widget_t* win = window_create(NULL, 0, 0, 800, 480);
  widget_t* view = coin3d_create(win, 0, 0, 0, 0);
  widget_t* models = combo_box_create(win, 0, 0, 0, 0);
  widget_t* close = button_create(win, 0, 0, 0, 0);
  widget_t* pan_left = button_create(win, 0, 0, 0, 0);
  widget_t* rot_x_pos = button_create(win, 0, 0, 0, 0);
  widget_t* zoom_in = button_create(win, 0, 0, 0, 0);
  widget_t* target = NULL;
  ASSERT_EQ(win != NULL && view != NULL && models != NULL && close != NULL, true);
  ASSERT_EQ(pan_left != NULL && rot_x_pos != NULL && zoom_in != NULL, true);

  widget_set_name(models, "models");
  widget_set_name(close, "close");
  widget_set_name(pan_left, "pan_left");
  widget_set_name(rot_x_pos, "rot_x_pos");
  widget_set_name(zoom_in, "zoom_in");
  widget_set_self_layout_params(view, "c", "10", "90%", "-100");
  widget_set_self_layout_params(models, "10", "b:50", "280", "30");
  widget_set_self_layout_params(close, "r:10", "b:50", "80", "30");
  widget_set_self_layout_params(pan_left, "10", "b:10", "50", "30");
  widget_set_self_layout_params(rot_x_pos, "220", "b:10", "60", "30");
  widget_set_self_layout_params(zoom_in, "470", "b:10", "60", "30");
  widget_layout(win);

  target = widget_find_target(win, close->x + close->w / 2, close->y + close->h / 2);
  ASSERT_EQ(target, close);
  target = widget_find_target(win, pan_left->x + pan_left->w / 2, pan_left->y + pan_left->h / 2);
  ASSERT_EQ(target, pan_left);
  target = widget_find_target(win, rot_x_pos->x + rot_x_pos->w / 2, rot_x_pos->y + rot_x_pos->h / 2);
  ASSERT_EQ(target, rot_x_pos);
  target = widget_find_target(win, zoom_in->x + zoom_in->w / 2, zoom_in->y + zoom_in->h / 2);
  ASSERT_EQ(target, zoom_in);

  widget_destroy(win);
}

static ret_t coin3d_test_load_named_nodes(widget_t* w) {
  char path[MAX_PATH + 1];
  memset(path, 0, sizeof(path));
  tk_snprintf(path, sizeof(path), "%s/design/default/data/named_nodes.iv", APP_ROOT);
  return coin3d_set_model(w, path);
}

TEST(coin3d, find_node_by_def_name) {
  widget_t* w = coin3d_create(NULL, 0, 0, 100, 100);
  ASSERT_EQ(w != NULL, true);
  ASSERT_EQ(coin3d_test_load_named_nodes(w), RET_OK);

  ASSERT_TRUE(coin3d_find_node(w, "box") != NULL);
  ASSERT_TRUE(coin3d_find_node(w, "ball") != NULL);
  ASSERT_TRUE(coin3d_find_node(w, "mover") != NULL);
  ASSERT_TRUE(coin3d_find_node(w, "missing") == NULL);
  ASSERT_TRUE(coin3d_find_node(NULL, "box") == NULL);
  ASSERT_TRUE(coin3d_find_node(w, NULL) == NULL);
  ASSERT_TRUE(coin3d_find_node(w, "") == NULL);

  ASSERT_EQ(widget_destroy(w), RET_OK);
}

TEST(coin3d, node_move_sets_translation) {
  widget_t* w = coin3d_create(NULL, 0, 0, 100, 100);
  float x = 0.0f;
  float y = 0.0f;
  float z = 0.0f;
  ASSERT_EQ(w != NULL, true);
  ASSERT_EQ(coin3d_test_load_named_nodes(w), RET_OK);

  ASSERT_EQ(coin3d_node_move(w, "box", 1.5f, 2.5f, 3.5f), RET_OK);
  ASSERT_EQ(coin3d_node_get_translation(w, "box", &x, &y, &z), RET_OK);
  ASSERT_NEAR(x, 1.5f, 0.001f);
  ASSERT_NEAR(y, 2.5f, 0.001f);
  ASSERT_NEAR(z, 3.5f, 0.001f);

  ASSERT_EQ(coin3d_node_move(w, "mover", -1.0f, 0.5f, 4.0f), RET_OK);
  ASSERT_EQ(coin3d_node_get_translation(w, "mover", &x, &y, &z), RET_OK);
  ASSERT_NEAR(x, -1.0f, 0.001f);
  ASSERT_NEAR(y, 0.5f, 0.001f);
  ASSERT_NEAR(z, 4.0f, 0.001f);

  ASSERT_EQ(coin3d_node_move(w, "ball", 0.25f, -0.5f, 1.0f), RET_OK);
  ASSERT_EQ(coin3d_node_get_translation(w, "ball", &x, &y, &z), RET_OK);
  ASSERT_NEAR(x, 0.25f, 0.001f);
  ASSERT_NEAR(y, -0.5f, 0.001f);
  ASSERT_NEAR(z, 1.0f, 0.001f);

  ASSERT_EQ(widget_destroy(w), RET_OK);
}

TEST(coin3d, node_rotate_sets_xyz_degrees) {
  widget_t* w = coin3d_create(NULL, 0, 0, 100, 100);
  float x = 0.0f;
  float y = 0.0f;
  float z = 0.0f;
  ASSERT_EQ(w != NULL, true);
  ASSERT_EQ(coin3d_test_load_named_nodes(w), RET_OK);

  ASSERT_EQ(coin3d_node_rotate(w, "box", 90.0f, 0.0f, 0.0f), RET_OK);
  ASSERT_EQ(coin3d_node_get_rotation(w, "box", &x, &y, &z), RET_OK);
  ASSERT_NEAR(x, 90.0f, 0.1f);
  ASSERT_NEAR(y, 0.0f, 0.1f);
  ASSERT_NEAR(z, 0.0f, 0.1f);

  ASSERT_EQ(coin3d_node_rotate(w, "box", 0.0f, 45.0f, 0.0f), RET_OK);
  ASSERT_EQ(coin3d_node_get_rotation(w, "box", &x, &y, &z), RET_OK);
  ASSERT_NEAR(x, 0.0f, 0.1f);
  ASSERT_NEAR(y, 45.0f, 0.1f);
  ASSERT_NEAR(z, 0.0f, 0.1f);

  ASSERT_EQ(coin3d_node_rotate(w, "box", 0.0f, 0.0f, -30.0f), RET_OK);
  ASSERT_EQ(coin3d_node_get_rotation(w, "box", &x, &y, &z), RET_OK);
  ASSERT_NEAR(x, 0.0f, 0.1f);
  ASSERT_NEAR(y, 0.0f, 0.1f);
  ASSERT_NEAR(z, -30.0f, 0.1f);

  ASSERT_EQ(widget_destroy(w), RET_OK);
}

TEST(coin3d, node_resize_sets_scale) {
  widget_t* w = coin3d_create(NULL, 0, 0, 100, 100);
  float x = 0.0f;
  float y = 0.0f;
  float z = 0.0f;
  ASSERT_EQ(w != NULL, true);
  ASSERT_EQ(coin3d_test_load_named_nodes(w), RET_OK);

  ASSERT_EQ(coin3d_node_resize(w, "box", 2.0f, 0.5f, 3.0f), RET_OK);
  ASSERT_EQ(coin3d_node_get_scale(w, "box", &x, &y, &z), RET_OK);
  ASSERT_NEAR(x, 2.0f, 0.001f);
  ASSERT_NEAR(y, 0.5f, 0.001f);
  ASSERT_NEAR(z, 3.0f, 0.001f);

  ASSERT_EQ(widget_destroy(w), RET_OK);
}

#define PARKBENCH_CREST_FRONT_NZ 0.5f
#define PARKBENCH_CREST_FRONT_Z -11.6f
#define PARKBENCH_CREST_DEFECT_X 12.0f
#define PARKBENCH_CREST_DEFECT_Y 17.5f
#define PARKBENCH_CREST_INNER_X 4.8f
#define PARKBENCH_CREST_INNER_Y 18.5f
#define PARKBENCH_CREST_LEFT_X -15.0f
#define PARKBENCH_CREST_LEFT_Y 17.0f

typedef struct _parkbench_tri_t {
  float x[3];
  float y[3];
  float z[3];
  float geom_nz;
  float nrm_nz;
} parkbench_tri_t;

static void parkbench_collect_triangle(void* userdata, SoCallbackAction* action,
                                       const SoPrimitiveVertex* v1, const SoPrimitiveVertex* v2,
                                       const SoPrimitiveVertex* v3) {
  std::vector<parkbench_tri_t>* tris = (std::vector<parkbench_tri_t>*)userdata;
  parkbench_tri_t t;
  SbVec3f p1;
  SbVec3f p2;
  SbVec3f p3;
  SbVec3f e1;
  SbVec3f e2;
  SbVec3f n;
  (void)action;

  p1 = v1->getPoint();
  p2 = v2->getPoint();
  p3 = v3->getPoint();
  t.x[0] = p1[0];
  t.y[0] = p1[1];
  t.z[0] = p1[2];
  t.x[1] = p2[0];
  t.y[1] = p2[1];
  t.z[1] = p2[2];
  t.x[2] = p3[0];
  t.y[2] = p3[1];
  t.z[2] = p3[2];
  e1 = p2 - p1;
  e2 = p3 - p1;
  n = e1.cross(e2);
  n.normalize();
  t.geom_nz = n[2];
  t.nrm_nz = (v1->getNormal()[2] + v2->getNormal()[2] + v3->getNormal()[2]) / 3.0f;
  tris->push_back(t);
}

static float parkbench_orient_xy(float ax, float ay, float bx, float by, float px, float py) {
  return (bx - ax) * (py - ay) - (px - ax) * (by - ay);
}

static bool_t parkbench_point_in_tri_xy(const parkbench_tri_t* t, float x, float y) {
  float d1 = 0.0f;
  float d2 = 0.0f;
  float d3 = 0.0f;
  bool_t has_neg = FALSE;
  bool_t has_pos = FALSE;

  d1 = parkbench_orient_xy(t->x[0], t->y[0], t->x[1], t->y[1], x, y);
  d2 = parkbench_orient_xy(t->x[1], t->y[1], t->x[2], t->y[2], x, y);
  d3 = parkbench_orient_xy(t->x[2], t->y[2], t->x[0], t->y[0], x, y);
  has_neg = (d1 < 0.0f) || (d2 < 0.0f) || (d3 < 0.0f);
  has_pos = (d1 > 0.0f) || (d2 > 0.0f) || (d3 > 0.0f);
  return (has_neg && has_pos) ? FALSE : TRUE;
}

static bool_t parkbench_crest_front_covers(const std::vector<parkbench_tri_t>& tris, float x,
                                           float y) {
  uint32_t i = 0;
  for (i = 0; i < tris.size(); i++) {
    const parkbench_tri_t* t = &tris[i];
    float cz = (t->z[0] + t->z[1] + t->z[2]) / 3.0f;
    if (t->geom_nz <= PARKBENCH_CREST_FRONT_NZ) {
      continue;
    }
    if (t->nrm_nz <= PARKBENCH_CREST_FRONT_NZ) {
      continue;
    }
    if (cz <= PARKBENCH_CREST_FRONT_Z) {
      continue;
    }
    if (parkbench_point_in_tri_xy(t, x, y)) {
      return TRUE;
    }
  }
  return FALSE;
}

TEST(coin3d, parkbench_backrest_crest_front_faces_camera) {
  widget_t* w = coin3d_create(NULL, 0, 0, 100, 100);
  char path[MAX_PATH + 1];
  SoInput in;
  SoSeparator* root = NULL;
  SoCallbackAction action;
  std::vector<parkbench_tri_t> tris;
  ASSERT_EQ(w != NULL, true);

  memset(path, 0, sizeof(path));
  tk_snprintf(path, sizeof(path), "%s/design/default/data/parkbench.iv", APP_ROOT);
  ASSERT_EQ(file_exist(path), true);
  ASSERT_EQ(in.openFile(path), TRUE);
  root = SoDB::readAll(&in);
  ASSERT_EQ(root != NULL, true);
  root->ref();

  action.addTriangleCallback(SoShape::getClassTypeId(), parkbench_collect_triangle, &tris);
  action.apply(root);

  ASSERT_EQ(parkbench_crest_front_covers(tris, PARKBENCH_CREST_DEFECT_X, PARKBENCH_CREST_DEFECT_Y),
            TRUE);
  ASSERT_EQ(parkbench_crest_front_covers(tris, PARKBENCH_CREST_INNER_X, PARKBENCH_CREST_INNER_Y),
            TRUE);
  ASSERT_EQ(parkbench_crest_front_covers(tris, PARKBENCH_CREST_LEFT_X, PARKBENCH_CREST_LEFT_Y),
            TRUE);

  root->unref();
  ASSERT_EQ(widget_destroy(w), RET_OK);
}

TEST(coin3d, node_api_bad_params) {
  widget_t* w = coin3d_create(NULL, 0, 0, 100, 100);
  float x = 0.0f;
  float y = 0.0f;
  float z = 0.0f;
  ASSERT_EQ(w != NULL, true);
  ASSERT_EQ(coin3d_test_load_named_nodes(w), RET_OK);

  ASSERT_EQ(coin3d_node_move(NULL, "box", 1.0f, 0.0f, 0.0f), RET_BAD_PARAMS);
  ASSERT_EQ(coin3d_node_move(w, NULL, 1.0f, 0.0f, 0.0f), RET_BAD_PARAMS);
  ASSERT_EQ(coin3d_node_move(w, "", 1.0f, 0.0f, 0.0f), RET_BAD_PARAMS);
  ASSERT_EQ(coin3d_node_move(w, "missing", 1.0f, 0.0f, 0.0f), RET_NOT_FOUND);
  ASSERT_EQ(coin3d_node_rotate(w, "missing", 1.0f, 0.0f, 0.0f), RET_NOT_FOUND);
  ASSERT_EQ(coin3d_node_resize(w, "missing", 1.0f, 1.0f, 1.0f), RET_NOT_FOUND);
  ASSERT_EQ(coin3d_node_get_translation(w, "missing", &x, &y, &z), RET_NOT_FOUND);
  ASSERT_EQ(coin3d_node_get_translation(w, "box", NULL, &y, &z), RET_BAD_PARAMS);

  ASSERT_EQ(widget_destroy(w), RET_OK);
}
