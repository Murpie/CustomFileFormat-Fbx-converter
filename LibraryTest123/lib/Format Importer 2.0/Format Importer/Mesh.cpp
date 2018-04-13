#include "Mesh.h"

Mesh::Mesh()
{
}

Mesh::Mesh(const std::string& fileName)
{
	Vertex temp;
	this->vertices.push_back(temp);
	Animation aTemp;
	this->animations.push_back(aTemp);
	loader(fileName);
}

Mesh::~Mesh()
{
}

void Mesh::loader(const std::string& fileName)
{
	//Fill data from file
}
