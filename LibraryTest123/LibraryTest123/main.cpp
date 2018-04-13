#include <iostream>
#include <fstream>
#include <string>


#include "CustomImporter.h"

#pragma comment(lib, "Format Importer.lib")

int main() {
	
	std::ifstream infile("testt.leap", std::ifstream::binary);

	Counter counterReader;

	infile.read((char*)&counterReader, sizeof(Counter));

	Vertex *vertices = new Vertex[counterReader.vertexCount];
	infile.read((char*)vertices, counterReader.vertexCount * sizeof(Vertex));


	infile.close();

	for (int i = 0; i < counterReader.vertexCount; i++)
	{
		std::cout << i << "|  X: " << vertices[i].x << " " << vertices[i].y << " " << vertices[i].z << std::endl;
		std::cout << i << "| NX: " << vertices[i].nx << " " << vertices[i].ny << " " << vertices[i].nz << std::endl;

		printf("%d| UV: %f %f\n\n", i, vertices[i].u, vertices[i].v);

	}

	getchar();

	return 0;
}