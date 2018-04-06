#pragma once

#include <fbxsdk.h>

class Converter
{
public:
	Converter();
	~Converter();

	FbxNode* rootNode;
	FbxMesh* mesh;

	void loadMesh(const char* fileName);
	void printName(FbxNode* nodeName);

private:
	FbxManager * manager;
	FbxIOSettings* settings;
	FbxScene* ourScene;
	FbxImporter* importer;
};

