# ElgForward
An experimental forward renderer written with DirectX 11

Goals:
- Support Blinn-Phong and Cook-Torrance shading models
- Support normal mapping and bump mapping
- Support shadow maps
- Support for ambient occlusion (technique TBD)

Articles:
- http://www.thetenthplanet.de/archives/1180
- http://john-chapman-graphics.blogspot.nl/2013/01/ssao-tutorial.html
- http://www.gamedev.net/page/resources/_/technical/graphics-programming-and-theory/a-simple-and-practical-approach-to-ssao-r2753

Needed per-vertex data:
- Normal (for Blinn-Phong and Cook-Torrance, normal mapping, bump mapping)
- Texture coords (diffuse, specular, normal/bump, normal mapping)

Current agenda:
- ~~Trackball camera~~
- FPS camera (?, http://in2gpu.com/2016/02/26/opengl-fps-camera/)
- ~~Restructure the mesh, material, input layout architecture~~
  - ~~Shader reflection~~
  - ~~Move to using fcx to compile shaders~~
  - ~~Move to using AssImp for loading to support wider range of mesh data~~
- ~~Move to using handles for shaders, meshes, inputlayouts etc. and managers to enable caching~~
  - ~~Add a drawable manager and drawable handle~~
- Scene loading
  - ~~JSON Format~~
  - ~~Loading of json~~
  - ~~Loading of transforms for drawables~~
  - Loading of lights
  - Loading of textures
  - Change the camera update script to a general update script and expose buffers to it
- Shader reflection
  - Mapping of textures
  - Mapping of constant buffers and structured buffers
- General TODO:
  - Add mapping of custom shader input semantics to mesh channels 
