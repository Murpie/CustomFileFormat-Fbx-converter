#pragma once
using namespace std;
#include <string>

class Material
{
private:
	char ambiantPath[100];
	char specualPath[100];

	void loader(const std::string& fileName);

public:
	Material();
	Material(const std::string& fileName);
	~Material();

	//GetFunctions for everything
};