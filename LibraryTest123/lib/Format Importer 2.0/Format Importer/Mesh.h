#pragma once
#include "MeshStructs.h"
#include <vector>
#include <string>

using namespace std;

class Mesh
{
private:
	void loader(const std::string& fileName); 

public:
	Mesh();
	Mesh(const std::string& fileName);
	~Mesh();

	unsigned int vertexCount;
	vector <Vertex> vertices;
	unsigned int boundingBoxCount;
	vector <BoundingBox> boundingBoxes;
	unsigned int jointCount;
	vector <Joint> joints;
	unsigned int animationCount;
	vector <Animation> animations;
	unsigned int blendShapeCount;
	vector <BlendShape> blendShapes;
	unsigned int groupID;
	vector <Group> group;
	unsigned int meshType;
};