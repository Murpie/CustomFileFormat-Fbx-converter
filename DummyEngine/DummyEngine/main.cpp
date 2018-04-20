#include "LeapImporter.h"

int main()
{
	LeapImporter importer;

	LeapMesh* mesh = importer.getMesh("FBXcustomAttribute.leap");

	int vertexCount = mesh->getVertexCount();

	printf("%d\n", vertexCount);

	for (int i = 0; i < vertexCount; i++)
	{
		printf("Vertex: %d \nPositions: %f %f %f\n", i, mesh->vertices[i].x, mesh->vertices[i].y, mesh->vertices[i].z);
		printf("Normals: %f %f %f\n", mesh->vertices[i].nx, mesh->vertices[i].ny, mesh->vertices[i].nz);
		printf("UVs: %f %f\n\n", mesh->vertices[i].u, mesh->vertices[i].v);
	}

	getchar();
	importer.deleteObject(mesh);
	return 0;
}