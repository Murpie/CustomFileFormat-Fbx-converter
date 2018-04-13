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

	Mesh* getMesh(const std::string& meshName);
	Camera* getCamera(const std::string& cameraName);
	Light* getLight(const std::string& lightName);
	Material* getMaterial(const std::string& materialName);

	void deleteObject(Mesh* mesh);
	void deleteObject(Camera* camera);
	void deleteObject(Light* light);
};