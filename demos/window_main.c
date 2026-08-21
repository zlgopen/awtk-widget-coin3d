#include "awtk.h"
#include "coin3d_register.h"
#include "coin3d/coin3d.h"
#include "demo_model_arg.h"
#include "demo_screenshot.h"
#include "demo_set_model.h"
#include "demo_transform.h"

#define DEMO_PAN_STEP 0.2f
#define DEMO_ROTATE_STEP 15.0f
#define DEMO_ZOOM_STEP 0.5f
#define DEMO_SCREENSHOT_DELAY_MS 400

static char s_cmd_model[MAX_PATH + 1];
static char s_screenshot_path[MAX_PATH + 1];

static ret_t on_close(void* ctx, event_t* e) {
  tk_quit();

  return RET_OK;
}

static ret_t on_model_changed(void* ctx, event_t* e) {
  widget_t* win = WIDGET(ctx);
  widget_t* models = WIDGET(e->target);
  widget_t* coin3d = widget_lookup(win, "coin3d", TRUE);
  const char* name = combo_box_get_text(models);

  return_value_if_fail(coin3d != NULL && name != NULL && name[0] != '\0', RET_BAD_PARAMS);

  return demo_set_model_keep_orbit(coin3d, name);
}

static widget_t* demo_lookup_coin3d(void* ctx) {
  widget_t* win = WIDGET(ctx);
  return_value_if_fail(win != NULL, NULL);
  return widget_lookup(win, "coin3d", TRUE);
}

static ret_t on_transform_click(void* ctx, event_t* e) {
  widget_t* coin3d = demo_lookup_coin3d(ctx);
  const char* name = NULL;
  return_value_if_fail(coin3d != NULL && e != NULL && e->target != NULL, RET_BAD_PARAMS);

  name = WIDGET(e->target)->name;
  return_value_if_fail(name != NULL, RET_BAD_PARAMS);

  if (tk_str_eq(name, "pan_left") || tk_str_eq(name, "pan_right") || tk_str_eq(name, "pan_up") ||
      tk_str_eq(name, "pan_down")) {
    float dx = 0.0f;
    float dy = 0.0f;
    return_value_if_fail(demo_pan_delta(name, DEMO_PAN_STEP, &dx, &dy) == RET_OK, RET_FAIL);
    return coin3d_pan(coin3d, dx, dy);
  }
  if (tk_str_eq(name, "rot_x_pos")) {
    return coin3d_rotate(coin3d, DEMO_ROTATE_STEP, 0.0f);
  }
  if (tk_str_eq(name, "rot_x_neg")) {
    return coin3d_rotate(coin3d, -DEMO_ROTATE_STEP, 0.0f);
  }
  if (tk_str_eq(name, "rot_y_pos")) {
    return coin3d_rotate(coin3d, 0.0f, DEMO_ROTATE_STEP);
  }
  if (tk_str_eq(name, "rot_y_neg")) {
    return coin3d_rotate(coin3d, 0.0f, -DEMO_ROTATE_STEP);
  }
  if (tk_str_eq(name, "zoom_in")) {
    return coin3d_zoom(coin3d, -DEMO_ZOOM_STEP);
  }
  if (tk_str_eq(name, "zoom_out")) {
    return coin3d_zoom(coin3d, DEMO_ZOOM_STEP);
  }

  return RET_NOT_FOUND;
}

static ret_t demo_apply_cli_model(widget_t* win, const char* model) {
  char name[MAX_PATH + 1];
  widget_t* coin3d = widget_lookup(win, "coin3d", TRUE);
  widget_t* models = widget_lookup(win, "models", TRUE);

  memset(name, 0, sizeof(name));
  return_value_if_fail(coin3d != NULL && model != NULL && model[0] != '\0', RET_BAD_PARAMS);

  if (models != NULL) {
    path_basename(model, name, sizeof(name));
    if (combo_box_has_option_text(models, name)) {
      combo_box_set_selected_index_by_text(models, name);
    } else {
      if (!combo_box_has_option_text(models, model)) {
        combo_box_append_option(models, combo_box_count_options(models), model);
      }
      combo_box_set_selected_index_by_text(models, model);
    }
  }

  return demo_set_model_keep_orbit(coin3d, model);
}

static ret_t on_screenshot_timer(const timer_info_t* info) {
  widget_t* target = WIDGET(info->ctx);
  bitmap_t* bmp = NULL;
  bool_t ok = FALSE;

  return_value_if_fail(target != NULL && s_screenshot_path[0] != '\0', RET_REMOVE);

  bmp = demo_capture_displayed_window(target);
  if (bmp != NULL) {
    ok = bitmap_save_png(bmp, s_screenshot_path);
    bitmap_destroy(bmp);
  }
  if (!ok) {
    log_error("screenshot failed: %s\n", s_screenshot_path);
  }
  tk_quit();
  return RET_REMOVE;
}

ret_t application_on_cmd_line(int argc, char* argv[]) {
  const char* model = demo_find_model_arg(argc, argv);
  const char* screenshot = demo_find_screenshot_arg(argc, argv);

  memset(s_cmd_model, 0, sizeof(s_cmd_model));
  memset(s_screenshot_path, 0, sizeof(s_screenshot_path));
  if (model != NULL) {
    tk_strncpy(s_cmd_model, model, sizeof(s_cmd_model) - 1);
  }
  if (screenshot != NULL) {
    tk_strncpy(s_screenshot_path, screenshot, sizeof(s_screenshot_path) - 1);
  }

  return RET_OK;
}

/**
 * 初始化
 */
ret_t application_init(void) {
  coin3d_register();

  widget_t* win = window_open("main");
  widget_child_on(win, "close", EVT_CLICK, on_close, NULL);
  widget_child_on(win, "pan_left", EVT_CLICK, on_transform_click, win);
  widget_child_on(win, "pan_right", EVT_CLICK, on_transform_click, win);
  widget_child_on(win, "pan_up", EVT_CLICK, on_transform_click, win);
  widget_child_on(win, "pan_down", EVT_CLICK, on_transform_click, win);
  widget_child_on(win, "rot_x_pos", EVT_CLICK, on_transform_click, win);
  widget_child_on(win, "rot_x_neg", EVT_CLICK, on_transform_click, win);
  widget_child_on(win, "rot_y_pos", EVT_CLICK, on_transform_click, win);
  widget_child_on(win, "rot_y_neg", EVT_CLICK, on_transform_click, win);
  widget_child_on(win, "zoom_in", EVT_CLICK, on_transform_click, win);
  widget_child_on(win, "zoom_out", EVT_CLICK, on_transform_click, win);

  if (s_cmd_model[0] != '\0') {
    demo_apply_cli_model(win, s_cmd_model);
  }

  widget_child_on(win, "models", EVT_VALUE_CHANGED, on_model_changed, win);

  if (s_screenshot_path[0] != '\0') {
    timer_add(on_screenshot_timer, win, DEMO_SCREENSHOT_DELAY_MS);
  }

  return RET_OK;
}

/**
 * 退出
 */
ret_t application_exit(void) {
  log_debug("application_exit\n");
  return RET_OK;
}
