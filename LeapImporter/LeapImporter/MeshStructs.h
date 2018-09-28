#pragma once
#include <vector>

struct Counter
{
	unsigned int vertexCount;
	unsigned int meshCount;
	unsigned int customMayaAttributeCount;
	unsigned int matCount;
	unsigned int levelObjectCount;
};

struct MeshInfo {
	char meshName[100];
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

struct KeyFrame {
	float time;
	float position[3];
	float rotation[3];
	float scaling[3];
};

struct Joint {
	char joint_name[100];
	char parent_name[100];
	int joint_id;
	int parent_id;
	float local_transform_matrix[4][4];
	float bind_pose_matrix[4][4];
	float translation[3];
	float rotation[3];
	float scale[3];
	vector<KeyFrame> keyFrames;
};

struct Animation {
	char animation_name[9];
	int nr_of_keyframes;
	int nr_of_joints;
	float current_time;
	float max_time;
	bool looping;
	bool switching;
	vector<Joint> joints;
};

struct CustomMayaAttributes {
	float particlePivot[3];
	float centerPivot[3];
	float height;
	float width;
	int id;
};

struct LevelObject {
	float x, y, z;
	float rotationX, rotationY, rotationZ;
	int id;
};