#include "Camera.h"

void Camera::loader(const std::string& fileName)
{

}

Camera::Camera()
{
}

Camera::Camera(const std::string& fileName)
{
	loader(fileName);
	//Fill data from file
}

Camera::~Camera()
{
}
