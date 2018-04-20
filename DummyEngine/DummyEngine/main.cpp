#include "LeapImporter.h"

int main()
{
	LeapImporter importer;

	Mesh* mesh = importer.getMesh("FBXcustomAttribute.leap");

	int vertexCount = mesh->getVertexCount();
	unsigned int customMeshType = (int)mesh->customMayaAttribute;

	printf("%d\n", vertexCount);

	for (int i = 0; i < vertexCount; i++)
	{
		printf("Vertex: %d \nPositions: %f %f %f\n", i, mesh->vertices[i].x, mesh->vertices[i].y, mesh->vertices[i].z);
		printf("Normals: %f %f %f\n", mesh->vertices[i].nx, mesh->vertices[i].ny, mesh->vertices[i].nz);
		printf("UVs: %f %f\n\n", mesh->vertices[i].u, mesh->vertices[i].v);
	}
	printf("CustomMayaAttribute: %d", mesh->customMayaAttribute);




	getchar();
	importer.deleteObject(mesh);
	return 0;
}