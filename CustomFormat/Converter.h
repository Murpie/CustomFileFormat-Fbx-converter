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
	void loadGlobaltransform(FbxNode* currentNode);
	void loadVertex(FbxMesh* currentMesh);
	void loadMaterial(FbxNode* currentNode);
	void loadBlendShape(FbxMesh* currentMesh, FbxScene* scene);
	void loadCamera(FbxCamera* currentNode);
	void loadLights(FbxLight* currentLight);
	void createCustomFile();

	FbxManager * manager;
	FbxIOSettings* settings;
	FbxScene* ourScene;
	FbxImporter* importer;

	Counter counter;
	MeshInfo* meshInfo;
	VertexInformation* vertices;
	MaterialInformation* matInfo;
	BlendShapes* objectBlendShapes;

	FbxVector4* controlPoints;
	FbxVector4* blendShapeControlPoints;
	FbxNode* rootNode;
	FbxNode* child;
	FbxMesh* mesh;
	FbxBlendShape* blendShape;
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

