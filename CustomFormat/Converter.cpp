#include "Converter.h"
#include <fstream>
#include <vector>
#include <string>

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
}
//----------------------------------------------------------------------------------------------------------------------------------------------------------------
Converter::~Converter()
{
	//delete[] objectBlendShapes;
	delete ret;

	delete[] animationInfo;

	meshInfo.clear();
	vertices.clear();
	matInfo.clear();
	//exportCamera.clear();
	//exportLight.clear();
	customMayaAttribute.clear();
	//groups.clear();
	
	ourScene->Destroy();
	settings->Destroy();
	manager->Destroy();
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
			exportFile(rootNode->GetChild(i));
		}
	}
	
	counter.vertexCount = totalNrOfVertices;
	exportAnimation(ourScene, rootNode);
	//printInfo();
	createCustomFile();
}
//----------------------------------------------------------------------------------------------------------------------------------------------------------------
void Converter::exportFile(FbxNode* currentNode)
{
	//printf("\nNode: %s\n", currentNode->GetName());

	/*if (!isLevel)
		loadGlobaltransform(currentNode);*/
	
	mesh = currentNode->GetMesh();
	light = currentNode->GetLight();
	camera = currentNode->GetCamera();

	if (currentNode)
	{
		if (mesh)
		{
			loadGlobaltransform(currentNode);
			loadVertex(mesh, currentNode);
			loadMaterial(currentNode);
			//loadBlendShape(mesh, ourScene);
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
/*void Converter::loadBlendShape(FbxMesh* currentMesh, FbxScene* scene)
{
	FbxAnimStack* animStack = scene->GetSrcObject<FbxAnimStack>();

	int animLayers = animStack->GetMemberCount<FbxAnimLayer>();

	FbxString lOutputString;

	int blendShapeDeformerCount = mesh->GetDeformerCount(FbxDeformer::eBlendShape);

	if (blendShapeDeformerCount != 0)
	{
		objectBlendShapes = new BlendShapes[blendShapeDeformerCount];

		for (int animLayerIndex = 0; animLayerIndex < animLayers; animLayerIndex++)
		{
			FbxAnimLayer* animLayer = animStack->GetMember<FbxAnimLayer>(animLayerIndex);

			counter.blendShapeCount = blendShapeDeformerCount;

			//Create Blendshapes
			for (int blendShapeIndex = 0; blendShapeIndex < blendShapeDeformerCount; ++blendShapeIndex)
			{
				FbxBlendShape* blendShape = (FbxBlendShape*)mesh->GetDeformer(blendShapeIndex, FbxDeformer::eBlendShape);
				int blendShapeChannelCount = blendShape->GetBlendShapeChannelCount();

				//Setting blendshapeCount for animation
				objectBlendShapes[0].blendShapeCount = blendShapeChannelCount;

				BlendShape tempBlendShape;


				for (int channelIndex = 0; channelIndex < blendShapeChannelCount; ++channelIndex)
				{
					FbxBlendShapeChannel* channel = blendShape->GetBlendShapeChannel(channelIndex);
					const char* channelName = channel->GetName();

					FbxShape* shape = channel->GetTargetShape(0);

					int blendShapeControlPointsCount = shape->GetControlPointsCount();

					tempBlendShape.blendShapeVertexCount = blendShapeControlPointsCount;

					blendShapeControlPoints = shape->GetControlPoints();

					FbxVector4 bPos;
					FbxVector4 bNormal;

					FbxGeometry* geom = blendShape->GetGeometry();

					FbxLayerElementNormal* pLayerNormals = geom->GetLayer(channelIndex)->GetNormals();

					BlendShapeVertex tempVertex;
					BlendShapeKeyframe tempKeyframe;

					for (int blendshapeControlIndex = 0; blendshapeControlIndex < blendShapeControlPointsCount; blendshapeControlIndex++)
					{
						bPos = blendShapeControlPoints[blendshapeControlIndex];
						bNormal = pLayerNormals->GetDirectArray().GetAt(blendshapeControlIndex);

						tempVertex.x = bPos[0];
						tempVertex.y = bPos[1];
						tempVertex.z = bPos[2];
						tempVertex.nx = bNormal[0];
						tempVertex.ny = bNormal[1];
						tempVertex.nz = bNormal[2];

						tempBlendShape.blendShapeVertices.push_back(tempVertex);
					}

					objectBlendShapes[0].blendShape.push_back(tempBlendShape);

					FbxAnimCurve* animCurve = mesh->GetShapeChannel(blendShapeIndex, channelIndex, animLayer, true);
					if (animCurve)
					{
						float keyValue;
						char timeString[256];
						int count;

						int keyCount = animCurve->KeyGetCount();

						objectBlendShapes[0].keyFrameCount = keyCount;

						for (count = 0; count < keyCount; count++)
						{
							keyValue = static_cast<float>(animCurve->KeyGetValue(count));

							tempKeyframe.blendShapeInfluense = keyValue;
							tempKeyframe.time = animCurve->KeyGetTime(count).GetSecondDouble();

							objectBlendShapes[0].keyframes.push_back(tempKeyframe);
						}
					}
				}
			}
		}
	}
}*/

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
//void Converter::printInfo()
//{
//	//...Prints
//	char answer;
//
//	//Counter
//	printf("Print Counter? Y/N:\t");
//	std::cin >> answer;
//
//	if (answer == 'Y' || answer == 'y')
//	{
//		FBXSDK_printf("\n\tVertex Count: %d\n", counter.vertexCount);
//		/*FBXSDK_printf("\tBlend Shape Count: %d\n", counter.blendShapeCount);*/
//		FBXSDK_printf("\tCustom Attribute Count: %d\n", counter.customMayaAttributeCount);
//		/*FBXSDK_printf("\tLight Count: %d\n", counter.lightCount);
//		FBXSDK_printf("\tCamera Count: %d\n", counter.cameraCount);*/
//		FBXSDK_printf("\tMaterial Count: %d\n\n", counter.matCount);
//	}
//
//	//VertexInformation
//	printf("Print VertexInformation? Y/N:\t");
//	std::cin >> answer;
//	if (answer == 'Y' || answer == 'y')
//	{
//		for (int i = 0; i < vertices.size(); i++)
//		{
//			FBXSDK_printf("\n\t|%d|Vertex: %f %f %f\n", i, vertices[i].x, vertices[i].y, vertices[i].z);
//			FBXSDK_printf("\t|%d|Normals: %f %f %f\n", i, vertices[i].nx, vertices[i].ny, vertices[i].nz);
//
//			if (foundBinormal)
//				FBXSDK_printf("\t|%d|Binormals: %f %f %f\n", i, vertices[i].bnx, vertices[i].bny, vertices[i].bnz);
//			if (foundTangent)
//				FBXSDK_printf("\t|%d|Tangents: %f %f %f\n", i, vertices[i].tx, vertices[i].ty, vertices[i].tz);
//
//			FBXSDK_printf("\t|%d|UVs: %f %f\n", i, vertices[i].u, vertices[i].v);
//		
//			for (int j = 0; j < 4; j++)
//			{
//				FBXSDK_printf("\tWeightID[%d]: %f\n", vertices[i].weightID[j], vertices[i].weight[j]);
//			}
//			printf("\n");
//		}
//	}
//
//	//MaterialInformation
//	if (matInfo.size() != 0)
//	{
//		printf("Print MaterialInformation? Y/N:\t");
//		std::cin >> answer;
//		if (answer == 'Y' || answer == 'y')
//		{
//			for (int i = 0; i < matInfo.size(); i++)
//			{
//				FBXSDK_printf("\n\tAmbient: %.2f %.2f %.2f\n", matInfo[i].ambient);
//				FBXSDK_printf("\tDiffuse: %.2f %.2f %.2f\n", matInfo[i].diffuse);
//				FBXSDK_printf("\tEmissive: %.2f %.2f %.2f\n", matInfo[i].emissive);
//				FBXSDK_printf("\tOpacity: %.2f\n", matInfo[i].opacity);
//				FBXSDK_printf("\tTexture File Path: %s\n", matInfo[i].textureFilePath);
//			}
//			printf("\n");
//		}
//	}
//	else
//	{
//		printf("\n\nThere's no Material Information\n\n");
//	}
//
//	//AnimationInformation
//	if (animationInfo)
//	{
//		printf("Print Skeleton Animation? Y/N:\t");
//		std::cin >> answer;
//		if (answer == 'Y' || answer == 'y')
//		{
//			FBXSDK_printf("\n\tAnimation Name: %s\n", animationInfo->animationName);
//			FBXSDK_printf("\tKey Frame Count: %d\n", animationInfo->keyFrameCount);
//			FBXSDK_printf("\tJoint Count: %d\n", animationInfo->nrOfJoints);
//
//
//			printf("Print Joint & Keyframes? Y/N:\t");
//			std::cin >> answer;
//
//			if (answer == 'Y' || answer == 'y')
//			{
//				for (int i = 0; i < animationInfo->joints.size(); i++)
//				{
//					FBXSDK_printf("\n\tJoint Name: %s\n", animationInfo->joints[i].jointName);
//					FBXSDK_printf("\tParent Name: %s\n", animationInfo->joints[i].parentName);
//
//					for (int j = 0; j < animationInfo->joints[i].keyFrames.size(); j++)
//					{
//						FBXSDK_printf("\t\tKey|%d| Time:     %.3f", j, animationInfo->joints[i].keyFrames[j].time);
//						FBXSDK_printf("\tPosition: %.3f %.3f %.3f", j, animationInfo->joints[i].keyFrames[j].position[0], animationInfo->joints[i].keyFrames[j].position[1], animationInfo->joints[i].keyFrames[j].position[2]);
//						FBXSDK_printf("\tRotation: %.3f %.3f %.3f", j, animationInfo->joints[i].keyFrames[j].rotation[0], animationInfo->joints[i].keyFrames[j].rotation[1], animationInfo->joints[i].keyFrames[j].rotation[2]);
//						FBXSDK_printf("\tScaling:  %.3f %.3f %.3f\n", j,  animationInfo->joints[i].keyFrames[j].scaling[0] , animationInfo->joints[i].keyFrames[j].scaling[1] , animationInfo->joints[i].keyFrames[j].scaling[2]);
//					}
//				}
//			}
//			printf("\n");
//		}
//	}
//	else
//	{
//		printf("\n\nThere's no Skeleton Animation\n\n");
//	}
//
//	//BlendShapes
//	if (objectBlendShapes->blendShapeCount != 0)
//	{
//		printf("Print BlendShapes? Y/N:\t");
//		std::cin >> answer;
//		if (answer == 'Y' || answer == 'y')
//		{
//			FBXSDK_printf("\n\tBlend Shape Count: %d", objectBlendShapes->blendShapeCount);
//			FBXSDK_printf("\tKey Frame Count:     %d", objectBlendShapes->keyFrameCount);
//
//			for (int i = 0; i < objectBlendShapes->blendShape.size(); i++)
//			{
//				FBXSDK_printf("\tBlend Shape Vertex Count: %d", objectBlendShapes->blendShape[i].blendShapeVertexCount);
//
//				for (int j = 0; j < objectBlendShapes->blendShape[i].blendShapeVertices.size(); j++)
//				{
//					FBXSDK_printf("\t\tBlend Shape|%d|\tVtx|%d|  X: %f\n", i, j, objectBlendShapes->blendShape[i].blendShapeVertices[j].x);
//					FBXSDK_printf("\t\tBlend Shape|%d|\tVtx|%d|  Y: %f\n", i, j, objectBlendShapes->blendShape[i].blendShapeVertices[j].y);
//					FBXSDK_printf("\t\tBlend Shape|%d|\tVtx|%d|  Z: %f\n", i, j, objectBlendShapes->blendShape[i].blendShapeVertices[j].z);
//					FBXSDK_printf("\t\tBlend Shape|%d|\tVtx|%d| NX: %f\n", i, j, objectBlendShapes->blendShape[i].blendShapeVertices[j].nx);
//					FBXSDK_printf("\t\tBlend Shape|%d|\tVtx|%d| NY: %f\n", i, j, objectBlendShapes->blendShape[i].blendShapeVertices[j].ny);
//					FBXSDK_printf("\t\tBlend Shape|%d|\tVtx|%d| NZ: %f\n", i, j, objectBlendShapes->blendShape[i].blendShapeVertices[j].nz);
//				}
//			}
//			printf("\n");
//		}
//	}
//	else
//	{
//		printf("\n\nThere's no Blend Shape\n\n");
//	}
//
//	//Groups
//	if (groups.size() != 0)
//	{
//		printf("Print Groups? Y/N:\t");
//		std::cin >> answer;
//		if (answer == 'Y' || answer == 'y')
//		{
//			for (int i = 0; i < groups.size(); i++)
//			{
//				FBXSDK_printf("\n\tGroup Name: %s\n", groups[i].groupName);
//				FBXSDK_printf("\tChild Count:   %d\n", groups[i].childCount);
//
//				for (int j = 0; j < groups[i].children.size(); j++)
//				{
//					FBXSDK_printf("\n\tChild|%d| Name: %s\n", i, groups[i].children[j].childName);
//				}
//			}
//
//			printf("\n");
//		}
//	}
//	else
//	{
//		printf("\n\nThere's no Groups\n\n");
//	}
//
//
//	//Custom Maya Attribute
//	if (customMayaAttribute.size() != 0)
//	{
//		printf("Print Custom Maya Attribute? Y/N:\t");
//		std::cin >> answer;
//		if (answer == 'Y' || answer == 'y')
//		{
//			printf("\n");
//			for (int i = 0; i < customMayaAttribute.size(); i++)
//			{
//				FBXSDK_printf("\tMesh Type: %d\n", customMayaAttribute[i].meshType);
//			}
//			printf("\n");
//		}
//	}
//	else
//	{
//		printf("\n\nThere's no Custom Maya Attribute\n\n");
//	}
//
//	//Light
//	if (exportLight.size() != 0)
//	{
//		printf("Print Lights? Y/N:\t");
//		std::cin >> answer;
//		if (answer == 'Y' || answer == 'y')
//		{
//			for (int i = 0; i < exportLight.size(); i++)
//			{
//				FBXSDK_printf("\n\tLight Type: ");
//				switch (exportLight[i].type)
//				{
//				case '0':
//					FBXSDK_printf("Point Light\n");
//					break;
//				case '1':
//					FBXSDK_printf("Directional Light\n");
//					break;
//				case '2':
//					FBXSDK_printf("Spotlight\n");
//					break;
//				default:
//					FBXSDK_printf("Unknown\n");
//					break;
//				}
//
//				FBXSDK_printf("\tColor:   R: %.2f G: %.2f B: %.2f\n", exportLight[i].color[0], exportLight[i].color[1], exportLight[i].color[2]);
//				FBXSDK_printf("\tIntensity:  %.2f\n", exportLight[i].intensity);
//				FBXSDK_printf("\tInner Cone:  %.2f\n", exportLight[i].innerCone);
//				FBXSDK_printf("\tOuter Cone: %.2f\n", exportLight[i].outerCone);
//			}
//			printf("\n");
//		}
//	}
//	else
//	{
//		printf("\n\n\tThere's no Light\n\n");
//	}
//
//	if (exportCamera.size() != 0)
//	{
//		printf("Print Camera? Y/N:\t");
//		std::cin >> answer;
//		getchar();
//		if (answer == 'Y' || answer == 'y')
//		{
//			for (int i = 0; i < exportCamera.size(); i++)
//			{
//				FBXSDK_printf("\n\tPosition:     X: %.2f Y: %.2f Z: %.2f\n", exportCamera[i].position[0], exportCamera[i].position[1], exportCamera[i].position[2]);
//				FBXSDK_printf("\tUp Vector:      X: %.2f Y: %.2f Z: %.2f\n", exportCamera[i].up[0], exportCamera[i].up[1], exportCamera[i].up[2]);
//				FBXSDK_printf("\tForward Vector: X: %.2f Y: %.2f Z: %.2f\n", exportCamera[i].forward[0], exportCamera[i].forward[1], exportCamera[i].forward[2]);
//				FBXSDK_printf("\n\tRoll:         %f\n", exportCamera[i].roll);
//				FBXSDK_printf("\tAspect Width:   %f\n", exportCamera[i].aspectWidth);
//				FBXSDK_printf("\tAspect Height:  %f\n", exportCamera[i].aspectHeight);
//				FBXSDK_printf("\tField of View:  %f\n", exportCamera[i].fov);
//				FBXSDK_printf("\tNear Plane:     %f\n", exportCamera[i].nearPlane);
//				FBXSDK_printf("\tFar Plane:      %f\n", exportCamera[i].farPlane);
//			}
//			printf("\n");
//		}
//	}
//	else
//	{
//		printf("\n\n\tThere's no Camera\n\n");
//	}
//}
//----------------------------------------------------------------------------------------------------------------------------------------------------------------
void Converter::loadCustomMayaAttributes(FbxNode * currentNode)
{
	CustomMayaAttributes tempCustom;		//Kolla så man får in alla värden; x, y och z;

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
		tempCustom.id = 0;
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

		if (animationInfo->nrOfJoints > 0)
		{
			//Skeletal animation
			outfile.write((const char*)animationInfo->animationName, sizeof(char) * 9);
			outfile.write((const char*)&animationInfo->keyFrameCount, sizeof(int));
			outfile.write((const char*)&animationInfo->nrOfJoints, sizeof(int));
			for (int i = 0; i < animationInfo->nrOfJoints; i++)
			{
				size_t jLen = strlen(animationInfo->joints[i].jointName);
				size_t pLen = strlen(animationInfo->joints[i].parentName);
				outfile.write((const char*)&animationInfo->joints[i].jointName, sizeof(char) * 100);
				outfile.write((const char*)&animationInfo->joints[i].parentName, sizeof(char) * 100);

				outfile.write((const char*)animationInfo->joints[i].keyFrames.data(), sizeof(KeyFrame) * animationInfo->keyFrameCount);
			}
		}

		//Morph animation/Blend shapes
		/*outfile.write((const char*)objectBlendShapes, 2 * sizeof(float));
		for (int i = 0; i < counter.blendShapeCount; i++)
		{
		outfile.write((const char*)&objectBlendShapes->blendShape[i].blendShapeVertexCount, sizeof(int));
		outfile.write((const char*)objectBlendShapes->blendShape[i].blendShapeVertices.data(), sizeof(BlendShapeVertex) * objectBlendShapes->blendShape[i].blendShapeVertexCount);
		}
		outfile.write((const char*)objectBlendShapes->keyframes.data(), sizeof(BlendShapeKeyframe)*objectBlendShapes->keyFrameCount);

		for (int i = 0; i < groups.size(); i++)
		{
		outfile.write((const char*)groups[i].groupName, sizeof(char) * 100);
		outfile.write((const char*)&groups[i].childCount, sizeof(int));
		for (int j = 0; j < groups[i].childCount; j++)
		{
		outfile.write((const char*)&groups[i].children[j].childName, sizeof(char) * 100);
		}
		}*/

		for (int i = 0; i < customMayaAttribute.size(); i++)
		{
			outfile.write((const char*)&customMayaAttribute[i], sizeof(CustomMayaAttributes));
		}
	}


	
	/*for (int i = 0; i < exportLight.size(); i++)
	{
		outfile.write((const char*)&exportLight[i], sizeof(Light));
	}*/
	
	/*for (int i = 0; i < exportCamera.size(); i++)
	{
		outfile.write((const char*)&exportCamera[i], sizeof(Camera));
	}*/

	outfile.close();
}
//----------------------------------------------------------------------------------------------------------------------------------------------------------------
void Converter::exportAnimation(FbxScene * scene, FbxNode* node)
{
	animationInfo = new AnimationInformation[1];
	animationInfo->nrOfJoints = 0;
	//GetSrcObjectCount: Returns the number of source objects with which this object connects. 
	for (int i = 0; i < scene->GetSrcObjectCount<FbxAnimStack>(); i++)
	{
		//AnimStack: The Animation stack is a collection of animation layers.
		//GetSrcObject: Returns the source object with which this object connects at the specified index.
		FbxAnimStack* animStack = scene->GetSrcObject<FbxAnimStack>(i);
		
		//STORE: AnimationInformation char animationName[]
		const char* tempAnimName = animStack->GetInitialName();
		for (int n = 0; n < strlen(tempAnimName) + 1; n++)
			animationInfo->animationName[n] = tempAnimName[n];

		//AnimLayer: The animation layer is a collection of animation curve nodes. 
		//GetMemberCount: Returns the number of objects contained within the collection.
		int animLayers = animStack->GetMemberCount<FbxAnimLayer>();

		if (animLayers != 0)
		{
			for (int j = 0; j < animLayers; j++)
			{
				//GetMember: Returns the member of the collection at the given index. 
				FbxAnimLayer* currentAnimLayer = animStack->GetMember<FbxAnimLayer>(j);
				getAnimation(currentAnimLayer, node);
			}
		}
	}
}
//----------------------------------------------------------------------------------------------------------------------------------------------------------------
void Converter::getAnimation(FbxAnimLayer* animLayer, FbxNode* node)
{
	int modelCount;
	getAnimationChannels(node, animLayer);

	for (modelCount = 0; modelCount < node->GetChildCount(); modelCount++)
	{
		getAnimation(animLayer, node->GetChild(modelCount));
	}
}
//----------------------------------------------------------------------------------------------------------------------------------------------------------------
void Converter::getAnimationChannels(FbxNode* node, FbxAnimLayer* animLayer)
{
	//AnimCurve: An animation curve, defined by a collection of keys (FbxAnimCurveKey), and indicating how a value changes over time. 
	FbxAnimCurve* animCurve = NULL;
	FbxString outputString;

	float keyValue;
	int keyCount;
	double keyTime;

	std::vector<float> tempPosition;
	std::vector<float> tempRotation;
	std::vector<float> tempScaling;
	
	//LclTranslation: This property contains the translation information of the node.
	animCurve = node->LclTranslation.GetCurve(animLayer, FBXSDK_CURVENODE_COMPONENT_X);
	if (animCurve)
	{
		JointInformation jointInformation;
		//STORE: JointInformation char jointName[]
		const char* tempJointName = node->GetName();
		for (unsigned int n = 0; n < strlen(tempJointName) + 1; n++)
			jointInformation.jointName[n] = tempJointName[n];

		//STORE: JointInformation char parentName[]
		const char* tempJointParentName = node->GetParent()->GetName();
		for (unsigned int n = 0; n < strlen(tempJointParentName) + 1; n++)
			jointInformation.parentName[n] = tempJointParentName[n];

		keyCount = animCurve->KeyGetCount();
		//STORE: AnimationInformation int keyFrameCount
		animationInfo->keyFrameCount = keyCount;

		for (int j = 0; j < keyCount; j++)
		{
			keyTime = animCurve->KeyGetTime(j).GetSecondDouble();
			//STORE: KeyFrame float time

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
			tempKeyFrameData.position[0] = tempPosition[0];
			tempKeyFrameData.position[1] = tempPosition[1];
			tempKeyFrameData.position[2] = tempPosition[2];

			tempKeyFrameData.rotation[0] = tempRotation[0];
			tempKeyFrameData.rotation[1] = tempRotation[1];
			tempKeyFrameData.rotation[2] = tempRotation[2];

			tempKeyFrameData.scaling[0] = tempScaling[0];
			tempKeyFrameData.scaling[1] = tempScaling[1];
			tempKeyFrameData.scaling[2] = tempScaling[2];

			jointInformation.keyFrames.push_back(tempKeyFrameData);
		}
		animationInfo->joints.push_back(jointInformation);
		animationInfo->nrOfJoints += 1;
	}
}
//----------------------------------------------------------------------------------------------------------------------------------------------------------------