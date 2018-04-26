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
	
	for (int i = 0; i < counterReader.blendShapeCount; i++)
	{
		infile.read((char*)blendShapes->blendShape[i].blendShapeVertexCount, sizeof(int));
		infile.read((char*)blendShapes->blendShape[i].blendShapeVertices.data(), sizeof(BlendShapeVertex)*blendShapes->blendShape[i].blendShapeVertexCount);
	}

	infile.read((char*)blendShapes->keyframes.data(), sizeof(BlendShapeKeyframe)*blendShapes->keyFrameCount);

	/*infile.read((char*)blendShapes, counterReader.blendShapeCount * sizeof(float));*/

	

	if (infile.is_open())
	{
		infile.close();
	}

	return;
}
