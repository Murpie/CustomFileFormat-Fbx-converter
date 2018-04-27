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

LeapLevel * LeapImporter::getLevel(const char * levelName)
{

	LeapLevel* newLevel = new LeapLevel(levelName);

	return newLevel;
}

void LeapImporter::deleteObject(LeapMesh * mesh)
{
	delete mesh;
}

void LeapImporter::deleteObject(LeapLevel * level)
{
	delete level;
}
