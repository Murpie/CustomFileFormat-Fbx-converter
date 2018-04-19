#include "LeapImporter.h"

LeapImporter::LeapImporter()
{
}

LeapImporter::~LeapImporter()
{
}

Mesh * LeapImporter::getMesh(const char* meshName)
{
	//Check file type and get error if not correct.

	Mesh* newMesh = new Mesh(meshName);

	return newMesh;
}

void LeapImporter::deleteObject(Mesh * mesh)
{
	delete mesh;
}