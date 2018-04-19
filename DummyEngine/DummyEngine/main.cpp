#include "LeapImporter.h"

int main()
{
	LeapImporter importer;

	Mesh* mesh = importer.getMesh("FBXcustomAttribute.leap");

	//int meshType1 = mesh->getMayaAttribute();
	int vertexCount = mesh->getVertexCount();

	//printf("%d\n", meshType1);
	printf("%d\n", vertexCount);

	for (int i = 0; i < vertexCount; i++)
	{
		printf("Vertex: %d \nPositions: %f %f %f\n", i, mesh->vertices[i].x, mesh->vertices[i].y, mesh->vertices[i].z);
		printf("Normals: %f %f %f\n", mesh->vertices[i].nx, mesh->vertices[i].ny, mesh->vertices[i].nz);
		printf("UVs: %f %f\n\n", mesh->vertices[i].u, mesh->vertices[i].v);
	}
	printf("CustomMayaAttribute: %d", mesh->customMayaAttribute->meshType);
	printf("CustomMayaAttribute: %d", mesh->customMayaAttribute[0].meshType);

	getchar();
	importer.deleteObject(mesh);
	return 0;
}