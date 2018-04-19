#pragma once
#include "Mesh.h"
#include <vector>
#include <iostream>

using namespace std;

class LeapImporter
{
public:
	LeapImporter();
	~LeapImporter();

	Mesh* getMesh(const char* meshName);

	void deleteObject(Mesh* mesh);
};