#pragma once
#include <vector>
using namespace std;

struct Vertex {
	float x, y, z;
	float nx, ny, nz;
	float u, v;
	float weight[4];
	float weightID[4];
};


struct BoundingBoxVertex {
	float bx, by, bz;
};

struct BoundingBox
{
	BoundingBoxVertex boundingBoxVertices[8];
};

struct Joint {
	char jointName[100];
	char parentName[100];
	float localTransformMatrix[16];
	float bindPoseMatrix[16];
};

struct KeyFrameData {
	float position[3];
	float rotation[4];
	float scaling[3];
};

struct KeyFrame {
	float time;
	vector <KeyFrameData> keyFrameData;
};

struct Animation {
	char animationName[100];
	int keyFrameCount;
	vector <KeyFrame> keyFrames;
};

struct BlendShapeVertex {
	float position[3];
};

struct BlendShape {
	float time;
	int blendShapeVertexCount;
	vector <BlendShapeVertex> blendShapeVertices;
};

struct Group {
	char groupName[100];
	char parentName[100];
};