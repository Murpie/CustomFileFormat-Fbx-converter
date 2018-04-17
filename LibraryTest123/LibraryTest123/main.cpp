#include <iostream>
#include <fstream>
#include <string>
#include "CustomImporter.h"

#pragma comment(lib, "Format Importer.lib")

int main() {
	
	CustomImporter importer;

	Mesh* mesh = importer.getMesh("plane.leap");

	std::cout << "Vtx count: " << mesh->counterReader.vertexCount << std::endl;

	for (int i = 0; i < mesh->counterReader.vertexCount; i++)
	{
		printf("Vertex[%d]: %f %f %f\n", i, mesh->vertices[i].x, mesh->vertices[i].y, mesh->vertices[i].z);
	}
	getchar();
	
	importer.deleteObject(mesh);

	getchar();
	return 0;
}