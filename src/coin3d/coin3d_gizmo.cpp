/**
 * File:   coin3d_gizmo.cpp
 * Author: AWTK Develop Team
 * Brief:  View orientation gizmo for coin3d
 *
 * Copyright (c) 2026 - 2026 Guangzhou ZHIYUAN Electronics Co.,Ltd.
 *
 */

/**
 * History:
 * ================================================================
 * 2026-08-18 Li XianJing <xianjimli@hotmail.com> created
 *
 */

#include "coin3d_gizmo.hpp"
#include "coin3d_view.hpp"

#include <cmath>
#include <cstring>

#include <Inventor/SbLinear.h>
#include <Inventor/nodes/SoCamera.h>

#include "tkc/utils.h"
#include "base/vgcanvas.h"

#define COIN3D_GIZMO_AXIS_RADIUS_RATIO 0.36f
#define COIN3D_GIZMO_AXIS_PICK_RATIO 0.18f
#define COIN3D_GIZMO_DISK_RADIUS_RATIO 0.5f
#define COIN3D_GIZMO_DOT_RADIUS_RATIO 0.08f
#define COIN3D_GIZMO_ROTATE_SCALE 0.01f
#define COIN3D_GIZMO_AXIS_COUNT 6
#define COIN3D_GIZMO_FRONT_EPS 0.0f
#define COIN3D_GIZMO_LOOK_ALIGN 0.99f
#define COIN3D_GIZMO_MIN_LEN 1e-6f

static const float s_coin3d_gizmo_axis_dir[COIN3D_GIZMO_AXIS_COUNT][3] = {
    {1.0f, 0.0f, 0.0f},  {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f},
    {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 1.0f},  {0.0f, 0.0f, -1.0f}};

static const coin3d_gizmo_pick_t s_coin3d_gizmo_axis_pick[COIN3D_GIZMO_AXIS_COUNT] = {
    COIN3D_GIZMO_PICK_POS_X, COIN3D_GIZMO_PICK_NEG_X, COIN3D_GIZMO_PICK_POS_Y,
    COIN3D_GIZMO_PICK_NEG_Y, COIN3D_GIZMO_PICK_POS_Z, COIN3D_GIZMO_PICK_NEG_Z};

static widget_t* coin3d_gizmo_grab_host(widget_t* widget) {
  return widget->parent != NULL ? widget->parent : widget;
}

static int32_t coin3d_gizmo_pick_to_axis(coin3d_gizmo_pick_t pick) {
  uint32_t i = 0;
  for (i = 0; i < COIN3D_GIZMO_AXIS_COUNT; i++) {
    if (s_coin3d_gizmo_axis_pick[i] == pick) {
      return (int32_t)i;
    }
  }
  return -1;
}

static color_t coin3d_gizmo_axis_color(uint32_t index) {
  if (index < 2) {
    return color_init(220, 74, 62, 255);
  }
  if (index < 4) {
    return color_init(66, 190, 90, 255);
  }
  return color_init(70, 130, 230, 255);
}

static color_t coin3d_gizmo_color_hovered(color_t c) {
  return color_init((uint8_t)tk_min(255, c.rgba.r + 35), (uint8_t)tk_min(255, c.rgba.g + 35),
                    (uint8_t)tk_min(255, c.rgba.b + 35), 255);
}

static bool_t coin3d_gizmo_label_can_draw(const float* drawn_x, const float* drawn_y,
                                          uint32_t drawn_count, float x, float y, float min_gap) {
  uint32_t i = 0;
  float min_gap_sq = min_gap * min_gap;
  for (i = 0; i < drawn_count; i++) {
    float dx = x - drawn_x[i];
    float dy = y - drawn_y[i];
    if (dx * dx + dy * dy < min_gap_sq) {
      return FALSE;
    }
  }
  return TRUE;
}

static void coin3d_gizmo_to_local(widget_t* widget, float in_x, float in_y, float* out_x,
                                  float* out_y) {
  point_t p = {(xy_t)in_x, (xy_t)in_y};
  widget_to_local(widget, &p);
  *out_x = (float)p.x;
  *out_y = (float)p.y;
}

ret_t coin3d_gizmo_init(coin3d_gizmo_t* gizmo) {
  return_value_if_fail(gizmo != NULL, RET_BAD_PARAMS);
  memset(gizmo, 0x00, sizeof(*gizmo));
  gizmo->visible = FALSE;
  return RET_OK;
}

ret_t coin3d_gizmo_calc_rect(wh_t widget_w, wh_t widget_h, rect_t* r) {
  (void)widget_h;
  return_value_if_fail(r != NULL, RET_BAD_PARAMS);
  r->x = (xy_t)((int32_t)widget_w - (int32_t)COIN3D_GIZMO_SIZE - (int32_t)COIN3D_GIZMO_MARGIN);
  r->y = (xy_t)COIN3D_GIZMO_MARGIN;
  r->w = COIN3D_GIZMO_SIZE;
  r->h = COIN3D_GIZMO_SIZE;
  return RET_OK;
}

coin3d_gizmo_pick_t coin3d_gizmo_pick_at(const rect_t* r, const float* points_x,
                                         const float* points_y, const float* depth, float x,
                                         float y) {
  float radius = 0;
  float best_d2 = 0;
  float cx = 0;
  float cy = 0;
  uint32_t i = 0;
  coin3d_gizmo_pick_t best = COIN3D_GIZMO_PICK_NONE;
  return_value_if_fail(r != NULL && points_x != NULL && points_y != NULL && depth != NULL,
                       COIN3D_GIZMO_PICK_NONE);

  radius = (float)r->w * COIN3D_GIZMO_AXIS_PICK_RATIO;
  best_d2 = radius * radius + 1.0f;
  for (i = 0; i < COIN3D_GIZMO_AXIS_COUNT; i++) {
    float dx = 0;
    float dy = 0;
    float d2 = 0;
    if (depth[i] < COIN3D_GIZMO_FRONT_EPS) {
      continue;
    }
    dx = x - points_x[i];
    dy = y - points_y[i];
    d2 = dx * dx + dy * dy;
    if (d2 <= radius * radius && d2 < best_d2) {
      best_d2 = d2;
      best = s_coin3d_gizmo_axis_pick[i];
    }
  }
  if (best != COIN3D_GIZMO_PICK_NONE) {
    return best;
  }

  cx = (float)r->x + (float)r->w * 0.5f;
  cy = (float)r->y + (float)r->h * 0.5f;
  radius = (float)r->w * COIN3D_GIZMO_DISK_RADIUS_RATIO;
  if ((x - cx) * (x - cx) + (y - cy) * (y - cy) <= radius * radius) {
    return COIN3D_GIZMO_PICK_DISK;
  }
  return COIN3D_GIZMO_PICK_NONE;
}

ret_t coin3d_gizmo_project_axes(SoCamera* camera, const rect_t* r, float* points_x, float* points_y,
                                float* depth) {
  float cx = 0;
  float cy = 0;
  float radius = 0;
  uint32_t i = 0;
  SbRotation inv;
  return_value_if_fail(camera != NULL && r != NULL && points_x != NULL && points_y != NULL &&
                           depth != NULL,
                       RET_BAD_PARAMS);

  cx = (float)r->x + (float)r->w * 0.5f;
  cy = (float)r->y + (float)r->h * 0.5f;
  radius = (float)r->w * COIN3D_GIZMO_AXIS_RADIUS_RATIO;
  inv = camera->orientation.getValue().inverse();
  for (i = 0; i < COIN3D_GIZMO_AXIS_COUNT; i++) {
    SbVec3f world(s_coin3d_gizmo_axis_dir[i][0], s_coin3d_gizmo_axis_dir[i][1],
                  s_coin3d_gizmo_axis_dir[i][2]);
    SbVec3f cam;
    inv.multVec(world, cam);
    points_x[i] = cx + cam[0] * radius;
    points_y[i] = cy - cam[1] * radius;
    depth[i] = cam[2];
  }
  return RET_OK;
}

ret_t coin3d_gizmo_snap_camera(SoCamera* camera, coin3d_gizmo_pick_t pick) {
  int32_t axis = coin3d_gizmo_pick_to_axis(pick);
  SbVec3f look;
  SbVec3f focal;
  SbVec3f dir;
  SbVec3f up(0.0f, 1.0f, 0.0f);
  SbVec3f right;
  SbVec3f z;
  SbMatrix m;
  float dist = 0;
  return_value_if_fail(camera != NULL, RET_BAD_PARAMS);
  return_value_if_fail(axis >= 0, RET_BAD_PARAMS);

  camera->orientation.getValue().multVec(SbVec3f(0, 0, -1), look);
  dist = camera->focalDistance.getValue();
  focal = camera->position.getValue() + dist * look;
  dir.setValue(s_coin3d_gizmo_axis_dir[axis][0], s_coin3d_gizmo_axis_dir[axis][1],
               s_coin3d_gizmo_axis_dir[axis][2]);
  camera->position = focal + dir * dist;

  look = -dir;
  if (fabsf(look.dot(up)) > COIN3D_GIZMO_LOOK_ALIGN) {
    up.setValue(0.0f, 0.0f, 1.0f);
  }
  right = look.cross(up);
  if (right.length() < COIN3D_GIZMO_MIN_LEN) {
    up.setValue(1.0f, 0.0f, 0.0f);
    right = look.cross(up);
  }
  right.normalize();
  up = right.cross(look);
  up.normalize();
  z = -look;
  m.makeIdentity();
  m[0][0] = right[0];
  m[0][1] = right[1];
  m[0][2] = right[2];
  m[1][0] = up[0];
  m[1][1] = up[1];
  m[1][2] = up[2];
  m[2][0] = z[0];
  m[2][1] = z[1];
  m[2][2] = z[2];
  camera->orientation = SbRotation(m);
  return RET_OK;
}

static coin3d_gizmo_pick_t coin3d_gizmo_pick_widget(coin3d_gizmo_t* gizmo, widget_t* widget,
                                                    SoCamera* camera, float x, float y) {
  rect_t r;
  float points_x[COIN3D_GIZMO_AXIS_COUNT];
  float points_y[COIN3D_GIZMO_AXIS_COUNT];
  float depth[COIN3D_GIZMO_AXIS_COUNT];
  float lx = 0;
  float ly = 0;
  (void)gizmo;
  coin3d_gizmo_calc_rect(widget->w, widget->h, &r);
  coin3d_gizmo_project_axes(camera, &r, points_x, points_y, depth);
  coin3d_gizmo_to_local(widget, x, y, &lx, &ly);
  return coin3d_gizmo_pick_at(&r, points_x, points_y, depth, lx, ly);
}

ret_t coin3d_gizmo_draw(coin3d_gizmo_t* gizmo, widget_t* widget, canvas_t* c, coin3d_coin_t* coin) {
  uint32_t i = 0;
  rect_t r;
  float points_x[COIN3D_GIZMO_AXIS_COUNT];
  float points_y[COIN3D_GIZMO_AXIS_COUNT];
  float depth[COIN3D_GIZMO_AXIS_COUNT];
  float cx = 0;
  float cy = 0;
  float radius = 0;
  vgcanvas_t* vg = NULL;
  SoCamera* camera = NULL;
  static const char* k_axis_labels[COIN3D_GIZMO_AXIS_COUNT] = {"X", "-X", "Y", "-Y", "Z", "-Z"};
  return_value_if_fail(gizmo != NULL && widget != NULL && c != NULL && coin != NULL, RET_BAD_PARAMS);
  if (!gizmo->visible) {
    return RET_OK;
  }

  camera = coin3d_coin_get_camera(coin);
  return_value_if_fail(camera != NULL, RET_BAD_PARAMS);
  vg = canvas_get_vgcanvas(c);
  return_value_if_fail(vg != NULL, RET_BAD_PARAMS);

  coin3d_gizmo_calc_rect(widget->w, widget->h, &r);
  coin3d_gizmo_project_axes(camera, &r, points_x, points_y, depth);

  cx = (float)r.x + (float)r.w * 0.5f + (float)c->ox;
  cy = (float)r.y + (float)r.h * 0.5f + (float)c->oy;
  radius = (float)r.w * 0.48f;
  vgcanvas_draw_circle(vg, cx, cy, radius, color_init(36, 36, 42, 225), TRUE, 1.0f);
  vgcanvas_draw_circle(vg, cx, cy, radius, color_init(110, 110, 120, 255), FALSE, 1.0f);

  for (i = 0; i < COIN3D_GIZMO_AXIS_COUNT; i++) {
    color_t color = coin3d_gizmo_axis_color(i);
    vgcanvas_begin_path(vg);
    vgcanvas_set_line_width(vg, 2.0f);
    vgcanvas_set_stroke_color(vg, color);
    vgcanvas_move_to(vg, cx, cy);
    vgcanvas_line_to(vg, points_x[i] + (float)c->ox, points_y[i] + (float)c->oy);
    vgcanvas_stroke(vg);
  }

  for (i = 0; i < COIN3D_GIZMO_AXIS_COUNT; i++) {
    color_t color = coin3d_gizmo_axis_color(i);
    float dot_r = (float)r.w * COIN3D_GIZMO_DOT_RADIUS_RATIO;
    float px = points_x[i] + (float)c->ox;
    float py = points_y[i] + (float)c->oy;
    if (s_coin3d_gizmo_axis_pick[i] == gizmo->hover && depth[i] >= COIN3D_GIZMO_FRONT_EPS) {
      color = coin3d_gizmo_color_hovered(color);
    }
    vgcanvas_draw_circle(vg, px, py, dot_r, color, depth[i] >= COIN3D_GIZMO_FRONT_EPS ? TRUE : FALSE,
                         1.0f);
  }

  {
    float drawn_x[COIN3D_GIZMO_AXIS_COUNT];
    float drawn_y[COIN3D_GIZMO_AXIS_COUNT];
    uint32_t drawn_count = 0;
    float font_size = (float)tk_max(10, (int32_t)((float)r.w * 0.16f));
    float min_gap = font_size * 0.9f;
    vgcanvas_set_font_size(vg, (uint32_t)font_size);
    vgcanvas_set_text_align(vg, "center");
    vgcanvas_set_text_baseline(vg, "middle");
    for (i = 0; i < COIN3D_GIZMO_AXIS_COUNT; i++) {
      float px = points_x[i] + (float)c->ox;
      float py = points_y[i] + (float)c->oy;
      if (depth[i] < COIN3D_GIZMO_FRONT_EPS) {
        continue;
      }
      if (!coin3d_gizmo_label_can_draw(drawn_x, drawn_y, drawn_count, px, py, min_gap)) {
        continue;
      }
      if ((i % 2) == 0) {
        vgcanvas_set_fill_color(vg, color_init(245, 245, 245, 255));
      } else {
        vgcanvas_set_fill_color(vg, color_init(210, 210, 210, 220));
      }
      vgcanvas_fill_text(vg, k_axis_labels[i], px, py, -1);
      drawn_x[drawn_count] = px;
      drawn_y[drawn_count] = py;
      drawn_count++;
    }
  }

  return RET_OK;
}

bool_t coin3d_gizmo_on_event(coin3d_gizmo_t* gizmo, widget_t* widget, event_t* e,
                             coin3d_coin_t* coin) {
  SoCamera* camera = NULL;
  Coin3dView* view = NULL;
  return_value_if_fail(gizmo != NULL && widget != NULL && e != NULL && coin != NULL, FALSE);
  if (!gizmo->visible) {
    return FALSE;
  }

  camera = coin3d_coin_get_camera(coin);
  view = coin3d_coin_get_view(coin);
  return_value_if_fail(camera != NULL && view != NULL, FALSE);

  if (e->type == EVT_POINTER_DOWN) {
    pointer_event_t* evt = pointer_event_cast(e);
    coin3d_gizmo_pick_t pick = COIN3D_GIZMO_PICK_NONE;
    return_value_if_fail(evt != NULL, FALSE);
    pick = coin3d_gizmo_pick_widget(gizmo, widget, camera, (float)evt->x, (float)evt->y);
    if (pick == COIN3D_GIZMO_PICK_NONE) {
      return FALSE;
    }
    if (pick == COIN3D_GIZMO_PICK_DISK) {
      gizmo->dragging = TRUE;
      gizmo->last_x = (float)evt->x;
      gizmo->last_y = (float)evt->y;
    } else {
      coin3d_gizmo_snap_camera(camera, pick);
      view->syncFromCamera();
      gizmo->dragging = FALSE;
    }
    gizmo->captured = TRUE;
    widget_grab(coin3d_gizmo_grab_host(widget), widget);
    return TRUE;
  }

  if (e->type == EVT_POINTER_MOVE) {
    pointer_event_t* evt = pointer_event_cast(e);
    coin3d_gizmo_pick_t hover = COIN3D_GIZMO_PICK_NONE;
    return_value_if_fail(evt != NULL, FALSE);
    hover = coin3d_gizmo_pick_widget(gizmo, widget, camera, (float)evt->x, (float)evt->y);
    if (hover != gizmo->hover) {
      gizmo->hover = hover;
      widget_invalidate_force(widget, NULL);
    }
    if (!gizmo->dragging) {
      return FALSE;
    }
    view->rotateXY(((float)evt->y - gizmo->last_y) * COIN3D_GIZMO_ROTATE_SCALE,
                   ((float)evt->x - gizmo->last_x) * COIN3D_GIZMO_ROTATE_SCALE);
    gizmo->last_x = (float)evt->x;
    gizmo->last_y = (float)evt->y;
    return TRUE;
  }

  if (e->type == EVT_POINTER_UP || e->type == EVT_POINTER_LEAVE) {
    if (!gizmo->captured) {
      return FALSE;
    }
    gizmo->dragging = FALSE;
    gizmo->captured = FALSE;
    widget_ungrab(coin3d_gizmo_grab_host(widget), widget);
    return TRUE;
  }

  return FALSE;
}
