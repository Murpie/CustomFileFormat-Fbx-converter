#include "Light.h"

void Light::loader(const std::string& fileName)
{

}

Light::Light()
{
}

Light::Light(const std::string& fileName)
{
	loader(fileName);
	//Fill data from file
}

Light::~Light()
{
}
