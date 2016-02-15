# ElgForward
An experimental forward renderer written with DirectX 11

Goals:
- Support Blinn-Phong and Cook-Torrance shading models
- Support normal mapping and bump mapping
- Support shadow maps
- Support for ambient occlusion (technique TBD)

Needed per-vertex data:
- Normal (for Blinn-Phong and Cook-Torrance)
- Texture coords (diffuse, specular, normal/bump)
-