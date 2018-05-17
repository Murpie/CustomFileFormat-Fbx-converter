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
	infile.read((char*)transform, sizeof(MeshInfo) * counterReader.meshCount);

	vertices = new VertexInformation[counterReader.vertexCount];
	infile.read((char*)vertices, counterReader.vertexCount * sizeof(VertexInformation));

	material = new MaterialInformation[counterReader.matCount];
	infile.read((char*)material, sizeof(MaterialInformation) * counterReader.matCount);

	animation = new AnimationInformation[1];

	infile.read((char*)animation->animationName, sizeof(char) * 9);
	infile.read((char*)&animation->keyFrameCount, sizeof(int));
	infile.read((char*)&animation->nrOfJoints, sizeof(int));
	animation->joints.resize(animation->nrOfJoints);
	for (int i = 0; i < animation->nrOfJoints; i++)
	{	
		infile.read((char*)&animation->joints[i].jointName, sizeof(char) * 100);
		infile.read((char*)&animation->joints[i].parentName, sizeof(char) * 100);
		animation->joints[i].keyFrames.resize(animation->keyFrameCount);
		infile.read((char*)animation->joints[i].keyFrames.data(), sizeof(KeyFrame) * animation->keyFrameCount);
	}

	blendShapes = new BlendShapes[counterReader.blendShapeCount];
	infile.read((char*)blendShapes, 2 * sizeof(float));
	blendShapes->blendShape.resize(blendShapes->blendShapeCount);
	blendShapes->keyframes.resize(blendShapes->keyFrameCount);
	
	for (int i = 0; i < blendShapes->blendShapeCount; i++)
	{
		int count = 0;
		BlendShape* bs = &blendShapes->blendShape[i];
		infile.read((char*)&count, sizeof(int));
		bs->blendShapeVertices.resize(count);
		bs->blendShapeVertexCount = count;
		infile.read((char*)bs->blendShapeVertices.data(), sizeof(BlendShapeVertex)*count);
	}

	infile.read((char*)blendShapes->keyframes.data(), sizeof(BlendShapeKeyframe)*blendShapes->keyFrameCount);

	group = new Group[1];
	infile.read((char*)group->groupName, sizeof(char) * 100);
	infile.read((char*)&group->childCount, sizeof(int));
	group->children.resize(group->childCount);
	for (int i = 0; i < group->childCount; i++)
	{
		infile.read((char*)&group->children[i].childName, sizeof(char) * 100);
	}

	customMayaAttribute = new CustomMayaAttributes[counterReader.customMayaAttributeCount];
	infile.read((char*)customMayaAttribute, sizeof(CustomMayaAttributes) * counterReader.customMayaAttributeCount);

	light = new Light[counterReader.lightCount];
	infile.read((char*)light, sizeof(Light) * counterReader.lightCount);

	camera = new Camera[counterReader.cameraCount];
	infile.read((char*)camera, sizeof(Camera) * counterReader.cameraCount);

	if (infile.is_open())
	{
		infile.close();
	}

	return;
}
