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

	/*for (int i = 0; i < counterReader.vertexCount; i++)
	{
		infile.read((char*)vertices->x, sizeof(Vertex)*);
	}*/


	infile.close();

	/*for (int i = 0; i < counterReader.vertexCount * 3; i++)
	{
		printf("X: %f Y: %f Z: %f\n", vertices->x, vertices->y, vertices->z);
	}*/
	
	std::cout << sizeof(vertices) << std::endl;

	getchar();

	return 0;
}