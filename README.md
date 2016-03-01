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
- Trackball camera
- FPS camera (?, http://in2gpu.com/2016/02/26/opengl-fps-camera/)
- Restructure the mesh, material, input layout architecture
  - Shader reflection
  - ~~Move to using fcx to compile shaders~~
  - Add common semnatic names to C++/HLSL defines file
  - ~~Move to using AssImp for loading to support wider range of mesh data~~
  - ...
- ...

Vertex channels:
- Positions (HasPositions & mVertices)
- Normals (HasNormals & mNormals)
- Tangents (HasTangentsAndBitangents & mTangents)
- Bitangents (HasTangentsAndBitangents & mBitangents)
- Texture coordinates 0..AI_MAX_NUMBER_OF_TEXTURECOORDS (HasTextureCoords & mTextureCoords & mNumUVComponents)
- Colors 0..AI_MAX_NUMBER_OF_COLOR_SETS (HasVertexColors & mColors)
- Max channels = 1 + 1 + 1 + 1 + AI_MAX_NUMBER_OF_TEXTURECOORDS + AI_MAX_NUMBER_OF_COLOR_SETS = 20

Indices:
- Organized in faces (HasFaces & mFaces)