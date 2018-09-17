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
	/*void loadBlendShape(FbxMesh* currentMesh, FbxScene* scene);
	void loadCamera(FbxCamera* currentNode);
	void loadGroups(FbxNode* currentNode);
	void loadLights(FbxLight* currentLight);*/
	void loadCustomMayaAttributes(FbxNode* currentNode);
	void loadWeights(FbxNode* currentNode, VertexInformation currentVertex, int vertexIndex);
	void printInfo();
	void createCustomFile();
	void getAnimation(FbxAnimLayer* animLayer, FbxNode* node);
	void getAnimationChannels(FbxNode* node, FbxAnimLayer* animLayer);

	FbxManager * manager;
	FbxIOSettings* settings;
	FbxScene* ourScene;
	FbxImporter* importer;

	Counter counter;
	std::vector<MeshInfo> meshInfo;
	std::vector<VertexInformation> vertices;
	std::vector<MaterialInformation> matInfo;

	AnimationInformation* animationInfo;
	/*BlendShapes* objectBlendShapes;
	std::vector<Group> groups;*/
	std::vector<CustomMayaAttributes> customMayaAttribute;
	/*std::vector<Camera> exportCamera;
	std::vector<Light> exportLight;*/

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

	struct tempWeight {
		int ID;
		float weight;
	};
};

