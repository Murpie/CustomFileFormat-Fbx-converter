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
	
	vector <Vertex> vertices;
	vector <BoundingBox> boundingBoxes;
	vector <Joint> joints;
	vector <Animation> animations;
	vector <BlendShape> blendShapes;
	vector <Group> group;
};