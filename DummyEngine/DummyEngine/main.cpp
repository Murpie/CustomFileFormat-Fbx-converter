#include "LeapImporter.h"
//int main(int argc, char** argv)
int main()
{
	LeapImporter importer;

	LeapMesh* mesh = importer.getMesh("Assignment1.leap");
	

	printf("Vertex Count: %d\n", mesh->counterReader.vertexCount);


	std::getchar();
	//importer.deleteObject(level);
	return 0;
}