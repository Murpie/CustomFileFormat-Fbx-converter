#pragma once
#include "Mesh.h"
#include "Camera.h"
#include "Light.h"
#include "Material.h"
#include <vector>
#include <iostream>

using namespace std;

class CustomImporter
{
public:
	CustomImporter();
	~CustomImporter();

	Mesh* getMesh(const char* meshName);
	Camera* getCamera(const char* cameraName);
	Light* getLight(const char* lightName);
	Material* getMaterial(const char* materialName);

	void deleteObject(Mesh* mesh);
	void deleteObject(Camera* camera);
	void deleteObject(Light* light);
};