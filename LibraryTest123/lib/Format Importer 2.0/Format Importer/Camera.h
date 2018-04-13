#pragma once
using namespace std;
#include <string>

class Camera
{
private:
	void loader(const std::string& fileName);

public:
	Camera();
	Camera(const std::string& fileName);
	~Camera();

	char cameraName[100];
	float aspectRatio;
	float fov;
	float nearPlane;
	float farPlane;
	float lookAt[3];
	float upVector[3];
	float position[3];

	//GetFunctions for everything
};