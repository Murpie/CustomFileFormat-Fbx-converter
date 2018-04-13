#include "CustomImporter.h"

CustomImporter::CustomImporter()
{
}

CustomImporter::~CustomImporter()
{
}

Mesh * CustomImporter::getMesh(const std::string & meshName)
{
	//Check file type and get error if not correct.

	Mesh* newMesh = new Mesh(meshName);

	return newMesh;
}

Camera * CustomImporter::getCamera(const std::string & cameraName)
{
	//Check file type and get error if not correct.

	Camera* newCamera = new Camera(cameraName);

	return newCamera;
}

Light * CustomImporter::getLight(const std::string & lightName)
{
	//Check file type and get error if not correct.

	Light* newLight = new Light(lightName);

	return newLight;
}

Material * CustomImporter::getMaterial(const std::string & materialName)
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