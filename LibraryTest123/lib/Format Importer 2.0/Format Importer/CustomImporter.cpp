#include "CustomImporter.h"

CustomImporter::CustomImporter()
{
}

CustomImporter::~CustomImporter()
{
}

Mesh * CustomImporter::getMesh(const char* meshName)
{
	//Check file type and get error if not correct.

	Mesh* newMesh = new Mesh(meshName);

	return newMesh;
}

Camera * CustomImporter::getCamera(const char* cameraName)
{
	//Check file type and get error if not correct.

	Camera* newCamera = new Camera(cameraName);

	return newCamera;
}

Light * CustomImporter::getLight(const char* lightName)
{
	//Check file type and get error if not correct.

	Light* newLight = new Light(lightName);

	return newLight;
}

Material * CustomImporter::getMaterial(const char* materialName)
{
	Material* newMaterial = new Material(materialName);

	return newMaterial;
}

void CustomImporter::deleteObject(Mesh * mesh)
{
	delete mesh;
}

void CustomImporter::deleteObject(Camera * camera)
{
	delete camera;
}

void CustomImporter::deleteObject(Light * light)
{
	delete light;
}