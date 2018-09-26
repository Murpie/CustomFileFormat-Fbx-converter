#pragma once
#include "LeapMesh.h"

#include <vector>
#include <iostream>

class LeapImporter
{
public:
	LeapImporter();
	~LeapImporter();

	LeapMesh* getMesh(const char* meshName);

	void deleteObject(LeapMesh* mesh);
};