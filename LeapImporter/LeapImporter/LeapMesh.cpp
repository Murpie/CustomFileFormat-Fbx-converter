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

	for (int i = 0; i < counterReader.boundingBoxCount; i++)
	{
		BoundingBox* bbox = new BoundingBox();
		boundingBoxes.push_back(bbox);
		infile.read((char*)boundingBoxes[i], sizeof(BoundingBox));
	}

	if (infile.is_open())
	{
		infile.close();
	}

	return;
}
