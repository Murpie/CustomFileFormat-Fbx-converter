#pragma once
#include "LeapMesh.h"
#include "LeapLevel.h"

#include <vector>
#include <iostream>

using namespace std;

class LeapImporter
{
public:
	LeapImporter();
	~LeapImporter();

	LeapMesh* getMesh(const char* meshName);
	LeapLevel* getLevel(const char* levelName);

	void deleteObject(LeapMesh* mesh);
	void deleteObject(LeapLevel* level);
};