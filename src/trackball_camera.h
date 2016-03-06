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

*/