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

	//animation = new AnimationInformation[counterReader.animationCount];
	animation = new AnimationInformation[1];

	infile.read((char*)animation->animationName, sizeof(char) * 9);
	infile.read((char*)&animation->keyFrameCount, sizeof(int));
	infile.read((char*)&animation->nrOfJoints, sizeof(int));
	animation->joints.resize(animation->nrOfJoints);
	for (int i = 0; i < animation->nrOfJoints; i++)
	{	
		infile.read((char*)&animation->joints[i].jointName, sizeof(char) * 100); // name is not 100 chars long
		infile.read((char*)&animation->joints[i].parentName, sizeof(char) * 100); //name is not 100 chars long
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
