#pragma once
#include <vector>
using namespace std;

struct Counter
{
	unsigned int vertexCount;
	unsigned int boundingBoxCount;
	unsigned int jointCount;
	unsigned int animationCount;
	unsigned int blendShapeCount;
	unsigned int groupID;
	unsigned int meshType;
};

struct MeshInfo {
	float globalTranslation[3];
	float globalRotation[3];
	float globalScaling[3];
};

struct VertexInformation {
	float x, y, z;
	float nx, ny, nz;
	float u, v;
	float weight[4];
	float weightID[4];
};

struct MaterialInformation {
	float ambient[3];
	float diffuse[3];
	float emissive[3];
	float opacity;
	char* textureFilePath[100];
};

struct BoundingBoxVertex {
	float bx, by, bz;
};

struct BoundingBox
{
	BoundingBoxVertex boundingBoxVertices[8];
};

struct KeyFrameData {
	float position[3]; //d								6
	float rotation[3]; //d								7
	float scaling[3]; //d								8
};

struct KeyFrame {
	float time; //d										5
	KeyFrameData keyFrameData; //d						9
};

struct JointInformation {
	char jointName[100]; //d							2
	char parentName[100]; //d							3
	float localTransformMatrix[16];						
	float bindPoseMatrix[16];							
	vector<KeyFrame> keyFrames; //d						?
};

struct AnimationInformation {
	char animationName[100]; //d						1
	int keyFrameCount; //d								4
	int nrOfJoints;
	vector<JointInformation> joints;					
	//vector <KeyFrame> keyFrames;
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