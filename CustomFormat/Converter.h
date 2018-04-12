#pragma once

#include <fbxsdk.h>
#include <stdlib.h>
#include <iostream>

class Converter
{
public:
	Converter();
	~Converter();

	void importMesh(const char* filename);
	void exportFile(FbxNode* currentNode);

private:
	FbxManager * manager;
	FbxIOSettings* settings;
	FbxScene* ourScene;
	FbxImporter* importer;

	FbxVector4* controlPoints;
	FbxNode* rootNode;
	FbxNode* child;
	FbxMesh* mesh;

	int polygonCount;
	int polygonSize;
};

