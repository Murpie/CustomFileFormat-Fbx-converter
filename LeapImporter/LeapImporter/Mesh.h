#pragma once
#include "MeshStructs.h"
#include <vector>
#include <string>
#include <fstream>

using namespace std;

class Mesh
{
private:
	void loader(const char* fileName);

public:
	Mesh();
	Mesh(const char* fileName);
	~Mesh();

	int getVertexCount();
	//int getMayaAttribute();

	Vertex* vertices;
	Counter counterReader;
	CustomMayaAttributes* customMayaAttribute;

	/*vector <BoundingBox> boundingBoxes;
	vector <Joint> joints;
	vector <Animation> animations;
	vector <BlendShape> blendShapes;
	vector <Group> group;*/
};