#pragma once

#include <fbxsdk.h>
#include <stdlib.h>
#include <iostream>

class Converter
{
public:
	Converter();
	Converter(const char* fileName);
	~Converter();

	void importMesh();
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

	const char* meshName;
};

