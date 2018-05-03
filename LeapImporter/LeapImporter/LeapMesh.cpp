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
	
	vertices = new VertexInformation[counterReader.vertexCount];

	infile.read((char*)vertices, counterReader.vertexCount * sizeof(VertexInformation));

	animation = new AnimationInformation[counterReader.animationCount];

	infile.read((char*)animation, sizeof(char) * 8);
	infile.read((char*)animation, sizeof(unsigned int) * 2);
	animation->joints.resize(animation->nrOfJoints);
	for (int i = 0; i < animation->nrOfJoints; i++)
	{	
		infile.read((char*)&animation->joints[i].jointName, sizeof(char*));
		infile.read((char*)&animation->joints[i].parentName, sizeof(char*));
		animation->joints[i].keyFrames.resize(animation->keyFrameCount);
		//infile.read((char*)animation->joints[i].localTransformMatrix, sizeof(float) * 16);
		//infile.read((char*)animation->joints[i].bindPoseMatrix, sizeof(float) * 16);
		infile.read((char*)animation->joints[i].keyFrames.data(), sizeof(KeyFrame) * animation->keyFrameCount);
	}

	if (infile.is_open())
	{
		infile.close();
	}

	return;
}
