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
	unsigned int customMayaAttributeCount;
	unsigned int levelObjectCount;
};

struct MeshInfo {
	float globalTranslation[3];
	float globalRotation[3];
	float globalScaling[3];
};

struct VertexInformation {
	float x, y, z;
	float nx, ny, nz;
	float bnx, bny, bnz;
	float tx, ty, tz;
	float u, v;
	float weight[4];
	int weightID[4];
};

struct MaterialInformation {
	float ambient[3];
	float diffuse[3];
	float emissive[3];
	float opacity;
	char textureFilePath[100];
};

struct BoundingBox
{
	float minVector[3];
	float maxVector[3];
	float center[3];
};

struct KeyFrame {
	float time;
	float position[3];
	float rotation[3];
	float scaling[3];
};

struct JointInformation {
	char jointName[100];
	char parentName[100];
	//float localTransformMatrix[16];
	//float bindPoseMatrix[16];					
	vector<KeyFrame> keyFrames;
};

struct AnimationInformation {
	char animationName[9];
	int keyFrameCount;
	int nrOfJoints;
	vector<JointInformation> joints;					
};

struct BlendShapeVertex {
	float x, y, z;
	float nx, ny, nz;
};

struct BlendShape
{
	int blendShapeVertexCount;
	vector <BlendShapeVertex> blendShapeVertices;
};

struct BlendShapeKeyframe
{
	float time;
	float blendShapeInfluense;
};

struct BlendShapes {
	float blendShapeCount;
	float keyFrameCount;
	vector <BlendShape> blendShape;
	vector <BlendShapeKeyframe> keyframes;
};


struct Group {
	char groupName[100];
	int childCount;
	char childName[100][100];
};

struct CustomMayaAttributes {
	unsigned int meshType;
};

struct LevelObject {
	float x, y, z;
	float rotationX, rotationY, rotationZ;
	int id;
};

struct Light {
	char type[10];
	float color[3];
	float intensity;
	float innerCone;
	float outerCone;
};

struct Camera {
	float position[3];
	float up[3];
	float forward[3];
	float roll;
	float aspectWidth;
	float aspectHeight;
	float fov;
	float nearPlane;
	float farPlane;
};