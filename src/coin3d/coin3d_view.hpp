/**
 * File:   coin3d_view.hpp
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

#ifndef TK_COIN3D_VIEW_HPP
#define TK_COIN3D_VIEW_HPP

#include "awtk.h"

class SoCamera;
class SoSceneManager;

#define COIN3D_VIEW_MIN_DISTANCE 0.01f
#define COIN3D_VIEW_NEAR_RATIO 0.01f
#define COIN3D_VIEW_MIN_FAR 100.0f
#define COIN3D_VIEW_RADIUS_SLACK 1.0f

/**
 * Examine 查看器：左键旋转、中键（AWTK 的 TK_KEY_WHEEL）平移、滚轮/右键缩放。
 * 以观察点、轨道角、距离为真源，再写回相机。
 */
class Coin3dView {
 public:
  Coin3dView();
  void init(SoCamera* camera, SoSceneManager* scene_manager);
  void reset(void);
  SoCamera* camera(void) const;
  bool onEvent(widget_t* widget, event_t* e);
  void rotateXY(float dx, float dy);
  void syncFromCamera(void);
  void panBy(float dx, float dy);
  void rotateByDegrees(float pitch_delta, float yaw_delta);
  void zoomByDistance(float delta);
  void setLookAt(float x, float y, float z);
  void getLookAt(float* x, float* y, float* z) const;
  void setOrbitDegrees(float pitch, float yaw);
  void getOrbitDegrees(float* pitch, float* yaw) const;
  void setDistance(float distance);
  float getDistance(void) const;

 private:
  void applyCamera(void);
  void updateClipPlanes(void);
  void zoomBy(float delta);
  void beginPan(widget_t* widget);
  bool panMotion(widget_t* widget, float_t prev_x, float_t prev_y, float_t curr_x, float_t curr_y);

  SoCamera* camera_;
  SoSceneManager* scene_manager_;
  float look_at_x_;
  float look_at_y_;
  float look_at_z_;
  float pitch_deg_;
  float yaw_deg_;
  float distance_;
  float_t last_x_;
  float_t last_y_;
  bool_t rotating_;
  bool_t panning_;
  bool_t zooming_;
  bool_t has_pan_plane_;
  float pan_normal_x_;
  float pan_normal_y_;
  float pan_normal_z_;
  float pan_distance_;
};

#endif /*TK_COIN3D_VIEW_HPP*/
