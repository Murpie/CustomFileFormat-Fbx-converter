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
	void exportAnimation(FbxScene* scene, FbxNode* node);

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
	void getAnimation(FbxAnimLayer* animLayer, FbxNode* node);
	void getAnimationChannels(FbxNode* node, FbxAnimLayer* animLayer);
	void printInformation();
	//void createCustomLevelFile();
	//void loadBbox(FbxNode* currentNode);
	//void loadLevel(FbxNode* currentNode);
	//bool isPartOf(const char* nodeName);

	FbxManager * manager;
	FbxIOSettings* settings;
	FbxScene* ourScene;
	FbxImporter* importer;

	Counter counter;
	MeshInfo* meshInfo;
	VertexInformation* vertices;
	struct tempWeight {
		int ID;
		float weight;
	};

	std::vector<MaterialInformation> matInfo;
	//std::vector<BoundingBox> vBBox;
	//std::vector<LevelObject> levelObjects;
	AnimationInformation* animationInfo;
	BlendShapes* objectBlendShapes;
	Group* groups;
	CustomMayaAttributes* customMayaAttribute;
	Camera* exportCamera;
	Light* exportLight;

	FbxVector4* controlPoints;
	FbxVector4* blendShapeControlPoints;
	FbxNode* rootNode;
	FbxNode* child;
	FbxMesh* mesh;
	FbxBlendShape* blendShape;
	FbxLight* light;
	FbxCamera* camera;
	FbxString lString;
	//FbxNode* group;

	int polygonCount;
	int polygonSize;
	int textureCount;
	int nrOfWeights;

	bool foundBinormal = false;
	bool foundTangent = false;
	bool foundVertexWeight = false;

	const char* meshName;
	const char* textureName = nullptr;
	char* ret;
};

