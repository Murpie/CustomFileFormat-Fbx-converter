#include "Converter.h"
#include <fstream>
#include <vector>
#include <string>

using namespace std;
#pragma warning(disable : 4996)

#define COLOR_RANGE 3
#define UV_RANGE 2

Converter::Converter(const char * fileName)
{
	manager = FbxManager::Create();
	settings = FbxIOSettings::Create(manager, IOSROOT);
	manager->SetIOSettings(settings);
	ourScene = FbxScene::Create(manager, "");
	importer = FbxImporter::Create(manager, "");
	this->counter.customMayaAttributeCount = 0;
	this->meshName = fileName;
	//tempMName = (char*)malloc(100);
}
//----------------------------------------------------------------------------------------------------------------------------------------------------------------
Converter::~Converter()
{
	delete ret;
	delete[] animationInfo;

	meshInfo.clear();
	vertices.clear();
	matInfo.clear();
	customMayaAttribute.clear();
	
	ourScene->Destroy();
	settings->Destroy();
	manager->Destroy();

	//free(tempMName);
	//free(tempMeshName);
}
//----------------------------------------------------------------------------------------------------------------------------------------------------------------
void Converter::importMesh()
{
	if (!importer->Initialize(meshName, -1, manager->GetIOSettings()))
	{
		printf("Call to fbximporter::initialize failed.\n");
		printf("Error returned: %s\n\n", importer->GetStatus().GetErrorString());
		getchar();
		exit(-1);
	}

	importer->Import(ourScene);
	importer->Destroy();

	rootNode = ourScene->GetRootNode();
	
	exportFile(rootNode);

	if (rootNode->GetChildCount() > 0)
	{
		for (int i = 0; i < rootNode->GetChildCount(); i++)
		{
			if (isLevel)
			{
				loadLevel(rootNode->GetChild(i));
			}
			else
			{
				exportFile(rootNode->GetChild(i));
			}
		}
	}
	
	counter.vertexCount = totalNrOfVertices;
	//exportAnimation(ourScene, rootNode);

	if (isLevel)
	{
		createCustomLevelFile();
	}
	else
	{
		createCustomFile();
	}
}
//----------------------------------------------------------------------------------------------------------------------------------------------------------------
void Converter::importAnimation()
{
	//importer->Import(ourScene);
	//rootNode = ourScene->GetRootNode();
	exportAnimation(ourScene, rootNode);
	createCustomAnimationFile();
}
//----------------------------------------------------------------------------------------------------------------------------------------------------------------
void Converter::exportFile(FbxNode* currentNode)
{
	mesh = currentNode->GetMesh();

	if (currentNode)
	{
		if (mesh)
		{
			loadGlobaltransform(currentNode);
			loadVertex(mesh, currentNode);
			loadMaterial(currentNode);
			loadCustomMayaAttributes(currentNode);
		}
	}
	else
	{
		printf("Access violation: Node not found\n\n");
		exit(-2);
	}
}
//----------------------------------------------------------------------------------------------------------------------------------------------------------------
void Converter::loadGlobaltransform(FbxNode* currentNode)
{
	MeshInfo tempMeshInfo;
	const char* tempString;
	FbxDouble3 tempTranslation = currentNode->LclTranslation.Get();
	FbxDouble3 tempRotation = currentNode->LclRotation.Get();
	FbxDouble3 tempScaling = currentNode->LclScaling.Get();

	for (int i = 0; i < COLOR_RANGE; i++)
	{
		tempMeshInfo.globalTranslation[i] = tempTranslation[i];
		tempMeshInfo.globalRotation[i] = tempRotation[i];
		tempMeshInfo.globalScaling[i] = tempScaling[i];
	}

	tempString = currentNode->GetName();

	for (unsigned int i = 0; i < strlen(tempString) + 1; i++)
	{
		tempMeshInfo.meshName[i] = tempString[i];
	}
	
	meshInfo.push_back(tempMeshInfo);
	counter.meshCount++;
}
//----------------------------------------------------------------------------------------------------------------------------------------------------------------
void Converter::loadVertex(FbxMesh* currentMesh, FbxNode* currentNode)
{
	polygonCount = currentMesh->GetPolygonCount();

	//Vertices
	controlPoints = currentMesh->GetControlPoints();

	std::vector<FbxVector4> pos;
	std::vector<FbxVector4> norm;
	std::vector<FbxVector4> biNorm;
	std::vector<FbxVector4> tangent;
	std::vector<FbxVector2> uv;
	FbxVector4 temp;
	FbxVector2 tempUv;
	FbxGeometryElementBinormal* vertexBinormal;
	FbxGeometryElementTangent * vertexTangent;

	bool ItIsFalse = false;

	int i = 0;
	for (int polygonIndex = 0; polygonIndex < polygonCount; polygonIndex++)
	{
		for (int vertexIndex = 0; vertexIndex < currentMesh->GetPolygonSize(polygonIndex); vertexIndex++)
		{
			VertexInformation tempVtx;

			//Positions
			pos.push_back(controlPoints[currentMesh->GetPolygonVertex(polygonIndex, vertexIndex)]);

			//Normals
			currentMesh->GetPolygonVertexNormal(polygonIndex, vertexIndex, temp);
			norm.push_back(temp);

			vertexBinormal = currentMesh->GetElementBinormal();
			if (vertexBinormal)
			{
				foundBinormal = true;
				
				int lNormalIndex = 0;

				if (vertexBinormal->GetReferenceMode() == FbxGeometryElement::eDirect)
					lNormalIndex = vertexIndex;

				if (vertexBinormal->GetReferenceMode() == FbxGeometryElement::eIndexToDirect)
					lNormalIndex = vertexBinormal->GetIndexArray().GetAt(vertexIndex);

				FbxVector4 lNormal = vertexBinormal->GetDirectArray().GetAt(lNormalIndex);
				biNorm.push_back(lNormal);
				

				tempVtx.bnx = (float)biNorm[i][0];
				tempVtx.bny = (float)biNorm[i][1];
				tempVtx.bnz = (float)biNorm[i][2];
			}

			vertexTangent = currentMesh->GetElementTangent();
			if (vertexTangent)
			{
				foundTangent = true;
				
				int lNormalIndex = 0;

				if (vertexTangent->GetReferenceMode() == FbxGeometryElement::eDirect)
					lNormalIndex = vertexIndex;

				if (vertexTangent->GetReferenceMode() == FbxGeometryElement::eIndexToDirect)
					lNormalIndex = vertexTangent->GetIndexArray().GetAt(vertexIndex);

				FbxVector4 lNormal = vertexTangent->GetDirectArray().GetAt(lNormalIndex);
				tangent.push_back(lNormal);

				tempVtx.tx = (float)tangent[i][0];
				tempVtx.ty = (float)tangent[i][1];
				tempVtx.tz = (float)tangent[i][2];
			}

			//UVs
			FbxStringList uvSetNamesList;
			currentMesh->GetUVSetNames(uvSetNamesList);
			const char* uvNames = uvSetNamesList.GetStringAt(0);
			currentMesh->GetPolygonVertexUV(polygonIndex, vertexIndex, uvNames, tempUv, ItIsFalse);
			uv.push_back(tempUv);

			tempVtx.x = (float)pos[i][0];
			tempVtx.y = (float)pos[i][1];
			tempVtx.z = (float)pos[i][2];

			tempVtx.nx = (float)norm[i][0];
			tempVtx.ny = (float)norm[i][1];
			tempVtx.nz = (float)norm[i][2];

			tempVtx.u = (float)uv[i][0];
			tempVtx.v = (float)uv[i][1];

			//Weights
			loadWeights(currentNode, tempVtx, i);

			i++;
			totalNrOfVertices++;
		}
	}
}
//----------------------------------------------------------------------------------------------------------------------------------------------------------------
void Converter::loadMaterial(FbxNode* currentNode)
{
	//Material & Texture
	int materialCount = currentNode->GetMaterialCount();

	//Material attributes
	FbxPropertyT<FbxDouble3> lKFbxDouble3;
	FbxPropertyT<FbxDouble> transparency;
	FbxColor ambient;
	FbxColor diffuse;
	FbxColor emissive;

	if (materialCount > 0)
	{
		for (int mat = 0; mat < materialCount; mat++)
		{
			FbxSurfaceMaterial *lMaterial = currentNode->GetMaterial(mat);

			if (lMaterial->GetClassId().Is(FbxSurfaceLambert::ClassId))
			{
				lKFbxDouble3 = ((FbxSurfaceLambert*)lMaterial)->Ambient;
				ambient.Set(lKFbxDouble3.Get()[0], lKFbxDouble3.Get()[1], lKFbxDouble3.Get()[2]);

				lKFbxDouble3 = ((FbxSurfaceLambert*)lMaterial)->Diffuse;
				diffuse.Set(lKFbxDouble3.Get()[0], lKFbxDouble3.Get()[1], lKFbxDouble3.Get()[2]);

				lKFbxDouble3 = ((FbxSurfaceLambert*)lMaterial)->Emissive;
				emissive.Set(lKFbxDouble3.Get()[0], lKFbxDouble3.Get()[1], lKFbxDouble3.Get()[2]);

				transparency = ((FbxSurfaceLambert*)lMaterial)->TransparencyFactor;
			}

			//File Texture path from Material
			FbxProperty fileTextureProp = lMaterial->FindProperty(FbxSurfaceMaterial::sDiffuse);

			if (fileTextureProp != NULL)
			{
				textureCount = fileTextureProp.GetSrcObjectCount<FbxFileTexture>();
				for (int i = 0; i < textureCount; i++)
				{
					FbxFileTexture* texture = FbxCast<FbxFileTexture>(fileTextureProp.GetSrcObject<FbxFileTexture>(i));

					textureName = texture->GetFileName();
				}
			}
		}
	}

	//Create custom materialInformation
	MaterialInformation tempMatInfo;

	//Colormaps
	for (int k = 0; k < COLOR_RANGE; k++)
	{
		switch (k)
		{
		case 0:
			tempMatInfo.ambient[k] = (float)ambient.mRed;
			tempMatInfo.diffuse[k] = (float)diffuse.mRed;
			tempMatInfo.emissive[k] = (float)emissive.mRed;
			break;
		case 1:
			tempMatInfo.ambient[k] = (float)ambient.mGreen;
			tempMatInfo.diffuse[k] = (float)diffuse.mGreen;
			tempMatInfo.emissive[k] = (float)emissive.mGreen;
			break;
		case 2:
			tempMatInfo.ambient[k] = (float)ambient.mBlue;
			tempMatInfo.diffuse[k] = (float)diffuse.mBlue;
			tempMatInfo.emissive[k] = (float)emissive.mBlue;
			break;
		default:
			break;
		}
	}

	//Path if there is any
	if (textureName != nullptr)
	{
		for (int i = 0; i < strlen(textureName) + 1; i++)
		{
			tempMatInfo.textureFilePath[i] = textureName[i];
		}
	}

	//Opacity
	tempMatInfo.opacity = (float)transparency;
	matInfo.push_back(tempMatInfo);
	counter.matCount++;
}
//----------------------------------------------------------------------------------------------------------------------------------------------------------------
void Converter::loadWeights(FbxNode* currentNode, VertexInformation currentVertex, int vertexIndex)
{
	foundVertexWeight = false;
	nrOfWeights = 0;

	//Skin with weights for vertices
	FbxPatch* patch = (FbxPatch*)currentNode->GetNodeAttribute();
	int controlPointsCount = patch->GetControlPointsCount();
	FbxVector4* controlPoints = patch->GetControlPoints();

	int clusterCount;
	int nrOfJoints = 0;
	FbxCluster* cluster;
	int skinCount = patch->GetDeformerCount(FbxDeformer::eSkin);
	
	vector<tempWeight> store;

	for (int i = 0; i < skinCount; i++)
	{
		clusterCount = ((FbxSkin*)patch->GetDeformer(i, FbxDeformer::eSkin))->GetClusterCount();
		for (int j = 0; j < clusterCount; j++)
		{
			cluster = ((FbxSkin*)patch->GetDeformer(i, FbxDeformer::eSkin))->GetCluster(j);

			int* indices = cluster->GetControlPointIndices();
			int indexCount = cluster->GetControlPointIndicesCount();
			double* weights = cluster->GetControlPointWeights();

			tempWeight tempStore;

			for (int index = 0; index < indexCount; index++)
			{
				if (vertexIndex == indices[index])
				{
					tempStore.ID = j;
					tempStore.weight = weights[index];

					store.push_back(tempStore);
					foundVertexWeight = true;
					nrOfWeights++;
				}
			}
		}
	}

	if (foundVertexWeight)
	{
		for (int i = 0; i < store.size(); i++)
		{
			int id = store[i].ID;
			float newTempWeight = store[i].weight;

			for (int j = 0; j < store.size(); j++)
			{
				if (store[j].weight < store[i].weight)
				{
					store[i].weight = store[j].weight;
					store[j].weight = newTempWeight;
					newTempWeight = store[i].weight;

					store[i].ID = store[j].ID;
					store[j].ID = id;
					id = store[i].ID;
				}
			}
		}

		
		for (int j = 0; j < nrOfWeights; j++)
		{
			if (j == 4)
			{
				break;
			}
			if (store[j].weight > 0)
			{
				currentVertex.weight[j] = store[j].weight;
				currentVertex.weightID[j] = store[j].ID;
			}
		}

		if (nrOfWeights < 4)
		{
			if (nrOfWeights == 3)
			{
				currentVertex.weight[3] = -1;
				currentVertex.weightID[3] = -1;
			}
			else if (nrOfWeights == 2)
			{
				currentVertex.weight[2] = -1;
				currentVertex.weightID[2] = -1;
				currentVertex.weight[3] = -1;
				currentVertex.weightID[3] = -1;
			}
			else if (nrOfWeights == 1)
			{
				currentVertex.weight[1] = -1;
				currentVertex.weightID[1] = -1;
				currentVertex.weight[2] = -1;
				currentVertex.weightID[2] = -1;
				currentVertex.weight[3] = -1;
				currentVertex.weightID[3] = -1;
			}
			else
			{
				currentVertex.weight[0] = -1;
				currentVertex.weightID[0] = -1;
				currentVertex.weight[1] = -1;
				currentVertex.weightID[1] = -1;
				currentVertex.weight[2] = -1;
				currentVertex.weightID[2] = -1;
				currentVertex.weight[3] = -1;
				currentVertex.weightID[3] = -1;
			}
		}

	}
	else
	{
		currentVertex.weight[0] = -1;
		currentVertex.weightID[0] = -1;
		currentVertex.weight[1] = -1;
		currentVertex.weightID[1] = -1;
		currentVertex.weight[2] = -1;
		currentVertex.weightID[2] = -1;
		currentVertex.weight[3] = -1;
		currentVertex.weightID[3] = -1;
	}

	store.clear();
	vertices.push_back(currentVertex);
}
//----------------------------------------------------------------------------------------------------------------------------------------------------------------
void Converter::loadCustomMayaAttributes(FbxNode * currentNode)
{
	CustomMayaAttributes tempCustom;		//Kolla s� man f�r in alla v�rden; x, y och z;

	std::string nodeName = currentNode->GetName();

	FbxProperty prop = currentNode->FindProperty("ParticlePivotX", false);
	if (prop.IsValid())
	{
		tempCustom.particlePivot[0] = prop.Get<float>();
	}
	else
	{
		tempCustom.particlePivot[0] = -999.0f;
	}

	prop = currentNode->FindProperty("ParticlePivotY", false);
	if (prop.IsValid())
	{
		tempCustom.particlePivot[1] = prop.Get<float>();
	}
	else
	{
		tempCustom.particlePivot[1] = -999.0f;
	}

	prop = currentNode->FindProperty("ParticlePivotZ", false);
	if (prop.IsValid())
	{
		tempCustom.particlePivot[2] = prop.Get<float>();
	}
	else
	{
		tempCustom.particlePivot[2] = -999.0f;
	}

	prop = currentNode->FindProperty("ID", false);
	if (prop.IsValid())
	{
		tempCustom.id = prop.Get<int>();
	}
	else
	{
		tempCustom.id = -1;
	}

	prop = currentNode->FindProperty("CBox_Height", false);
	if (prop.IsValid())
	{
		tempCustom.height = prop.Get<float>();
	}
	else
	{
		tempCustom.height = -1.0f;
	}

	prop = currentNode->FindProperty("CBox_Width", false);
	if (prop.IsValid())
	{
		tempCustom.width = prop.Get<float>();
	}
	else
	{
		tempCustom.width = -1.0f;
	}

	prop = currentNode->FindProperty("CenterPivotX", false);
	if (prop.IsValid())
	{
		tempCustom.centerPivot[0] = prop.Get<float>();
	}
	else
	{
		tempCustom.centerPivot[0] = -999.0f;
	}

	prop = currentNode->FindProperty("CenterPivotY", false);
	if (prop.IsValid())
	{
		tempCustom.centerPivot[1] = prop.Get<float>();
	}
	else
	{
		tempCustom.centerPivot[1] = -999.0f;
	}

	prop = currentNode->FindProperty("CenterPivotZ", false);
	if (prop.IsValid())
	{
		tempCustom.centerPivot[2] = prop.Get<float>();
	}
	else
	{
		tempCustom.centerPivot[2] = -999.0f;
	}

	customMayaAttribute.push_back(tempCustom);
	counter.customMayaAttributeCount++;
}
//----------------------------------------------------------------------------------------------------------------------------------------------------------------
void Converter::createCustomFile()
{
	size_t len = strlen(meshName);
	ret = new char[len + 2];
	strcpy(ret, meshName);
	ret[len - 3] = 's';
	ret[len - 2] = 's';
	ret[len - 1] = 'p';
	ret[len] = '\0';
	meshName = ret;

	/*tempMeshName = (char*)meshInfo[0].meshName;
	char tempFileName[5] = ".ssp";
	strcat(tempMeshName, tempFileName);*/

	std::ofstream outfile(meshName, std::ofstream::binary);

	outfile.write((const char*)&counter, sizeof(Counter));

	for (int i = 0; i < meshInfo.size(); i++)
	{
		outfile.write((const char*)&meshInfo[i].meshName, sizeof(char) * 100);
		outfile.write((const char*)&meshInfo[i].globalTranslation, sizeof(float) * 3);
		outfile.write((const char*)&meshInfo[i].globalRotation, sizeof(float) * 3);
		outfile.write((const char*)&meshInfo[i].globalScaling, sizeof(float) * 3);

		for (int i = 0; i < vertices.size(); i++)
		{
			outfile.write((const char*)&vertices[i], sizeof(VertexInformation));
		}

		for (int i = 0; i < matInfo.size(); i++)
		{
			outfile.write((const char*)&matInfo[i], sizeof(MaterialInformation));
		}

		for (int i = 0; i < customMayaAttribute.size(); i++)
		{
			outfile.write((const char*)&customMayaAttribute[i], sizeof(CustomMayaAttributes));
		}
	}

	outfile.close();
}
//----------------------------------------------------------------------------------------------------------------------------------------------------------------
void Converter::createCustomLevelFile()
{
	size_t len = strlen(meshName);
	ret = new char[len + 2];
	strcpy(ret, meshName);
	ret[len - 3] = 's';
	ret[len - 2] = 's';
	ret[len - 1] = 'p';
	ret[len] = '\0';
	meshName = ret;

	std::ofstream outfile(meshName, std::ofstream::binary);

	outfile.write((const char*)&counter, sizeof(Counter));

	for (int i = 0; i < levelObjects.size(); i++)
	{
		outfile.write((const char*)&levelObjects[i], sizeof(LevelObject));
	}

	std::cout << "Number of level objects: " << counter.levelObjectCount << std::endl;

	outfile.close();
}
//----------------------------------------------------------------------------------------------------------------------------------------------------------------
void Converter::createCustomAnimationFile()
{
	tempMName = (char*)meshInfo[0].meshName;

	char extraSymbol[2] = "_";
	char animFileName[9] = ".sspAnim";
	char tempAName[10] = {};

	for (int i = 0; i < strlen(animationInfo->animation_name); i++)
	{
		tempAName[i] = animationInfo->animation_name[i];
	}

	strcat(tempMName, extraSymbol);
	strcat(tempMName, tempAName);
	strcat(tempMName, animFileName);

	if (animationInfo->nr_of_joints > 0)
	{
		std::ofstream animOutfile(tempMName, std::ofstream::binary);

		animOutfile.write((const char*)animationInfo->animation_name, sizeof(char) * 9);
		animOutfile.write((const char*)&animationInfo->nr_of_keyframes, sizeof(int));
		animOutfile.write((const char*)&animationInfo->nr_of_joints, sizeof(int));

		animOutfile.write((const char*)&animationInfo->current_time, sizeof(float));
		animOutfile.write((const char*)&animationInfo->max_time, sizeof(float));
		animOutfile.write((const char*)&animationInfo->looping, sizeof(bool));
		animOutfile.write((const char*)&animationInfo->switching, sizeof(bool));

		for (int i = 0; i < animationInfo->nr_of_joints; i++)
		{
			animOutfile.write((const char*)&animationInfo->joints[i].joint_name, sizeof(char) * 100);
			animOutfile.write((const char*)&animationInfo->joints[i].parent_name, sizeof(char) * 100);
			
			animOutfile.write((const char*)&animationInfo->joints[i].joint_id, sizeof(int));
			animOutfile.write((const char*)&animationInfo->joints[i].parent_id, sizeof(int));
			
			animOutfile.write((const char*)&animationInfo->joints[i].local_transform_matrix, sizeof(float) * 16);
			animOutfile.write((const char*)&animationInfo->joints[i].bind_pose_matrix, sizeof(float) * 16);
			
			animOutfile.write((const char*)&animationInfo->joints[i].translation, sizeof(float) * 3);
			animOutfile.write((const char*)&animationInfo->joints[i].rotation, sizeof(float) * 3);
			animOutfile.write((const char*)&animationInfo->joints[i].scale, sizeof(float) * 3);
			
			animOutfile.write((const char*)animationInfo->joints[i].keyFrames.data(), sizeof(KeyFrame) * animationInfo->nr_of_keyframes);
		}
		animOutfile.close();
	}
}
//----------------------------------------------------------------------------------------------------------------------------------------------------------------
void Converter::exportAnimation(FbxScene * scene, FbxNode* node)
{
	animationInfo = new Animation[1];
	animationInfo->nr_of_joints = 0;
	//GetSrcObjectCount: Returns the number of source objects with which this object connects. 
	for (int i = 0; i < scene->GetSrcObjectCount<FbxAnimStack>(); i++)
	{
		//AnimStack: The Animation stack is a collection of animation layers.
		//GetSrcObject: Returns the source object with which this object connects at the specified index.
		FbxAnimStack* animStack = scene->GetSrcObject<FbxAnimStack>(i);
		
		/*const char* tempAnimName = animStack->GetInitialName();
		for (int n = 0; n < strlen(tempAnimName) + 1; n++)
			animationInfo->animation_name[n] = tempAnimName[n];*/
		std::cout << "=====================" << std::endl << "FBX File has animation!" << std::endl << "=====================" << std::endl << std::endl;
		char animation_name_input[9];
		std::cout << "Enter animation name (max 9 characters): ";
		std::cin >> animation_name_input;
		for(int n = 0; n < strlen(animation_name_input) +1; n++)
			animationInfo->animation_name[n] = animation_name_input[n];
		std::cout << "Name Saved." << std::endl << std::endl;

		char answer;
		bool is_looping = false;
		std::cout << "Looping animation (y/n): ";
		std::cin >> answer;
		if (answer == 'y' || answer == 'Y')
			is_looping = true;
		animationInfo->looping = is_looping;
		std::cout << "Looping set to " << animationInfo->looping << ". (1 = true, 0 = false) " << std::endl << std::endl;

		bool is_switching = false;
		std::cout << "Switching animation (y/n): ";
		std::cin >> answer;
		if (answer == 'y' || answer == 'Y')
			is_switching = true;
		animationInfo->switching = is_switching;
		std::cout << "Switching set to " << animationInfo->switching << ". (1 = true, 0 = false) " << std::endl << std::endl;

		float leons_number = 1.1;
		animationInfo->current_time = leons_number;

		//AnimLayer: The animation layer is a collection of animation curve nodes. 
		//GetMemberCount: Returns the number of objects contained within the collection.
		int animLayers = animStack->GetMemberCount<FbxAnimLayer>();

		if (animLayers != 0)
		{
			for (int j = 0; j < animLayers; j++) // j = 1 to skip BaseLayer/BaseAnimation
			{
				//GetMember: Returns the member of the collection at the given index.
				currentJointIndex = 0;
				FbxAnimLayer* currentAnimLayer = animStack->GetMember<FbxAnimLayer>(j);
				getAnimation(currentAnimLayer, node, scene);
			}
		}
		fixJointID();
		std::cout << "=====================" << std::endl << "Animation saved" << std::endl << "=====================" << std::endl << std::endl;
	}
}
//----------------------------------------------------------------------------------------------------------------------------------------------------------------
void Converter::getAnimation(FbxAnimLayer* animLayer, FbxNode* node, FbxScene* scene)
{
	int modelCount;
	const char* baseCheck;

	getAnimationChannels(node, animLayer, scene);

	for (modelCount = 0; modelCount < node->GetChildCount(); modelCount++)
	{
		//std::cout << node->GetChild(modelCount)->GetName() << std::endl;
		getAnimation(animLayer, node->GetChild(modelCount), scene);
	}
}
//----------------------------------------------------------------------------------------------------------------------------------------------------------------
void Converter::getAnimationChannels(FbxNode* node, FbxAnimLayer* animLayer, FbxScene* scene)
{
	//AnimCurve: An animation curve, defined by a collection of keys (FbxAnimCurveKey), and indicating how a value changes over time. 
	FbxAnimCurve* animCurve = NULL;
	//FbxString outputString;

	float keyValue;
	int keyCount;
	double keyTime;

	std::vector<float> tempPosition;
	std::vector<float> tempRotation;
	std::vector<float> tempScaling;
	
	//LclTranslation: This property contains the translation information of the node.
	//animCurve = node->LclTranslation.GetCurve(animLayer, FBXSDK_CURVENODE_COMPONENT_X);
	animCurve = node->LclTranslation.GetCurve(animLayer);
	if (node->LclTranslation.GetCurve(animLayer) != nullptr || node->LclRotation.GetCurve(animLayer) != nullptr || node->LclScaling.GetCurve(animLayer) != nullptr)
	{
		Joint jointInformation;
		//STORE: JointInformation char jointName[]
		const char* tempJointName = node->GetName();
		for (unsigned int n = 0; n < strlen(tempJointName) + 1; n++)
			jointInformation.joint_name[n] = tempJointName[n];

		//STORE: JointInformation char parentName[]
		const char* tempJointParentName = node->GetParent()->GetName();
		for (unsigned int n = 0; n < strlen(tempJointParentName) + 1; n++)
			jointInformation.parent_name[n] = tempJointParentName[n];

		jointInformation.joint_id = currentJointIndex;
		jointInformation.parent_id = - 1;
		currentJointIndex++;

		for (int i = 0; i < 3; i++)
		{
			jointInformation.translation[i] = 1;
			jointInformation.rotation[i] = 1;
			jointInformation.scale[i] = 1;
		}

		FbxMatrix tempTransform = node->EvaluateLocalTransform(FBXSDK_TIME_INFINITE);

		for (int k = 0; k < 4; k++)
		{
			for (int l = 0; l < 4; l++)
			{
				jointInformation.local_transform_matrix[k][l] = tempTransform[k][l];
				jointInformation.bind_pose_matrix[k][l] = tempTransform[k][l];
			}
		}

		keyCount = animCurve->KeyGetCount();
		//STORE: AnimationInformation int keyFrameCount
		animationInfo->nr_of_keyframes = keyCount;

		for (int j = 0; j < keyCount; j++)
		{
			keyTime = animCurve->KeyGetTime(j).GetSecondDouble();

			if (j == (keyCount - 1))
				animationInfo->max_time = keyTime;

			tempPosition.clear();
			tempRotation.clear();
			tempScaling.clear();

			animCurve = node->LclTranslation.GetCurve(animLayer, FBXSDK_CURVENODE_COMPONENT_X);
			keyValue = static_cast<float>(animCurve->KeyGetValue(j));
			tempPosition.push_back(keyValue);

			animCurve = node->LclTranslation.GetCurve(animLayer, FBXSDK_CURVENODE_COMPONENT_Y);
			keyValue = static_cast<float>(animCurve->KeyGetValue(j));
			tempPosition.push_back(keyValue);

			animCurve = node->LclTranslation.GetCurve(animLayer, FBXSDK_CURVENODE_COMPONENT_Z);
			keyValue = static_cast<float>(animCurve->KeyGetValue(j));
			tempPosition.push_back(keyValue);


			animCurve = node->LclRotation.GetCurve(animLayer, FBXSDK_CURVENODE_COMPONENT_X);
			keyValue = static_cast<float>(animCurve->KeyGetValue(j));
			tempRotation.push_back(keyValue);

			animCurve = node->LclRotation.GetCurve(animLayer, FBXSDK_CURVENODE_COMPONENT_Y);
			keyValue = static_cast<float>(animCurve->KeyGetValue(j));
			tempRotation.push_back(keyValue);

			animCurve = node->LclRotation.GetCurve(animLayer, FBXSDK_CURVENODE_COMPONENT_Z);
			keyValue = static_cast<float>(animCurve->KeyGetValue(j));
			tempRotation.push_back(keyValue);


			animCurve = node->LclScaling.GetCurve(animLayer, FBXSDK_CURVENODE_COMPONENT_X);
			keyValue = static_cast<float>(animCurve->KeyGetValue(j));
			tempScaling.push_back(keyValue);

			animCurve = node->LclScaling.GetCurve(animLayer, FBXSDK_CURVENODE_COMPONENT_Y);
			keyValue = static_cast<float>(animCurve->KeyGetValue(j));
			tempScaling.push_back(keyValue);

			animCurve = node->LclScaling.GetCurve(animLayer, FBXSDK_CURVENODE_COMPONENT_Z);
			keyValue = static_cast<float>(animCurve->KeyGetValue(j));
			tempScaling.push_back(keyValue);

			KeyFrame tempKeyFrameData;
			tempKeyFrameData.time = keyTime;
			tempKeyFrameData.translation[0] = tempPosition[0];
			tempKeyFrameData.translation[1] = tempPosition[1];
			tempKeyFrameData.translation[2] = tempPosition[2];

			tempKeyFrameData.rotation[0] = tempRotation[0];
			tempKeyFrameData.rotation[1] = tempRotation[1];
			tempKeyFrameData.rotation[2] = tempRotation[2];

			tempKeyFrameData.scaling[0] = tempScaling[0];
			tempKeyFrameData.scaling[1] = tempScaling[1];
			tempKeyFrameData.scaling[2] = tempScaling[2];

			jointInformation.keyFrames.push_back(tempKeyFrameData);
		}
		
		animationInfo->joints.push_back(jointInformation);
		animationInfo->nr_of_joints++;
	}
}
//----------------------------------------------------------------------------------------------------------------------------------------------------------------
void Converter::fixJointID()
{
	for (int i = 0; i < animationInfo->joints.size(); i++)
	{
		for (int k = 0; k < animationInfo->joints.size(); k++)
		{
			if (strcmp(animationInfo->joints[i].parent_name, animationInfo->joints[k].joint_name) == 0)
			{
				animationInfo->joints[i].parent_id = k;
			}
		}
	}
}
//----------------------------------------------------------------------------------------------------------------------------------------------------------------
void Converter::loadLevel(FbxNode * currentNode)
{
	printf("\n\t|| Node: %s\n", currentNode->GetName());

	mesh = currentNode->GetMesh();

	if (currentNode)
	{
		if (mesh)// && !isPartOf(currentNode->GetName()))
		{
			LevelObject lvlObj = LevelObject();
			FbxDouble3 tempTranslation = currentNode->LclTranslation.Get();
			FbxDouble3 tempRotation = currentNode->LclRotation.Get();
			// Save position
			lvlObj.x = (float)tempTranslation[0];
			lvlObj.y = (float)tempTranslation[1];
			lvlObj.z = (float)tempTranslation[2];
			// Save rotation
			lvlObj.rotationX = (float)tempRotation[0];
			lvlObj.rotationY = (float)tempRotation[1];
			lvlObj.rotationZ = (float)tempRotation[2];

			//FBXSDK_printf("\t|| Translation: %f %f %f\n", tempTranslation[0], tempTranslation[1], tempTranslation[2]);
			//FBXSDK_printf("\t|| Rotation: %f %f %f\n", meshInfo->globalRotation[0], meshInfo->globalRotation[1], meshInfo->globalRotation[2]);

			// Save ID
			unsigned int attributeValue;
			//std::string attributeName = "";

			FbxProperty prop = currentNode->FindProperty("ID", false);
			if (prop.IsValid())
			{
				//attributeName = prop.GetName();
				attributeValue = prop.Get<int>();

				//FBXSDK_printf("|| Mesh ID: %s\n", attributeName.c_str());
				FBXSDK_printf("\t|| ID: %d\n", attributeValue);
				lvlObj.id = prop.Get<int>();
			}
			else
			{
				lvlObj.id = -1;
			}
			levelObjects.push_back(lvlObj);
			counter.levelObjectCount++;
		}
	}
	else
	{
		printf("Access violation: Node not found\n\n");
		exit(-2);
	}
}