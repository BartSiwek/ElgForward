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

*/