#pragma once

#include <fbxsdk.h>
#include <string>
#include <stdlib.h>
#include <iostream>
#include "MeshStructs.h"

#define CUSTOM_ATTRIBUTE "MeshType"

class Converter
{
public:
	Converter(const char* fileName);
	~Converter();

	void importMesh();
	void exportFile(FbxNode* currentNode);

private:
	void loadGlobaltransform(FbxNode* currentNode);
	void loadVertex(FbxMesh* currentMesh);
	void loadMaterial(FbxNode* currentNode);
	void loadCamera(FbxCamera* currentNode);
	void loadLights(FbxLight* currentLight);
	void loadCustomMayaAttributes(FbxNode* currentNode);
	void createCustomFile();

	FbxManager * manager;
	FbxIOSettings* settings;
	FbxScene* ourScene;
	FbxImporter* importer;

	Counter counter;
	MeshInfo* meshInfo;
	Vertex* vertices;
	MaterialInformation* matInfo;
	CustomMayaAttributes* customMayaAttribute;

	FbxVector4* controlPoints;
	FbxNode* rootNode;
	FbxNode* child;
	FbxMesh* mesh;
	FbxLight* light;
	FbxCamera* camera;
	FbxString lString;;

	int polygonCount;
	int polygonSize;
	int textureCount;

	const char* meshName;
	const char* textureName;
	char* ret;
};

