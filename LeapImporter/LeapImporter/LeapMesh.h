#pragma once
#include "MeshStructs.h"
#include <vector>
#include <string>
#include <fstream>

using namespace std;

class LeapMesh
{
private:
	void loader(const char* fileName);

public:
	LeapMesh();
	LeapMesh(const char* fileName);
	~LeapMesh();

	int getVertexCount();

	VertexInformation* vertices;
	MeshInfo* transform;
	MaterialInformation* material;
	BoundingBox* BBox;
	BoundingBoxVertex* BBoxVertices;
	JointInformation* joints;
	AnimationInformation* animation;
	KeyFrame* keyFrame;
	KeyFrameData* keyFrameData;
	BlendShapes* blendShapes;
	Group* group;
	Counter counterReader;

	/*vector <BoundingBox> boundingBoxes;
	vector <Joint> joints;
	vector <Animation> animations;
	vector <BlendShape> blendShapes;
	vector <Group> group;*/
};