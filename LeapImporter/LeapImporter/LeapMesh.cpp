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

	

	if (infile.is_open())
	{
		infile.close();
	}

	return;
}
