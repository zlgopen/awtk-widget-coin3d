/**
 * File:   coin3d_view.cpp
 * Author: AWTK Develop Team
 * Brief:  Examine-style camera interaction for coin3d
 *
 * Copyright (c) 2026 - 2026 Guangzhou ZHIYUAN Electronics Co.,Ltd.
 *
 */

/**
 * History:
 * ================================================================
 * 2026-08-17 Li XianJing <xianjimli@hotmail.com> created
 * 2026-08-18 Li XianJing <xianjimli@hotmail.com> examine look-at/pitch/yaw/distance
 * 2026-08-19 Li XianJing <xianjimli@hotmail.com> update clip planes on zoom
 *
 */

#include "coin3d_view.hpp"

#include <cfloat>
#include <cmath>

#include <Inventor/SbBox3f.h>
#include <Inventor/SbLinear.h>
#include <Inventor/SbSphere.h>
#include <Inventor/SoSceneManager.h>
#include <Inventor/actions/SoGetBoundingBoxAction.h>
#include <Inventor/nodes/SoCamera.h>
#include <Inventor/nodes/SoOrthographicCamera.h>
#include <Inventor/nodes/SoPerspectiveCamera.h>

#include "base/keys.h"
#include "base/window_manager.h"

#define COIN3D_VIEW_ROTATE_SCALE 0.01f
#define COIN3D_VIEW_ZOOM_SCALE 0.02f
#define COIN3D_VIEW_WHEEL_STEP 0.15f
#define COIN3D_VIEW_BUTTON_LEFT 1
#define COIN3D_VIEW_BUTTON_MIDDLE 2
#define COIN3D_VIEW_BUTTON_RIGHT 3
#define COIN3D_VIEW_ZOOM_REL_MIN 0.1f
#define COIN3D_VIEW_DIR_X 0.0f
#define COIN3D_VIEW_DIR_Y 0.0f
#define COIN3D_VIEW_DIR_Z -1.0f
#define COIN3D_VIEW_RIGHT_X 1.0f
#define COIN3D_VIEW_RIGHT_Y 0.0f
#define COIN3D_VIEW_RIGHT_Z 0.0f
#define COIN3D_VIEW_UP_X 0.0f
#define COIN3D_VIEW_UP_Y 1.0f
#define COIN3D_VIEW_UP_Z 0.0f
#define COIN3D_VIEW_PITCH_AXIS_X -1.0f
#define COIN3D_VIEW_PITCH_AXIS_Y 0.0f
#define COIN3D_VIEW_PITCH_AXIS_Z 0.0f
#define COIN3D_VIEW_YAW_AXIS_X 0.0f
#define COIN3D_VIEW_YAW_AXIS_Y -1.0f
#define COIN3D_VIEW_YAW_AXIS_Z 0.0f
#define COIN3D_VIEW_DEG_PER_RAD (180.0f / 3.14159265f)
#define COIN3D_VIEW_RAD_PER_DEG (3.14159265f / 180.0f)
#define COIN3D_VIEW_DIR_CLAMP 1.0f

static widget_t* coin3d_view_grab_host(widget_t* widget) {
  return widget->parent != NULL ? widget->parent : widget;
}

static float coin3d_view_clampf(float v, float lo, float hi) {
  if (v < lo) {
    return lo;
  }
  if (v > hi) {
    return hi;
  }
  return v;
}

Coin3dView::Coin3dView()
    : camera_(NULL),
      scene_manager_(NULL),
      look_at_x_(0.0f),
      look_at_y_(0.0f),
      look_at_z_(0.0f),
      pitch_deg_(0.0f),
      yaw_deg_(0.0f),
      distance_(0.0f),
      last_x_(0.0f),
      last_y_(0.0f),
      rotating_(FALSE),
      panning_(FALSE),
      zooming_(FALSE),
      has_pan_plane_(FALSE),
      pan_normal_x_(0.0f),
      pan_normal_y_(0.0f),
      pan_normal_z_(1.0f),
      pan_distance_(0.0f) {
}

void Coin3dView::init(SoCamera* camera, SoSceneManager* scene_manager) {
  camera_ = camera;
  scene_manager_ = scene_manager;
  syncFromCamera();
  rotating_ = FALSE;
  panning_ = FALSE;
  zooming_ = FALSE;
  has_pan_plane_ = FALSE;
}

void Coin3dView::reset(void) {
  camera_ = NULL;
  scene_manager_ = NULL;
  rotating_ = FALSE;
  panning_ = FALSE;
  zooming_ = FALSE;
  has_pan_plane_ = FALSE;
}

SoCamera* Coin3dView::camera(void) const {
  return camera_;
}

void Coin3dView::syncFromCamera(void) {
  SbVec3f dir;
  SbVec3f pos;
  float focal = 0.0f;
  if (camera_ == NULL) {
    return;
  }

  camera_->orientation.getValue().multVec(
      SbVec3f(COIN3D_VIEW_DIR_X, COIN3D_VIEW_DIR_Y, COIN3D_VIEW_DIR_Z), dir);
  pos = camera_->position.getValue();
  focal = camera_->focalDistance.getValue();
  look_at_x_ = pos[0] + focal * dir[0];
  look_at_y_ = pos[1] + focal * dir[1];
  look_at_z_ = pos[2] + focal * dir[2];

  if (camera_->getTypeId().isDerivedFrom(SoOrthographicCamera::getClassTypeId())) {
    SoOrthographicCamera* oc = (SoOrthographicCamera*)camera_;
    distance_ = oc->height.getValue();
  } else {
    distance_ = focal;
  }

  pitch_deg_ = asinf(coin3d_view_clampf(-dir[1], -COIN3D_VIEW_DIR_CLAMP, COIN3D_VIEW_DIR_CLAMP)) *
               COIN3D_VIEW_DEG_PER_RAD;
  yaw_deg_ = atan2f(dir[0], -dir[2]) * COIN3D_VIEW_DEG_PER_RAD;
}

void Coin3dView::applyCamera(void) {
  SbRotation rx;
  SbRotation ry;
  SbVec3f dir;
  SbVec3f pos;
  float dist_origo = 0.0f;
  if (camera_ == NULL) {
    return;
  }

  if (distance_ < COIN3D_VIEW_MIN_DISTANCE) {
    distance_ = COIN3D_VIEW_MIN_DISTANCE;
  }

  rx = SbRotation(
      SbVec3f(COIN3D_VIEW_PITCH_AXIS_X, COIN3D_VIEW_PITCH_AXIS_Y, COIN3D_VIEW_PITCH_AXIS_Z),
      pitch_deg_ * COIN3D_VIEW_RAD_PER_DEG);
  ry = SbRotation(SbVec3f(COIN3D_VIEW_YAW_AXIS_X, COIN3D_VIEW_YAW_AXIS_Y, COIN3D_VIEW_YAW_AXIS_Z),
                  yaw_deg_ * COIN3D_VIEW_RAD_PER_DEG);
  camera_->orientation = ry * rx;
  camera_->orientation.getValue().multVec(
      SbVec3f(COIN3D_VIEW_DIR_X, COIN3D_VIEW_DIR_Y, COIN3D_VIEW_DIR_Z), dir);

  if (camera_->getTypeId().isDerivedFrom(SoOrthographicCamera::getClassTypeId())) {
    SoOrthographicCamera* oc = (SoOrthographicCamera*)camera_;
    oc->height = distance_;
    pos.setValue(look_at_x_, look_at_y_, look_at_z_);
    pos -= camera_->focalDistance.getValue() * dir;
    camera_->position = pos;
    return;
  }

  pos.setValue(look_at_x_ - distance_ * dir[0], look_at_y_ - distance_ * dir[1],
               look_at_z_ - distance_ * dir[2]);
  dist_origo = pos.length();
  if (dist_origo > float(sqrt(FLT_MAX))) {
    return;
  }
  camera_->position = pos;
  camera_->focalDistance = distance_;
  updateClipPlanes();
}

void Coin3dView::updateClipPlanes(void) {
  float radius = 0.0f;
  float near_d = 0.0f;
  float far_d = 0.0f;
  if (camera_ == NULL || !camera_->isOfType(SoPerspectiveCamera::getClassTypeId())) {
    return;
  }

  if (scene_manager_ != NULL && scene_manager_->getSceneGraph() != NULL) {
    SoGetBoundingBoxAction action(scene_manager_->getViewportRegion());
    action.apply(scene_manager_->getSceneGraph());
    SbBox3f box = action.getBoundingBox();
    if (!box.isEmpty()) {
      SbSphere sphere;
      sphere.circumscribe(box);
      radius = sphere.getRadius();
    }
  }

  near_d = tk_max(COIN3D_VIEW_MIN_DISTANCE, distance_ * COIN3D_VIEW_NEAR_RATIO);
  far_d = tk_max(COIN3D_VIEW_MIN_FAR, distance_ + radius * COIN3D_VIEW_RADIUS_SLACK);
  if (radius > 0.0f && distance_ > radius) {
    near_d = tk_max(COIN3D_VIEW_MIN_DISTANCE, distance_ - radius);
  }

  SoPerspectiveCamera* pc = (SoPerspectiveCamera*)camera_;
  pc->nearDistance = near_d;
  pc->farDistance = far_d;
}

void Coin3dView::setLookAt(float x, float y, float z) {
  look_at_x_ = x;
  look_at_y_ = y;
  look_at_z_ = z;
  applyCamera();
}

void Coin3dView::getLookAt(float* x, float* y, float* z) const {
  if (x != NULL) {
    *x = look_at_x_;
  }
  if (y != NULL) {
    *y = look_at_y_;
  }
  if (z != NULL) {
    *z = look_at_z_;
  }
}

void Coin3dView::setOrbitDegrees(float pitch, float yaw) {
  pitch_deg_ = pitch;
  yaw_deg_ = yaw;
  applyCamera();
}

void Coin3dView::getOrbitDegrees(float* pitch, float* yaw) const {
  if (pitch != NULL) {
    *pitch = pitch_deg_;
  }
  if (yaw != NULL) {
    *yaw = yaw_deg_;
  }
}

void Coin3dView::setDistance(float distance) {
  distance_ = distance;
  applyCamera();
}

float Coin3dView::getDistance(void) const {
  return distance_;
}

void Coin3dView::panBy(float dx, float dy) {
  SbVec3f right;
  SbVec3f up;
  if (camera_ == NULL) {
    return;
  }
  camera_->orientation.getValue().multVec(
      SbVec3f(COIN3D_VIEW_RIGHT_X, COIN3D_VIEW_RIGHT_Y, COIN3D_VIEW_RIGHT_Z), right);
  camera_->orientation.getValue().multVec(
      SbVec3f(COIN3D_VIEW_UP_X, COIN3D_VIEW_UP_Y, COIN3D_VIEW_UP_Z), up);
  look_at_x_ += right[0] * dx + up[0] * dy;
  look_at_y_ += right[1] * dx + up[1] * dy;
  look_at_z_ += right[2] * dx + up[2] * dy;
  applyCamera();
}

void Coin3dView::rotateByDegrees(float pitch_delta, float yaw_delta) {
  pitch_deg_ += pitch_delta;
  yaw_deg_ += yaw_delta;
  applyCamera();
}

void Coin3dView::zoomByDistance(float delta) {
  setDistance(distance_ + delta);
}

void Coin3dView::rotateXY(float dx, float dy) {
  rotateByDegrees(dx * COIN3D_VIEW_DEG_PER_RAD, dy * COIN3D_VIEW_DEG_PER_RAD);
}

void Coin3dView::zoomBy(float delta) {
  float next = distance_ + delta * tk_max(COIN3D_VIEW_ZOOM_REL_MIN, fabsf(distance_));
  setDistance(next);
}

void Coin3dView::beginPan(widget_t* widget) {
  float aspect = (widget->h > 0) ? ((float)widget->w / (float)widget->h) : 1.0f;
  SbViewVolume vv = camera_->getViewVolume(aspect);
  SbVec3f forward;
  camera_->orientation.getValue().multVec(
      SbVec3f(COIN3D_VIEW_DIR_X, COIN3D_VIEW_DIR_Y, COIN3D_VIEW_DIR_Z), forward);
  SbVec3f focal =
      camera_->position.getValue() + camera_->focalDistance.getValue() * forward;
  SbPlane plane(vv.getPlane(camera_->focalDistance.getValue()).getNormal(), focal);
  SbVec3f normal = plane.getNormal();
  pan_normal_x_ = normal[0];
  pan_normal_y_ = normal[1];
  pan_normal_z_ = normal[2];
  pan_distance_ = plane.getDistanceFromOrigin();
  has_pan_plane_ = TRUE;
}

bool Coin3dView::panMotion(widget_t* widget, float_t prev_x, float_t prev_y, float_t curr_x,
                           float_t curr_y) {
  if (camera_ == NULL || widget == NULL || widget->w <= 0 || widget->h <= 0) {
    return false;
  }
  if (!has_pan_plane_) {
    beginPan(widget);
  }

  float aspect = (float)widget->w / (float)widget->h;
  point_t local_curr = {(xy_t)curr_x, (xy_t)curr_y};
  point_t local_prev = {(xy_t)prev_x, (xy_t)prev_y};
  widget_to_local(widget, &local_curr);
  widget_to_local(widget, &local_prev);

  SbVec2f curr((float)local_curr.x / (float)widget->w,
               1.0f - (float)local_curr.y / (float)widget->h);
  SbVec2f prev((float)local_prev.x / (float)widget->w,
               1.0f - (float)local_prev.y / (float)widget->h);
  SbPlane pan_plane(SbVec3f(pan_normal_x_, pan_normal_y_, pan_normal_z_), pan_distance_);
  SbViewVolume vv = camera_->getViewVolume(aspect);
  SbLine line;
  vv.projectPointToLine(curr, line);
  SbVec3f current_pt;
  pan_plane.intersect(line, current_pt);
  vv.projectPointToLine(prev, line);
  SbVec3f old_pt;
  pan_plane.intersect(line, old_pt);
  camera_->position = camera_->position.getValue() - (current_pt - old_pt);
  syncFromCamera();
  return true;
}

bool Coin3dView::onEvent(widget_t* widget, event_t* e) {
  return_value_if_fail(widget != NULL && e != NULL, false);
  if (camera_ == NULL) {
    return false;
  }

  uint32_t type = e->type;
  /* AWTK 将鼠标中键映射为 TK_KEY_WHEEL，而不是 POINTER_DOWN button=2 */
  if (type == EVT_KEY_DOWN) {
    key_event_t* evt = key_event_cast(e);
    widget_t* wm = NULL;
    return_value_if_fail(evt != NULL, false);
    if (evt->key != TK_KEY_WHEEL) {
      return false;
    }
    rotating_ = FALSE;
    zooming_ = FALSE;
    panning_ = TRUE;
    wm = widget_get_window_manager(widget);
    if (wm != NULL) {
      last_x_ = (float_t)window_manager_get_pointer_x(wm);
      last_y_ = (float_t)window_manager_get_pointer_y(wm);
    }
    beginPan(widget);
    widget_grab(coin3d_view_grab_host(widget), widget);
    return true;
  }

  if (type == EVT_KEY_UP) {
    key_event_t* evt = key_event_cast(e);
    return_value_if_fail(evt != NULL, false);
    if (evt->key != TK_KEY_WHEEL) {
      return false;
    }
    panning_ = FALSE;
    has_pan_plane_ = FALSE;
    widget_ungrab(coin3d_view_grab_host(widget), widget);
    return true;
  }

  if (type == EVT_POINTER_DOWN) {
    pointer_event_t* evt = pointer_event_cast(e);
    return_value_if_fail(evt != NULL, false);
    last_x_ = (float_t)evt->x;
    last_y_ = (float_t)evt->y;
    rotating_ = FALSE;
    panning_ = FALSE;
    zooming_ = FALSE;
    if (evt->button == COIN3D_VIEW_BUTTON_LEFT) {
      rotating_ = TRUE;
    } else if (evt->button == COIN3D_VIEW_BUTTON_MIDDLE) {
      panning_ = TRUE;
      beginPan(widget);
    } else if (evt->button == COIN3D_VIEW_BUTTON_RIGHT) {
      zooming_ = TRUE;
    } else {
      return false;
    }
    widget_grab(coin3d_view_grab_host(widget), widget);
    return true;
  }

  if (type == EVT_POINTER_MOVE) {
    pointer_event_t* evt = pointer_event_cast(e);
    return_value_if_fail(evt != NULL, false);
    if (!rotating_ && !panning_ && !zooming_) {
      return false;
    }
    float_t prev_x = last_x_;
    float_t prev_y = last_y_;
    float_t dx = (float_t)evt->x - last_x_;
    float_t dy = (float_t)evt->y - last_y_;
    last_x_ = (float_t)evt->x;
    last_y_ = (float_t)evt->y;
    if (rotating_) {
      rotateXY(dy * COIN3D_VIEW_ROTATE_SCALE, dx * COIN3D_VIEW_ROTATE_SCALE);
      return true;
    }
    if (panning_) {
      return panMotion(widget, prev_x, prev_y, (float_t)evt->x, (float_t)evt->y);
    }
    if (zooming_) {
      zoomBy(dy * COIN3D_VIEW_ZOOM_SCALE);
      return true;
    }
    return false;
  }

  if (type == EVT_POINTER_UP || type == EVT_POINTER_LEAVE) {
    rotating_ = FALSE;
    panning_ = FALSE;
    zooming_ = FALSE;
    has_pan_plane_ = FALSE;
    widget_ungrab(coin3d_view_grab_host(widget), widget);
    return true;
  }

  if (type == EVT_WHEEL) {
    wheel_event_t* evt = wheel_event_cast(e);
    return_value_if_fail(evt != NULL, false);
    float_t step = (evt->dy > 0) ? -COIN3D_VIEW_WHEEL_STEP : COIN3D_VIEW_WHEEL_STEP;
    zoomBy(step);
    return true;
  }

  return false;
}
