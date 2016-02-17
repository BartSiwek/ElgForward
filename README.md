# ElgForward
An experimental forward renderer written with DirectX 11

Goals:
- Support Blinn-Phong and Cook-Torrance shading models
- Support normal mapping and bump mapping
- Support shadow maps
- Support for ambient occlusion (technique TBD)

Articles:
	http://www.thetenthplanet.de/archives/1180
	http://john-chapman-graphics.blogspot.nl/2013/01/ssao-tutorial.html
	http://www.gamedev.net/page/resources/_/technical/graphics-programming-and-theory/a-simple-and-practical-approach-to-ssao-r2753

Needed per-vertex data:
- Normal (for Blinn-Phong and Cook-Torrance, normal mapping, bump mapping)
- Texture coords (diffuse, specular, normal/bump, normal mapping)