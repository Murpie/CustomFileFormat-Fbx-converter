#pragma once
#include "LeapMesh.h"

#include <vector>
#include <iostream>

using namespace std;

class LeapImporter
{
public:
	LeapImporter();
	~LeapImporter();

	LeapMesh* getMesh(const char* meshName);

	void deleteObject(LeapMesh* mesh);
};