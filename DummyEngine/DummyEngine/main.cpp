#include "LeapImporter.h"
//int main(int argc, char** argv)
int main()
{

	/*fprintf(stderr, "arguments: %d\n", argc);
	fprintf(stderr, "arguments: %s\n", argv[1]);
	fflush(NULL);
	getchar();*/
	LeapImporter importer;

	LeapMesh* mesh = importer.getMesh("BlendShapeMesh.leap");

	int vertexCount = mesh->getVertexCount();

	printf("%d\n", vertexCount);
	printf("Blendshape x: %f\n\n", mesh->blendShapes->blendShapeCount);
	
	for (int i = 0; i < vertexCount; i++)
	{
		/*printf("Vertex: %d \nPositions: %f %f %f\n", i, mesh->vertices[i].x, mesh->vertices[i].y, mesh->vertices[i].z);
		printf("Normals: %f %f %f\n", mesh->vertices[i].nx, mesh->vertices[i].ny, mesh->vertices[i].nz);
		printf("UVs: %f %f\n\n", mesh->vertices[i].u, mesh->vertices[i].v);*/
		
	}

	getchar();
	importer.deleteObject(mesh);
	return 0;
}