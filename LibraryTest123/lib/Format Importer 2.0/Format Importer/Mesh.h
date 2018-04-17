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
	
	Vertex* vertices;
	vector <BoundingBox> boundingBoxes;
	vector <Joint> joints;
	vector <Animation> animations;
	vector <BlendShape> blendShapes;
	vector <Group> group;
	Counter counterReader;
};