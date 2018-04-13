#include "Material.h"

void Material::loader(const std::string & fileName)
{
	//Read material from file
}

Material::Material()
{
}

Material::Material(const std::string & fileName)
{
	loader(fileName);
}

Material::~Material()
{
}
