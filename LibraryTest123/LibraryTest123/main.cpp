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
		std::cout << i << "|  X: " << vertices[i].x << ", Y: " << vertices[i].y << ", Z: " << vertices[i].z << std::endl;
		std::cout << i << "| NX: " << vertices[i].nx << ", NY: " << vertices[i].ny << ", NZ: " << vertices[i].nz << std::endl << std::endl;
	}
	
	std::cout << "Vertex count: " << counterReader.vertexCount << std::endl << std::endl;
	std::cout << "Vtx on 0" << "   x: " << vertices[0].x << "   y: " << vertices[0].y << "   z: " << vertices[0].z << std::endl;
	std::cout << "Vtx on 1" << "   x: " << vertices[1].x << "   y: " << vertices[1].y << "   z: " << vertices[1].z << std::endl;
	std::cout << "Vtx on 15" << "   x: " << vertices[15].x << "   y: " << vertices[15].y << "   z: " << vertices[15].z << std::endl;

	getchar();

	return 0;
}