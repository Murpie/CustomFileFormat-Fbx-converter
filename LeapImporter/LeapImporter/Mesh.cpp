#include "Mesh.h"
#include <iostream>
#include <string>

Mesh::Mesh()
{
}

Mesh::Mesh(const char* fileName)
{
	loader(fileName);
}

Mesh::~Mesh()
{
}

int Mesh::getVertexCount()
{
	return this->counterReader.vertexCount;
}

int Mesh::getMayaAttribute()
{
	return this->customMayaAttribute->meshType;
}

void Mesh::loader(const char* fileName)
{
	
	std::ifstream infile(fileName, std::ifstream::binary);

	infile.read((char*)&counterReader, sizeof(Counter));
	
	vertices = new Vertex[counterReader.vertexCount];

	infile.read((char*)vertices, counterReader.vertexCount * sizeof(Vertex));

	customMayaAttribute = new CustomMayaAttributes[1];

	infile.read((char*)customMayaAttribute, sizeof(CustomMayaAttributes));

	if (infile.is_open())
	{
		infile.close();
	}

	return;
}
