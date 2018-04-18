#pragma once

#include <fbxsdk.h>
#include <stdlib.h>
#include <iostream>
#include "MeshStructs.h"

class Converter
{
public:
	Converter(const char* fileName);
	~Converter();

	void importMesh();
	void exportFile(FbxNode* currentNode);

private:
	void loadGlobaltransform();
	void loadVertex();
	void loadMaterial();
	void loadCameras();
	void createCustomFile();

	FbxManager * manager;
	FbxIOSettings* settings;
	FbxScene* ourScene;
	FbxImporter* importer;

	Counter counter;
	MeshInfo* meshInfo;
	Vertex* vertices;
	MaterialInformation* matInfo;

	FbxVector4* controlPoints;
	FbxNode* rootNode;
	FbxNode* child;
	FbxMesh* mesh;
	FbxCamera* camera;
	FbxString lString;;

	int polygonCount;
	int polygonSize;
	int textureCount;

	const char* meshName;
	const char* textureName;
	char* ret;
};

