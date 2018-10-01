#pragma once

#include <fbxsdk.h>
#include <string>
#include <stdlib.h>
#include <iostream>   
#include "MeshStructs.h"
#include "AnimationStructs.h"

#define CUSTOM_ATTRIBUTE "MeshType"
#define TYPE_ID  "TypeID"

class Converter
{
public:
	Converter(const char* fileName);
	~Converter();

	bool isLevel;
	int currentJointIndex;

	void importMesh();
	void importAnimation();
	void exportFile(FbxNode* currentNode);
	void exportAnimation(FbxScene* scene, FbxNode* node);

private:
	void loadGlobaltransform(FbxNode* currentNode);
	void loadVertex(FbxMesh* currentMesh, FbxNode* currentNode);
	void loadMaterial(FbxNode* currentNode);
	void loadCustomMayaAttributes(FbxNode* currentNode);
	void loadWeights(FbxNode* currentNode, VertexInformation currentVertex, int vertexIndex);
	void loadLevel(FbxNode * currentNode);
	void createCustomFile();
	void createCustomLevelFile();
	void createCustomAnimationFile();
	void getAnimation(FbxAnimLayer* animLayer, FbxNode* node, FbxScene* scene);
	void getAnimationChannels(FbxNode* node, FbxAnimLayer* animLayer, FbxScene* scene);
	void fixJointID();

	FbxManager * manager;
	FbxIOSettings* settings;
	FbxScene* ourScene;
	FbxImporter* importer;

	Counter counter;
	std::vector<MeshInfo> meshInfo;
	std::vector<VertexInformation> vertices;
	std::vector<MaterialInformation> matInfo;

	Animation* animationInfo;
	std::vector<CustomMayaAttributes> customMayaAttribute;
	std::vector<LevelObject> levelObjects;

	FbxVector4* controlPoints;
	FbxVector4* blendShapeControlPoints;
	FbxNode* rootNode;
	FbxNode* child;
	FbxMesh* mesh;
	FbxBlendShape* blendShape;
	FbxLight* light;
	FbxCamera* camera;
	FbxString lString;

	int polygonCount;
	int polygonSize;
	int textureCount;
	int nrOfWeights;
	int totalNrOfVertices;

	bool foundBinormal = false;
	bool foundTangent = false;
	bool foundVertexWeight = false;

	const char* meshName;
	const char* textureName = nullptr;
	char* ret;
	
	char* tempMName = nullptr;

	struct tempWeight {
		int ID;
		float weight;
	};
};

