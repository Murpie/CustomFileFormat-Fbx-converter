#pragma once

#include <fbxsdk.h>
#include <stdlib.h>
#include <iostream>

class Converter
{
public:
	Converter();
	~Converter();

	FbxNode* rootNode;
	FbxMesh* mesh;

	void loadMesh(FbxNode* node);
	void exportFile();
	void printName(FbxMesh* meshName);

private:
	FbxManager * manager;
	FbxIOSettings* settings;
	FbxScene* ourScene;
	FbxImporter* importer;
};

