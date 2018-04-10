#pragma once
using namespace std;
#include <string>

class Light
{
private:
	void loader(const std::string& fileName);

public:
	Light();
	Light(const std::string& fileName);
	~Light();

	char lightName[100];
	float position[3];
	float color[3];
	int type;

	//GetFunctions for everything
};