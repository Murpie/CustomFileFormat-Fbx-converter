#include "LeapImporter.h"

int main()
{
	LeapImporter importer;

	LeapMesh* mesh = importer.getMesh("testBox.leap");

	int vertexCount = mesh->getVertexCount();

	printf("%d\n", vertexCount);

	for (int i = 0; i < vertexCount; i++)
	{
		printf("Vertex: %d \nPositions: %f %f %f\n", i, mesh->vertices[i].x, mesh->vertices[i].y, mesh->vertices[i].z);
		printf("Normals: %f %f %f\n", mesh->vertices[i].nx, mesh->vertices[i].ny, mesh->vertices[i].nz);
		printf("UVs: %f %f\n\n", mesh->vertices[i].u, mesh->vertices[i].v);
	}
	for (int i = 0; i < mesh->boundingBoxes.size(); i++)
	{
		printf("BBox minVector: %f %f %f\n", mesh->boundingBoxes[i]->minVector[0], mesh->boundingBoxes[i]->minVector[1], mesh->boundingBoxes[i]->minVector[2]);
		printf("BBox maxVector: %f %f %f\n", mesh->boundingBoxes[i]->maxVector[0], mesh->boundingBoxes[i]->maxVector[1], mesh->boundingBoxes[i]->maxVector[2]);
		printf("BBox center: %f %f %f\n\n", mesh->boundingBoxes[i]->center[0], mesh->boundingBoxes[i]->center[1], mesh->boundingBoxes[i]->center[2]);
	}

	getchar();
	importer.deleteObject(mesh);
	return 0;
}