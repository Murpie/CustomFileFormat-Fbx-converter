#pragma once

#include <fbxsdk.h>
#include <stdlib.h>
#include <iostream>

class Converter
{
public:
	Converter();
	~Converter();

	

	void loadMesh(FbxNode* node);
	void exportFile();
	void exportFile2();
	void printName(FbxMesh* meshName);

private:
	FbxManager * manager;
	FbxIOSettings* settings;
	FbxScene* ourScene;
	FbxImporter* importer;

	FbxVector4* controlPoints;
	FbxLayerElementNormal* normalElement;
	FbxVector4 normal;
	FbxVector4 vtxNormal;
	FbxNode* child;
	FbxNode* rootNode;
	FbxMesh* mesh;

	int polygonCount;
	int polygonSize;
};

