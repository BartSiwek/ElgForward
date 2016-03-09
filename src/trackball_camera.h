#pragma once

/*
MODES:
- Pan
- Rotate
- Zoom

EVENTS:
- On client size changed - register new size
- On mouse down - call PanStart / RotationStart / ZoomStart
- On mouse move - call PositionUpdate
- On mouse up - call PanEnd / RotationEnd / ZoomEnd

Functions:
- Get model matrix
- Get projection matrix

Possible members:
- Mode = enum { PAN, ROTATE, ZOOM }
- Current view matrix
- Current projection matrix
- Start point (of any operation)
- Current point (of any operation)
- Additional view matrix (to handle pan and rotate between Start and End)
- Additional projection matrix (to handle zoom between Start and End)

Rotation:
- Remember the start point on unit sphere 
- On move compute the new point on unit sphere
- Calculate the rotation axis and angle
  - Axis = cross product of the two points - its unit since the points are on unit sphere
  - Angle - take dot product
- Convert axis and angle to quaternion
- Update the current rotation quaternion with this info

Pan:
- Keep the current center point
- With mouse down remember the current start point in screen coords
- With mouse move:
  - Take the new pos in screen coord
  - Subrstract it from start pos to get the vector
  - Translate vector to a [0,1]x[0,1] space
  - Multiply this by width & height of the near plane to get the vector in world coords

Zoom:
- Keep the zoom factor - default = 1.0
- Map the difference between starting and current point on the screen to [-1, 1] according to the vertical axis
- Map [-1, 1] to some reasonable range [0.5, 2] for example
- 

*/

#include <DirectXMath.h>

class TrackballCamera {
public:
  TrackballCamera(uint32_t width, uint32_t height, float near, float far, float fovy)
      : m_center_(0, 0, 0),
        m_rotation_quaterion_(DirectX::XMQuaternionIdentity()),
        m_zoom_factor_(1.0f),
        m_view_matrix_(DirectX::XMMatrixIdentity()),
        m_proj_matrix_(DirectX::XMMatrixIdentity()),
        m_width_(width),
        m_height_(height),
        m_near_(near),
        m_far_(far),
        m_fov_(fovy) {
  }

  ~TrackballCamera() = default;

  TrackballCamera(const TrackballCamera&) = delete;
  TrackballCamera& operator=(const TrackballCamera&) = delete;

  TrackballCamera(TrackballCamera&&) = default;
  TrackballCamera& operator=(TrackballCamera&&) = default;

  void SetViewportSize(uint32_t width, uint32_t height) {
    m_width_ = width;
    m_height_ = height;
  }

  void StartPan(uint32_t x, uint32_t y) {

  }

  void EndPan() {

  }

  void StartRotation(uint32_t x, uint32_t y) {

  }

  void EndRotation() {

  }

  void StartZoom(uint32_t x, uint32_t y) {

  }

  void EndZoom() {

  }

  void UpdatePosition(uint32_t x, uint32_t y) {

  }

  void UpdateMatrices() {
    UpdateViewMatrix();
    UpdateProjMatrix();
  }

  const DirectX::XMMATRIX& GetViewMatrix() const {
    return m_view_matrix_;
  }

  const DirectX::XMMATRIX& GetProjectionMatrix() const {
    return m_proj_matrix_;
  }

private:
  void UpdateViewMatrix() {

  }

  void UpdateProjMatrix() {

  }

  DirectX::XMFLOAT3 m_center_;
  DirectX::XMVECTOR m_rotation_quaterion_;
  float m_zoom_factor_;

  DirectX::XMMATRIX m_view_matrix_;
  DirectX::XMMATRIX m_proj_matrix_;

  uint32_t m_width_;
  uint32_t m_height_;
  float m_near_;
  float m_far_;
  float m_fov_;
};