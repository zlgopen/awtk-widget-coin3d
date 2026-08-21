/**
 * File:   coin3d.cpp
 * Author: AWTK Develop Team
 * Brief:  awtk widget coin3d
 *
 * Copyright (c) 2026 - 2026 Guangzhou ZHIYUAN Electronics Co.,Ltd.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * License file for more details.
 *
 */

/**
 * History:
 * ================================================================
 * 2026-08-16 Li XianJing <xianjimli@hotmail.com> created
 * 2026-08-17 Li XianJing <xianjimli@hotmail.com> integrate Coin3D
 * 2026-08-18 Li XianJing <xianjimli@hotmail.com> camera pan/rotate/zoom props
 * 2026-08-18 Li XianJing <xianjimli@hotmail.com> node find/move/rotate/resize
 * 2026-08-19 Li XianJing <xianjimli@hotmail.com> set_model viewAll without camera props
 *
 */

#include "tkc/mem.h"
#include "tkc/utils.h"
#include "coin3d.h"
#include "coin3d_gizmo.hpp"
#include "coin3d_view.hpp"

#define COIN3D_SENSOR_TIMER_MS 16
#define COIN3D_VEC3_COUNT 3
#define COIN3D_VEC2_COUNT 2

static Coin3dView* coin3d_get_view(coin3d_t* coin3d) {
  return_value_if_fail(coin3d != NULL && coin3d->coin != NULL, NULL);
  return coin3d_coin_get_view(coin3d->coin);
}

static ret_t coin3d_parse_vec3(const char* str, float* x, float* y, float* z) {
  return_value_if_fail(str != NULL && x != NULL && y != NULL && z != NULL, RET_BAD_PARAMS);
  if (tk_sscanf(str, "%f,%f,%f", x, y, z) != COIN3D_VEC3_COUNT) {
    return RET_BAD_PARAMS;
  }
  return RET_OK;
}

static ret_t coin3d_parse_vec2(const char* str, float* x, float* y) {
  return_value_if_fail(str != NULL && x != NULL && y != NULL, RET_BAD_PARAMS);
  if (tk_sscanf(str, "%f,%f", x, y) != COIN3D_VEC2_COUNT) {
    return RET_BAD_PARAMS;
  }
  return RET_OK;
}

static ret_t coin3d_on_sensor_timer(const timer_info_t* info) {
  coin3d_t* coin3d = COIN3D(info->ctx);
  return_value_if_fail(coin3d != NULL && coin3d->coin != NULL, RET_REMOVE);

  if (coin3d_coin_process_sensors(coin3d->coin)) {
    widget_invalidate_force(WIDGET(coin3d), NULL);
  }

  return RET_REPEAT;
}

static ret_t coin3d_get_prop(widget_t* widget, const char* name, value_t* v) {
  coin3d_t* coin3d = COIN3D(widget);
  Coin3dView* view = NULL;
  float x = 0.0f;
  float y = 0.0f;
  float z = 0.0f;
  float pitch = 0.0f;
  float yaw = 0.0f;
  return_value_if_fail(coin3d != NULL && name != NULL && v != NULL, RET_BAD_PARAMS);

  if (tk_str_eq(name, COIN3D_PROP_MODEL)) {
    value_set_str(v, coin3d->model);
    return RET_OK;
  } else if (tk_str_eq(name, COIN3D_PROP_BACKGROUND)) {
    value_set_str(v, coin3d->background);
    return RET_OK;
  } else if (tk_str_eq(name, COIN3D_PROP_GIZMO)) {
    value_set_bool(v, coin3d->gizmo);
    return RET_OK;
  }

  view = coin3d_get_view(coin3d);
  return_value_if_fail(view != NULL, RET_BAD_PARAMS);

  if (tk_str_eq(name, COIN3D_PROP_TRANSLATION)) {
    view->getLookAt(&x, &y, &z);
    tk_snprintf(coin3d->translation, sizeof(coin3d->translation), "%g,%g,%g", x, y, z);
    value_set_str(v, coin3d->translation);
    return RET_OK;
  } else if (tk_str_eq(name, COIN3D_PROP_ROTATION)) {
    view->getOrbitDegrees(&pitch, &yaw);
    tk_snprintf(coin3d->rotation, sizeof(coin3d->rotation), "%g,%g", pitch, yaw);
    value_set_str(v, coin3d->rotation);
    return RET_OK;
  } else if (tk_str_eq(name, COIN3D_PROP_SCALE)) {
    value_set_float(v, view->getDistance());
    return RET_OK;
  }

  return RET_NOT_FOUND;
}

static ret_t coin3d_set_prop(widget_t* widget, const char* name, const value_t* v) {
  return_value_if_fail(COIN3D(widget) != NULL && name != NULL && v != NULL, RET_BAD_PARAMS);

  if (tk_str_eq(name, COIN3D_PROP_MODEL)) {
    return coin3d_set_model(widget, value_str(v));
  } else if (tk_str_eq(name, COIN3D_PROP_BACKGROUND)) {
    return coin3d_set_background(widget, value_str(v));
  } else if (tk_str_eq(name, COIN3D_PROP_GIZMO)) {
    return coin3d_set_gizmo(widget, value_bool(v));
  } else if (tk_str_eq(name, COIN3D_PROP_TRANSLATION)) {
    return coin3d_set_translation(widget, value_str(v));
  } else if (tk_str_eq(name, COIN3D_PROP_ROTATION)) {
    return coin3d_set_rotation(widget, value_str(v));
  } else if (tk_str_eq(name, COIN3D_PROP_SCALE)) {
    return coin3d_set_scale(widget, value_float(v));
  }

  return RET_NOT_FOUND;
}

static ret_t coin3d_on_destroy(widget_t* widget) {
  coin3d_t* coin3d = COIN3D(widget);
  return_value_if_fail(widget != NULL && coin3d != NULL, RET_BAD_PARAMS);

  if (coin3d->timer_id != TK_INVALID_ID) {
    timer_remove(coin3d->timer_id);
    coin3d->timer_id = TK_INVALID_ID;
  }

  coin3d_gl_deinit(&coin3d->gl);
  if (coin3d->coin != NULL) {
    coin3d_coin_destroy(coin3d->coin);
    coin3d->coin = NULL;
  }
  TKMEM_FREE(coin3d->model);
  TKMEM_FREE(coin3d->background);

  return RET_OK;
}

static ret_t coin3d_on_paint_self(widget_t* widget, canvas_t* c) {
  coin3d_t* coin3d = COIN3D(widget);
  return_value_if_fail(coin3d != NULL && coin3d->coin != NULL, RET_BAD_PARAMS);

  return_value_if_fail(coin3d_gl_paint(&coin3d->gl, coin3d->coin, widget, c) == RET_OK, RET_FAIL);
  if (coin3d->gizmo) {
    return coin3d_gizmo_draw(&coin3d->gizmo_state, widget, c, coin3d->coin);
  }
  return RET_OK;
}

static ret_t coin3d_on_event(widget_t* widget, event_t* e) {
  coin3d_t* coin3d = COIN3D(widget);
  return_value_if_fail(widget != NULL && coin3d != NULL && coin3d->coin != NULL, RET_BAD_PARAMS);

  if (coin3d->gizmo &&
      coin3d_gizmo_on_event(&coin3d->gizmo_state, widget, e, coin3d->coin)) {
    widget_invalidate_force(widget, NULL);
    return RET_STOP;
  }

  if (coin3d_coin_handle_pointer(coin3d->coin, widget, e) == RET_OK) {
    widget_invalidate_force(widget, NULL);
    return RET_STOP;
  }

  return RET_OK;
}

const char* s_coin3d_properties[] = {COIN3D_PROP_MODEL,
                                     COIN3D_PROP_BACKGROUND,
                                     COIN3D_PROP_GIZMO,
                                     COIN3D_PROP_TRANSLATION,
                                     COIN3D_PROP_ROTATION,
                                     COIN3D_PROP_SCALE,
                                     NULL};

/* MSVC C++17 does not support designated initializers; fill fields explicitly. */
static widget_vtable_t coin3d_make_vtable(void) {
  widget_vtable_t vt;
  memset(&vt, 0, sizeof(vt));
  vt.size = sizeof(coin3d_t);
  vt.type = WIDGET_TYPE_COIN3D;
  vt.clone_properties = s_coin3d_properties;
  vt.persistent_properties = s_coin3d_properties;
  vt.parent = NULL;
  vt.get_parent_vt = TK_GET_PARENT_VTABLE(widget);
  vt.create = coin3d_create;
  vt.get_prop = coin3d_get_prop;
  vt.set_prop = coin3d_set_prop;
  vt.on_paint_self = coin3d_on_paint_self;
  vt.on_event = coin3d_on_event;
  vt.on_destroy = coin3d_on_destroy;
  return vt;
}

TK_DECL_VTABLE(coin3d) = coin3d_make_vtable();

ret_t coin3d_set_model(widget_t* widget, const char* model) {
  coin3d_t* coin3d = COIN3D(widget);
  return_value_if_fail(coin3d != NULL && coin3d->coin != NULL, RET_BAD_PARAMS);

  TKMEM_FREE(coin3d->model);
  coin3d->model = tk_strdup(model != NULL ? model : "");
  return_value_if_fail(coin3d_coin_load_model(coin3d->coin, coin3d->model) == RET_OK, RET_FAIL);
  widget_invalidate_force(widget, NULL);
  return RET_OK;
}

ret_t coin3d_set_background(widget_t* widget, const char* background) {
  coin3d_t* coin3d = COIN3D(widget);
  return_value_if_fail(coin3d != NULL && coin3d->coin != NULL, RET_BAD_PARAMS);

  TKMEM_FREE(coin3d->background);
  coin3d->background = tk_strdup(background != NULL ? background : "");
  return_value_if_fail(coin3d_coin_set_background(coin3d->coin, coin3d->background) == RET_OK,
                       RET_FAIL);
  widget_invalidate_force(widget, NULL);
  return RET_OK;
}

ret_t coin3d_set_gizmo(widget_t* widget, bool_t gizmo) {
  coin3d_t* coin3d = COIN3D(widget);
  return_value_if_fail(coin3d != NULL, RET_BAD_PARAMS);

  coin3d->gizmo = gizmo;
  coin3d->gizmo_state.visible = gizmo;
  widget_invalidate_force(widget, NULL);
  return RET_OK;
}

ret_t coin3d_set_translation(widget_t* widget, const char* translation) {
  coin3d_t* coin3d = COIN3D(widget);
  Coin3dView* view = coin3d_get_view(coin3d);
  float x = 0.0f;
  float y = 0.0f;
  float z = 0.0f;
  return_value_if_fail(coin3d != NULL && view != NULL && view->camera() != NULL, RET_BAD_PARAMS);
  return_value_if_fail(coin3d_parse_vec3(translation, &x, &y, &z) == RET_OK, RET_BAD_PARAMS);

  view->setLookAt(x, y, z);
  widget_invalidate_force(widget, NULL);
  return RET_OK;
}

ret_t coin3d_set_rotation(widget_t* widget, const char* rotation) {
  coin3d_t* coin3d = COIN3D(widget);
  Coin3dView* view = coin3d_get_view(coin3d);
  float pitch = 0.0f;
  float yaw = 0.0f;
  return_value_if_fail(coin3d != NULL && view != NULL && view->camera() != NULL, RET_BAD_PARAMS);
  return_value_if_fail(coin3d_parse_vec2(rotation, &pitch, &yaw) == RET_OK, RET_BAD_PARAMS);

  view->setOrbitDegrees(pitch, yaw);
  widget_invalidate_force(widget, NULL);
  return RET_OK;
}

ret_t coin3d_set_scale(widget_t* widget, float scale) {
  coin3d_t* coin3d = COIN3D(widget);
  Coin3dView* view = coin3d_get_view(coin3d);
  return_value_if_fail(coin3d != NULL && view != NULL && view->camera() != NULL, RET_BAD_PARAMS);

  coin3d->scale = scale;
  view->setDistance(scale);
  widget_invalidate_force(widget, NULL);
  return RET_OK;
}

ret_t coin3d_pan(widget_t* widget, float dx, float dy) {
  coin3d_t* coin3d = COIN3D(widget);
  Coin3dView* view = coin3d_get_view(coin3d);
  return_value_if_fail(coin3d != NULL && view != NULL && view->camera() != NULL, RET_BAD_PARAMS);

  view->panBy(dx, dy);
  widget_invalidate_force(widget, NULL);
  return RET_OK;
}

ret_t coin3d_rotate(widget_t* widget, float dx, float dy) {
  coin3d_t* coin3d = COIN3D(widget);
  Coin3dView* view = coin3d_get_view(coin3d);
  return_value_if_fail(coin3d != NULL && view != NULL && view->camera() != NULL, RET_BAD_PARAMS);

  view->rotateByDegrees(dx, dy);
  widget_invalidate_force(widget, NULL);
  return RET_OK;
}

ret_t coin3d_zoom(widget_t* widget, float delta) {
  coin3d_t* coin3d = COIN3D(widget);
  Coin3dView* view = coin3d_get_view(coin3d);
  return_value_if_fail(coin3d != NULL && view != NULL && view->camera() != NULL, RET_BAD_PARAMS);

  view->zoomByDistance(delta);
  widget_invalidate_force(widget, NULL);
  return RET_OK;
}

static ret_t coin3d_after_node_change(widget_t* widget, ret_t ret) {
  if (ret == RET_OK) {
    widget_invalidate_force(widget, NULL);
  }
  return ret;
}

void* coin3d_find_node(widget_t* widget, const char* name) {
  coin3d_t* coin3d = COIN3D(widget);
  return_value_if_fail(coin3d != NULL && coin3d->coin != NULL, NULL);
  return coin3d_coin_find_node(coin3d->coin, name);
}

ret_t coin3d_node_move(widget_t* widget, const char* name, float x, float y, float z) {
  coin3d_t* coin3d = COIN3D(widget);
  return_value_if_fail(coin3d != NULL && coin3d->coin != NULL, RET_BAD_PARAMS);
  return coin3d_after_node_change(widget, coin3d_coin_node_move(coin3d->coin, name, x, y, z));
}

ret_t coin3d_node_rotate(widget_t* widget, const char* name, float x, float y, float z) {
  coin3d_t* coin3d = COIN3D(widget);
  return_value_if_fail(coin3d != NULL && coin3d->coin != NULL, RET_BAD_PARAMS);
  return coin3d_after_node_change(widget, coin3d_coin_node_rotate(coin3d->coin, name, x, y, z));
}

ret_t coin3d_node_resize(widget_t* widget, const char* name, float x, float y, float z) {
  coin3d_t* coin3d = COIN3D(widget);
  return_value_if_fail(coin3d != NULL && coin3d->coin != NULL, RET_BAD_PARAMS);
  return coin3d_after_node_change(widget, coin3d_coin_node_resize(coin3d->coin, name, x, y, z));
}

ret_t coin3d_node_get_translation(widget_t* widget, const char* name, float* x, float* y, float* z) {
  coin3d_t* coin3d = COIN3D(widget);
  return_value_if_fail(coin3d != NULL && coin3d->coin != NULL, RET_BAD_PARAMS);
  return coin3d_coin_node_get_translation(coin3d->coin, name, x, y, z);
}

ret_t coin3d_node_get_rotation(widget_t* widget, const char* name, float* x, float* y, float* z) {
  coin3d_t* coin3d = COIN3D(widget);
  return_value_if_fail(coin3d != NULL && coin3d->coin != NULL, RET_BAD_PARAMS);
  return coin3d_coin_node_get_rotation(coin3d->coin, name, x, y, z);
}

ret_t coin3d_node_get_scale(widget_t* widget, const char* name, float* x, float* y, float* z) {
  coin3d_t* coin3d = COIN3D(widget);
  return_value_if_fail(coin3d != NULL && coin3d->coin != NULL, RET_BAD_PARAMS);
  return coin3d_coin_node_get_scale(coin3d->coin, name, x, y, z);
}

widget_t* coin3d_create(widget_t* parent, xy_t x, xy_t y, wh_t w, wh_t h) {
  widget_t* widget = NULL;
  coin3d_t* coin3d = NULL;
  widget = widget_create(parent, TK_REF_VTABLE(coin3d), x, y, w, h);
  coin3d = COIN3D(widget);
  return_value_if_fail(coin3d != NULL, NULL);

  memset(&coin3d->gl, 0x00, sizeof(coin3d->gl));
  coin3d->model = NULL;
  coin3d->background = NULL;
  coin3d->gizmo = FALSE;
  memset(coin3d->translation, 0x00, sizeof(coin3d->translation));
  memset(coin3d->rotation, 0x00, sizeof(coin3d->rotation));
  coin3d->scale = 0.0f;
  coin3d_gizmo_init(&coin3d->gizmo_state);
  coin3d->timer_id = TK_INVALID_ID;
  coin3d->coin = coin3d_coin_create();
  if (coin3d->coin == NULL) {
    widget_destroy(widget);
    return NULL;
  }

  coin3d_coin_ensure_ready(coin3d->coin);
  coin3d->timer_id = widget_add_timer(widget, coin3d_on_sensor_timer, COIN3D_SENSOR_TIMER_MS);
  widget_set_focusable(widget, TRUE);
  widget_set_prop_bool(widget, WIDGET_PROP_FEEDBACK, FALSE);

  return widget;
}

widget_t* coin3d_cast(widget_t* widget) {
  return_value_if_fail(WIDGET_IS_INSTANCE_OF(widget, coin3d), NULL);

  return widget;
}
