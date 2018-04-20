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
	void exportAnimation(FbxScene* scene, FbxNode* node);

private:
	void loadGlobaltransform(FbxNode* currentNode);
	void loadVertex(FbxMesh* currentMesh);
	void loadMaterial(FbxNode* currentNode);
	void loadCamera(FbxCamera* currentNode);
	void loadLights(FbxLight* currentLight);
	void createCustomFile();
	void getAnimation(FbxAnimLayer* animLayer, FbxNode* node);
	void getAnimationChannels(FbxNode* node, FbxAnimLayer* animLayer);
	void displayCurveKeys(FbxAnimCurve* curve);
	void printInformation();

	FbxManager * manager;
	FbxIOSettings* settings;
	FbxScene* ourScene;
	FbxImporter* importer;

	Counter counter;
	MeshInfo* meshInfo;
	VertexInformation* vertices;
	MaterialInformation* matInfo;
	AnimationInformation* animationInfo;

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

