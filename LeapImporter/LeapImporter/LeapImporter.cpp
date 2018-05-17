#include "LeapImporter.h"

LeapImporter::LeapImporter()
{
}

LeapImporter::~LeapImporter()
{

}

LeapMesh * LeapImporter::getMesh(const char* meshName)
{
	//Check file type and get error if not correct.

	LeapMesh* newMesh = new LeapMesh(meshName);

	return newMesh;
}

void LeapImporter::deleteObject(LeapMesh * mesh)
{
	delete mesh;
}
