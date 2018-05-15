#pragma once

#include <fbxsdk.h>
#include <string>
#include <stdlib.h>
#include <iostream>   
#include "MeshStructs.h"

#define CUSTOM_ATTRIBUTE "MeshType"
#define TYPE_ID  "TypeID"

class Converter
{
public:
	Converter(const char* fileName);
	~Converter();

	bool isLevel;

	void importMesh();
	void exportFile(FbxNode* currentNode);

private:
	void loadGlobaltransform(FbxNode* currentNode);
	void loadVertex(FbxMesh* currentMesh, FbxNode* currentNode);
	void loadMaterial(FbxNode* currentNode);
	void loadBlendShape(FbxMesh* currentMesh, FbxScene* scene);
	void loadCamera(FbxCamera* currentNode);
	void loadGroups(FbxNode* currentNode);
	void loadLights(FbxLight* currentLight);
	void loadCustomMayaAttributes(FbxNode* currentNode);
	void loadWeights(FbxNode* currentNode, int vertexIndex);
	void printInfo();
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
	Group* groups;
	CustomMayaAttributes* customMayaAttribute;

	FbxVector4* controlPoints;
	FbxVector4* blendShapeControlPoints;
	FbxNode* rootNode;
	FbxNode* child;
	FbxMesh* mesh;
	FbxBlendShape* blendShape;
	FbxLight* light;
	FbxCamera* camera;
	FbxString lString;
	FbxNode* group;

	int polygonCount;
	int polygonSize;
	int textureCount;
	int nrOfWeights;

	bool foundBinormal = false;
	bool foundTangent = false;
	bool foundVertexWeight = false;

	const char* meshName;
	const char* textureName;
	char* ret;

	//
	std::vector<BoundingBox> vBBox;
	bool isPartOf(const char* nodeName);
	void loadBbox(FbxNode* currentNode);

	//
	std::vector<LevelObject> levelObjects;
	void loadLevel(FbxNode* currentNode);
	void createCustomLevelFile();

	struct tempWeight {
		int ID;
		float weight;
	};
};

