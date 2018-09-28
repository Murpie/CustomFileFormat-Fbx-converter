#include "LeapMesh.h"

LeapMesh::LeapMesh()
{
}

LeapMesh::LeapMesh(const char* fileName)
{
	loader(fileName);
}

LeapMesh::~LeapMesh()
{

}

int LeapMesh::getVertexCount()
{
	return this->counterReader.vertexCount;
}

void LeapMesh::loader(const char* fileName)
{
	
	std::ifstream infile(fileName, std::ifstream::binary);

	infile.read((char*)&counterReader, sizeof(Counter));

	transform = new MeshInfo[counterReader.meshCount];
	
	for (unsigned int i = 0; i < counterReader.meshCount; i++)
	{
		infile.read((char*)transform[i].meshName, sizeof(char) * 100);
		infile.read((char*)transform[i].globalTranslation, sizeof(float) * 3);
		infile.read((char*)transform[i].globalRotation, sizeof(float) * 3);
		infile.read((char*)transform[i].globalScaling, sizeof(float) * 3);
	}

	vertices = new VertexInformation[counterReader.vertexCount];
	infile.read((char*)vertices, counterReader.vertexCount * sizeof(VertexInformation));

	material = new MaterialInformation[counterReader.matCount];
	infile.read((char*)material, sizeof(MaterialInformation) * counterReader.matCount);

	animation = new Animation[1];

	infile.read((char*)animation->animation_name, sizeof(char) * 9);
	infile.read((char*)&animation->nr_of_keyframes, sizeof(int));
	infile.read((char*)&animation->nr_of_joints, sizeof(int));
	infile.read((char*)&animation->current_time, sizeof(float));
	infile.read((char*)&animation->max_time, sizeof(float));
	infile.read((char*)&animation->looping, sizeof(bool));
	infile.read((char*)&animation->switching, sizeof(bool));

	if (animation->nr_of_joints > 0)
	{
		animation->joints.resize(animation->nr_of_joints);
		for (int i = 0; i < animation->nr_of_joints; i++)
		{	
			infile.read((char*)&animation->joints[i].joint_name, sizeof(char) * 100);
			infile.read((char*)&animation->joints[i].parent_name, sizeof(char) * 100);
			infile.read((char*)&animation->joints[i].joint_id, sizeof(int));
			infile.read((char*)&animation->joints[i].parent_id, sizeof(int));

			infile.read((char*)&animation->joints[i].local_transform_matrix, sizeof(float) * 16);
			infile.read((char*)&animation->joints[i].bind_pose_matrix, sizeof(float) * 16);

			infile.read((char*)&animation->joints[i].translation, sizeof(float) * 3);
			infile.read((char*)&animation->joints[i].rotation, sizeof(float) * 3);
			infile.read((char*)&animation->joints[i].scale, sizeof(float) * 3);

			animation->joints[i].keyFrames.resize(animation->nr_of_keyframes);
			infile.read((char*)animation->joints[i].keyFrames.data(), sizeof(KeyFrame) * animation->nr_of_keyframes);
		}
	}

	customMayaAttribute = new CustomMayaAttributes[counterReader.customMayaAttributeCount];
	infile.read((char*)customMayaAttribute, sizeof(CustomMayaAttributes) * counterReader.customMayaAttributeCount);

	if (infile.is_open())
	{
		infile.close();
	}

	return;
}