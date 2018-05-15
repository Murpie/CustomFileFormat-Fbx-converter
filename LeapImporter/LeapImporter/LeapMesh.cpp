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
	for (BoundingBox* bbox_ptr : boundingBoxes)
	{
		delete bbox_ptr;
	}
	boundingBoxes.clear();
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

	/*infile.read((char*)blendShapes, counterReader.blendShapeCount * sizeof(float));*/

	

	for (int i = 0; i < counterReader.boundingBoxCount; i++)
	{
		BoundingBox* bbox = new BoundingBox();
		boundingBoxes.push_back(bbox);
		infile.read((char*)boundingBoxes[i], sizeof(BoundingBox));
	}

	//customMayaAttribute = new CustomMayaAttributes;

	//infile.read((char*)&customMayaAttribute, sizeof(CustomMayaAttributes));
	group = new Group;
	infile.read((char*)group, sizeof(Group));


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
