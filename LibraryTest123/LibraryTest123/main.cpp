#include <iostream>
#include <fstream>
#include <string>
#include "CustomImporter.h"

#pragma comment(lib, "Format Importer.lib")

int main() {
	
	CustomImporter importer;

	Mesh* mesh = importer.getMesh("wierdBox.leap");

	for (int i = 0; i < 108; i++)
	{
		printf("Vertex[%d]: %f %f %f\n", i, mesh->vertices[i].x, mesh->vertices[i].y, mesh->vertices[i].z);
	}
	
	importer.deleteObject(mesh);

	return 0;
}