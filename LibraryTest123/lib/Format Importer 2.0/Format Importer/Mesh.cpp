#include "Mesh.h"

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

void Mesh::loader(const char* fileName)
{
	vertices = new Vertex[counterReader.vertexCount];
	
	std::ifstream infile(fileName, std::ifstream::binary);

	/*Animation aTemp;
	this->animations.push_back(aTemp);*/

	infile.read((char*)&counterReader, sizeof(Counter));
	infile.read((char*)vertices, counterReader.vertexCount * sizeof(Vertex));

	infile.close();
}
