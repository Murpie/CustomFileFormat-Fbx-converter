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

	//counter.boundingBoxCount = 0;
	//counter.levelObjectCount = 0;
}
//----------------------------------------------------------------------------------------------------------------------------------------------------------------
Converter::~Converter()
{
	delete[] objectBlendShapes;
	delete ret;

	delete[] animationInfo;

	meshInfo.clear();
	vertices.clear();
	matInfo.clear();
	exportCamera.clear();
	exportLight.clear();
	customMayaAttribute.clear();
	groups.clear();
	
	ourScene->Destroy();
	settings->Destroy();
	manager->Destroy();
	//vBBox.clear();
	//levelObjects.clear();
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
	
	exportAnimation(ourScene, rootNode);
	//Create the Custom File
	//printInfo();
	createCustomFile();
}
//----------------------------------------------------------------------------------------------------------------------------------------------------------------
void Converter::exportFile(FbxNode* currentNode)
{
	//printf("\nNode: %s\n", currentNode->GetName());

	if (!isLevel)
		loadGlobaltransform(currentNode);
	
	mesh = currentNode->GetMesh();
	light = currentNode->GetLight();
	camera = currentNode->GetCamera();

	if (currentNode)
	{
		printf("Node: %s\n", currentNode->GetName());

		if ((std::string)currentNode->GetName() == "Group")
		{
			for (int i = 0; i < currentNode->GetChildCount(); i++)
			{
				FbxNode* tempNode = currentNode->GetChild(i);
				printf("Node: %s\n", tempNode->GetName());

				mesh = tempNode->GetMesh();

				if (mesh)
				{
					loadVertex(mesh, tempNode);
					loadMaterial(tempNode);
					loadCustomMayaAttributes(tempNode);
				}
				
			}
		}
		//Load in Vertex data
		else
		{
			if (mesh)
			{
				loadVertex(mesh, currentNode);
				loadMaterial(currentNode);
				loadBlendShape(mesh, ourScene);
				loadCustomMayaAttributes(currentNode);
			}
		}

		//Load Cameras
		if (camera)
		{
			loadCamera(camera);
		}

		//Load Lights
		if (light)
		{
			loadLights(light);
		}

		//Groups
		if (currentNode)
		{
			loadGroups(currentNode);
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
	FbxDouble3 tempTranslation = currentNode->LclTranslation.Get();
	FbxDouble3 tempRotation = currentNode->LclRotation.Get();
	FbxDouble3 tempScaling = currentNode->LclScaling.Get();

	for (int i = 0; i < COLOR_RANGE; i++)
	{
		tempMeshInfo.globalTranslation[i] = tempTranslation[i];
		tempMeshInfo.globalRotation[i] = tempRotation[i];
		tempMeshInfo.globalScaling[i] = tempScaling[i];
	}

	//FBXSDK_printf("Translation: %f %f %f\n", meshInfo->globalTranslation[0], meshInfo->globalTranslation[1], meshInfo->globalTranslation[2]);
	//FBXSDK_printf("Rotation: %f %f %f\n", meshInfo->globalRotation[0], meshInfo->globalRotation[1], meshInfo->globalRotation[2]);
	//FBXSDK_printf("Scaling: %f %f %f\n\n", meshInfo->globalScaling[0], meshInfo->globalScaling[1], meshInfo->globalScaling[2]);
	
	meshInfo.push_back(tempMeshInfo);
}
//----------------------------------------------------------------------------------------------------------------------------------------------------------------
void Converter::loadVertex(FbxMesh* currentMesh, FbxNode* currentNode)
{
	polygonCount = currentMesh->GetPolygonCount();

	//Vertices
	controlPoints = currentMesh->GetControlPoints();

	counter.vertexCount = polygonCount * 3;

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
				//reference mode is direct, the normal index is same as vertex index.
				//get normals by the index of control vertex
				if (vertexBinormal->GetReferenceMode() == FbxGeometryElement::eDirect)
					lNormalIndex = vertexIndex;
				//reference mode is index-to-direct, get normals by the index-to-direct
				if (vertexBinormal->GetReferenceMode() == FbxGeometryElement::eIndexToDirect)
					lNormalIndex = vertexBinormal->GetIndexArray().GetAt(vertexIndex);
				//Got normals of each vertex.
				FbxVector4 lNormal = vertexBinormal->GetDirectArray().GetAt(lNormalIndex);
				//FBXSDK_printf("Binormals for vertex[%d]: %f %f %f %f \n", lVertexIndex, lNormal[0], lNormal[1], lNormal[2], lNormal[3]);
				//add your custom code here, to output normals or get them into a list, such as KArrayTemplate<FbxVector4>
				//. . .
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
				//reference mode is direct, the normal index is same as vertex index.
				//get normals by the index of control vertex
				if (vertexTangent->GetReferenceMode() == FbxGeometryElement::eDirect)
					lNormalIndex = vertexIndex;
				//reference mode is index-to-direct, get normals by the index-to-direct
				if (vertexTangent->GetReferenceMode() == FbxGeometryElement::eIndexToDirect)
					lNormalIndex = vertexTangent->GetIndexArray().GetAt(vertexIndex);
				//Got normals of each vertex.
				FbxVector4 lNormal = vertexTangent->GetDirectArray().GetAt(lNormalIndex);
				//FBXSDK_printf("Tangents for vertex[%d]: %f %f %f %f \n", lVertexIndex, lNormal[0], lNormal[1], lNormal[2], lNormal[3]);
				//add your custom code here, to output normals or get them into a list, such as KArrayTemplate<FbxVector4>
				//. . .
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
			//std::cout << "\nMaterial name: " << lMaterial->GetName() << std::endl << std::endl;

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

			/*lString = nullptr;
			lString = "            Ambient: ";
			lString += (float)ambient.mRed;
			lString += " (red), ";
			lString += (float)ambient.mGreen;
			lString += " (green), ";
			lString += (float)ambient.mBlue;
			lString += " (blue), ";
			lString += "";
			lString += "\n\n";
			FBXSDK_printf(lString);

			lString = nullptr;
			lString = "            Diffuse: ";
			lString += (float)diffuse.mRed;
			lString += " (red), ";
			lString += (float)diffuse.mGreen;
			lString += " (green), ";
			lString += (float)diffuse.mBlue;
			lString += " (blue), ";
			lString += "";
			lString += "\n\n";
			FBXSDK_printf(lString);


			lString = nullptr;
			lString = "            Emissive: ";
			lString += (float)emissive.mRed;
			lString += " (red), ";
			lString += (float)emissive.mGreen;
			lString += " (green), ";
			lString += (float)emissive.mBlue;
			lString += " (blue), ";
			lString += "";
			lString += "\n\n";
			FBXSDK_printf(lString);

			lString = nullptr;
			FbxString lFloatValue = (float)transparency;
			lFloatValue = transparency <= -HUGE_VAL ? "-INFINITY" : lFloatValue.Buffer();
			lFloatValue = transparency >= HUGE_VAL ? "INFINITY" : lFloatValue.Buffer();
			lString = "            Opacity: ";
			lString += lFloatValue;
			lString += "";
			lString += "\n\n";
			FBXSDK_printf(lString);*/

			//File Texture path from Material
			FbxProperty fileTextureProp = lMaterial->FindProperty(FbxSurfaceMaterial::sDiffuse);

			if (fileTextureProp != NULL)
			{
				textureCount = fileTextureProp.GetSrcObjectCount<FbxFileTexture>();
				for (int i = 0; i < textureCount; i++)
				{
					FbxFileTexture* texture = FbxCast<FbxFileTexture>(fileTextureProp.GetSrcObject<FbxFileTexture>(i));

					textureName = texture->GetFileName();

					//FBXSDK_printf("Texture file found!\nPath: %s\n", textureName);
				}
			}
		}
	}

	//Create custom materialInformation
	//matInfo = new MaterialInformation[1];
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
void Converter::loadBlendShape(FbxMesh* currentMesh, FbxScene* scene)
{
	FbxAnimStack* animStack = scene->GetSrcObject<FbxAnimStack>();

	int animLayers = animStack->GetMemberCount<FbxAnimLayer>();

	FbxString lOutputString;

	int blendShapeDeformerCount = mesh->GetDeformerCount(FbxDeformer::eBlendShape);

	if (blendShapeDeformerCount != 0)
	{
		objectBlendShapes = new BlendShapes[blendShapeDeformerCount];

		//lOutputString = "	Contains: ";
		//if (animLayers == 0)
		//	lOutputString += "no layers";

		//if (animLayers)
		//{
		//	lOutputString += animLayers;
		//	lOutputString += " Animation Layer\n";
		//}
		////FBXSDK_printf(lOutputString);

		for (int animLayerIndex = 0; animLayerIndex < animLayers; animLayerIndex++)
		{
			FbxAnimLayer* animLayer = animStack->GetMember<FbxAnimLayer>(animLayerIndex);

			counter.blendShapeCount = blendShapeDeformerCount;
			//cout << "	Number of blendshapes Animations: " << counter.blendShapeCount << endl;

			//Create Blendshapes


			for (int blendShapeIndex = 0; blendShapeIndex < blendShapeDeformerCount; ++blendShapeIndex)
			{
				FbxBlendShape* blendShape = (FbxBlendShape*)mesh->GetDeformer(blendShapeIndex, FbxDeformer::eBlendShape);
				int blendShapeChannelCount = blendShape->GetBlendShapeChannelCount();

				//Setting blendshapeCount for animation
				objectBlendShapes[0].blendShapeCount = blendShapeChannelCount;

				//cout << "	Number of blendshapes for this Animation: " << objectBlendShapes[0].blendShapeCount << endl;

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

						/*FBXSDK_printf("	BlendShape Vertex: %d\n", blendshapeControlIndex);
						FBXSDK_printf("		x = %f", tempBlendShape.blendShapeVertices[blendshapeControlIndex].x);
						FBXSDK_printf(" y = %f", tempBlendShape.blendShapeVertices[blendshapeControlIndex].y);
						FBXSDK_printf(" z = %f\n", tempBlendShape.blendShapeVertices[blendshapeControlIndex].z);

						FBXSDK_printf("		nx = %f", tempBlendShape.blendShapeVertices[blendshapeControlIndex].nx);
						FBXSDK_printf(" ny = %f", tempBlendShape.blendShapeVertices[blendshapeControlIndex].ny);
						FBXSDK_printf(" nz = %f\n", tempBlendShape.blendShapeVertices[blendshapeControlIndex].nz);*/
					}

					objectBlendShapes[0].blendShape.push_back(tempBlendShape);

					FbxAnimCurve* animCurve = mesh->GetShapeChannel(blendShapeIndex, channelIndex, animLayer, true);
					if (animCurve)
					{
						//FBXSDK_printf("	Shape Name: %s\n", channelName);

						float keyValue;
						char timeString[256];
						int count;

						int keyCount = animCurve->KeyGetCount();

						objectBlendShapes[0].keyFrameCount = keyCount;


						//cout << "	KeyCount: " << objectBlendShapes[0].keyFrameCount << endl;

						for (count = 0; count < keyCount; count++)
						{
							keyValue = static_cast<float>(animCurve->KeyGetValue(count));

							tempKeyframe.blendShapeInfluense = keyValue;
							tempKeyframe.time = animCurve->KeyGetTime(count).GetSecondDouble();

							objectBlendShapes[0].keyframes.push_back(tempKeyframe);

							lOutputString = "	Key Frame: ";
							lOutputString += count;
							lOutputString += "	Key Time: ";
							lOutputString += objectBlendShapes[0].keyframes[count].time;
							lOutputString += "	Blendshape Value: ";
							lOutputString += objectBlendShapes[0].keyframes[count].blendShapeInfluense;
							lOutputString += "\n";
							//FBXSDK_printf(lOutputString);
						}
					}
				}
			}
		}
	}
}
//----------------------------------------------------------------------------------------------------------------------------------------------------------------
void Converter::loadCamera(FbxCamera* currentNode)
{
	FbxGlobalSettings& globalSettings = ourScene->GetGlobalSettings();
	FbxGlobalCameraSettings& globalCameraSettings = ourScene->GlobalCameraSettings();
	FbxString currentCameraName = globalSettings.GetDefaultCamera();

	if (currentCameraName.Compare(FBXSDK_CAMERA_PERSPECTIVE) == 0)
	{
		camera = globalCameraSettings.GetCameraProducerPerspective();
	}
	else
	{
		FbxNode* cameraNode = ourScene->FindNodeByName(currentCameraName);
		if (cameraNode)
		{
			camera = cameraNode->GetCamera();
		}
	}

	if (camera)
	{
		FbxVector4 position;
		FbxVector4 upVector;
		FbxVector4 forwardVector;
		float roll;

		float aspectWidth, aspectHeight;
		float fov;
		float nearPlane, farPlane;

		Camera tempCamera;

		position = currentNode->Position.Get();
		upVector = currentNode->UpVector.Get();
		forwardVector = currentNode->InterestPosition.Get();
		roll = currentNode->Roll.Get();
		aspectWidth = currentNode->AspectWidth.Get();
		aspectHeight = currentNode->AspectHeight.Get();
		fov = currentNode->FieldOfView.Get();
		nearPlane = currentNode->NearPlane.Get();
		farPlane = currentNode->FarPlane.Get();

		for (int i = 0; i < COLOR_RANGE; i++)
		{
			tempCamera.position[i] = position[i];
			tempCamera.up[i] = upVector[i];
			tempCamera.forward[i] = forwardVector[i];
		}

		tempCamera.roll = roll;
		tempCamera.aspectWidth = aspectWidth;
		tempCamera.aspectHeight = aspectHeight;
		tempCamera.fov = fov;
		tempCamera.nearPlane = nearPlane;
		tempCamera.farPlane = farPlane;

		exportCamera.push_back(tempCamera);

		/*FBXSDK_printf("\tPosition: %.2f %.2f %.2f\n", position[0], position[1], position[2]);
		FBXSDK_printf("\tUp: %.2f %.2f %.2f\n", upVector[0], upVector[1], upVector[2]);
		FBXSDK_printf("\tLook At: %.2f %.2f %.2f\n", forwardVector[0], forwardVector[1], forwardVector[2]);
		FBXSDK_printf("\tRoll: %.2f\n", roll);
		FBXSDK_printf("\tAspect Ratio: %.fx%.f\n", aspectWidth, aspectHeight);
		FBXSDK_printf("\tField of View: %.f\n", fov);
		FBXSDK_printf("\tNear Plane: %.2f\n\tFar Plane: %.2f\n\n", nearPlane, farPlane);*/
		counter.cameraCount++;
	}
}
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------
void Converter::loadGroups(FbxNode* currentNode)
{
	if ((std::string)currentNode->GetName() == "Group")
	{
		Group tempGroup;
		GroupChild tempChild;

		const char* tempGroupName = currentNode->GetName();
		for (int i = 0; i < strlen(tempGroupName) + 1; i++)
		{
			tempGroup.groupName[i] = tempGroupName[i];
		}

		//std::cout << "GroupName:" << groups->groupName << std::endl << "Nr of children: " << currentNode->GetChildCount() << endl;

		int childrenSize = 0;
		for (int i = 0; i < currentNode->GetChildCount(); i++)
		{
			const char* tempChildrenName = currentNode->GetChild(i)->GetName();
			for (int j = 0; j < strlen(tempChildrenName) + 1; j++)
			{
				//FBXSDK_printf("%d\t", j);
				tempChild.childName[j] = tempChildrenName[j];
			}

			tempGroup.children.push_back(tempChild);
			childrenSize++;
		}
		tempGroup.childCount = childrenSize;

		groups.push_back(tempGroup);

		/*for (int i = 0; i < groups->childCount; i++)
			std::cout << groups->childName[i] << std::endl;*/
	}
}
//----------------------------------------------------------------------------------------------------------------------------------------------------------------
void Converter::loadLights(FbxLight* currentLight)
{
	//exportLight = new Light[1];

	Light tempLight;

	FbxString lightType = currentLight->LightType.Get();
	FbxDouble3 lightColor = currentLight->Color.Get();
	FbxFloat intensity = currentLight->Intensity.Get();
	FbxFloat innerCone = currentLight->InnerAngle.Get();
	FbxFloat outerCone = currentLight->OuterAngle.Get();
	
	tempLight.type = lightType[0];	//Type only contains a single number

	for (int i = 0; i < COLOR_RANGE; i++)
	{
		tempLight.color[i] = lightColor[i];
	}

	tempLight.intensity = intensity;
	tempLight.innerCone = innerCone;
	tempLight.outerCone = outerCone;

	exportLight.push_back(tempLight);

	//if (lightType == "0")
	//{
	//	//FBXSDK_printf("\tLight Type: Point Light\n");
	//}
	//else if(lightType == "1")
	//{
	//	//FBXSDK_printf("\tLight Type: Directional Light\n");
	//}
	//else if(lightType == "2")
	//{
	//	//FBXSDK_printf("\tLight Type: Spotlight\n");
	//}

	////FBXSDK_printf("\tColor: %.3f %.3f %.3f\n", lightColor[0], lightColor[1], lightColor[2]);
	////FBXSDK_printf("\tIntensity: %.2f\n", intensity);

	//if (lightType == "2")
	//{
	//	//FBXSDK_printf("\tInner Cone: %.2f\n", innerCone);
	//	//FBXSDK_printf("\tOuter Cone: %.2f\n", outerCone);
	//}
	counter.lightCount++;
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
			//FBXSDK_printf("Joint Name: %s\n", cluster->GetLink()->GetName());

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
void Converter::printInfo()
{
	//...Prints
	char answer;

	//Counter
	printf("Print Counter? Y/N:\t");
	std::cin >> answer;

	if (answer == 'Y' || answer == 'y')
	{
		FBXSDK_printf("\n\tVertex Count: %d\n", counter.vertexCount);
		FBXSDK_printf("\tBlend Shape Count: %d\n", counter.blendShapeCount);
		FBXSDK_printf("\tCustom Attribute Count: %d\n", counter.customMayaAttributeCount);
		FBXSDK_printf("\tLight Count: %d\n", counter.lightCount);
		FBXSDK_printf("\tCamera Count: %d\n", counter.cameraCount);
		FBXSDK_printf("\tMaterial Count: %d\n\n", counter.matCount);
	}

	//VertexInformation
	printf("Print VertexInformation? Y/N:\t");
	std::cin >> answer;
	if (answer == 'Y' || answer == 'y')
	{
		for (int i = 0; i < vertices.size(); i++)
		{
			FBXSDK_printf("\n\t|%d|Vertex: %f %f %f\n", i, vertices[i].x, vertices[i].y, vertices[i].z);
			FBXSDK_printf("\t|%d|Normals: %f %f %f\n", i, vertices[i].nx, vertices[i].ny, vertices[i].nz);

			if (foundBinormal)
				FBXSDK_printf("\t|%d|Binormals: %f %f %f\n", i, vertices[i].bnx, vertices[i].bny, vertices[i].bnz);
			if (foundTangent)
				FBXSDK_printf("\t|%d|Tangents: %f %f %f\n", i, vertices[i].tx, vertices[i].ty, vertices[i].tz);

			FBXSDK_printf("\t|%d|UVs: %f %f\n", i, vertices[i].u, vertices[i].v);
		
			for (int j = 0; j < 4; j++)
			{
				FBXSDK_printf("\tWeightID[%d]: %f\n", vertices[i].weightID[j], vertices[i].weight[j]);
			}
			printf("\n");
		}
	}

	//MaterialInformation
	if (matInfo.size() != 0)
	{
		printf("Print MaterialInformation? Y/N:\t");
		std::cin >> answer;
		if (answer == 'Y' || answer == 'y')
		{
			for (int i = 0; i < matInfo.size(); i++)
			{
				FBXSDK_printf("\n\tAmbient: %.2f %.2f %.2f\n", matInfo[i].ambient);
				FBXSDK_printf("\tDiffuse: %.2f %.2f %.2f\n", matInfo[i].diffuse);
				FBXSDK_printf("\tEmissive: %.2f %.2f %.2f\n", matInfo[i].emissive);
				FBXSDK_printf("\tOpacity: %.2f\n", matInfo[i].opacity);
				FBXSDK_printf("\tTexture File Path: %s\n", matInfo[i].textureFilePath);
			}
			printf("\n");
		}
	}
	else
	{
		printf("\n\nThere's no Material Information\n\n");
	}

	//AnimationInformation
	if (animationInfo)
	{
		printf("Print Skeleton Animation? Y/N:\t");
		std::cin >> answer;
		if (answer == 'Y' || answer == 'y')
		{
			FBXSDK_printf("\n\tAnimation Name: %s\n", animationInfo->animationName);
			FBXSDK_printf("\tKey Frame Count: %d\n", animationInfo->keyFrameCount);
			FBXSDK_printf("\tJoint Count: %d\n", animationInfo->nrOfJoints);


			printf("Print Joint & Keyframes? Y/N:\t");
			std::cin >> answer;

			if (answer == 'Y' || answer == 'y')
			{
				for (int i = 0; i < animationInfo->joints.size(); i++)
				{
					FBXSDK_printf("\n\tJoint Name: %s\n", animationInfo->joints[i].jointName);
					FBXSDK_printf("\tParent Name: %s\n", animationInfo->joints[i].parentName);

					for (int j = 0; j < animationInfo->joints[i].keyFrames.size(); j++)
					{
						FBXSDK_printf("\t\tKey|%d| Time:     %.3f", j, animationInfo->joints[i].keyFrames[j].time);
						FBXSDK_printf("\tPosition: %.3f %.3f %.3f", j, animationInfo->joints[i].keyFrames[j].position[0], animationInfo->joints[i].keyFrames[j].position[1], animationInfo->joints[i].keyFrames[j].position[2]);
						FBXSDK_printf("\tRotation: %.3f %.3f %.3f", j, animationInfo->joints[i].keyFrames[j].rotation[0], animationInfo->joints[i].keyFrames[j].rotation[1], animationInfo->joints[i].keyFrames[j].rotation[2]);
						FBXSDK_printf("\tScaling:  %.3f %.3f %.3f\n", j,  animationInfo->joints[i].keyFrames[j].scaling[0] , animationInfo->joints[i].keyFrames[j].scaling[1] , animationInfo->joints[i].keyFrames[j].scaling[2]);
					}
				}
			}
			printf("\n");
		}
	}
	else
	{
		printf("\n\nThere's no Skeleton Animation\n\n");
	}

	//BlendShapes
	if (objectBlendShapes->blendShapeCount != 0)
	{
		printf("Print BlendShapes? Y/N:\t");
		std::cin >> answer;
		if (answer == 'Y' || answer == 'y')
		{
			FBXSDK_printf("\n\tBlend Shape Count: %d", objectBlendShapes->blendShapeCount);
			FBXSDK_printf("\tKey Frame Count:     %d", objectBlendShapes->keyFrameCount);

			for (int i = 0; i < objectBlendShapes->blendShape.size(); i++)
			{
				FBXSDK_printf("\tBlend Shape Vertex Count: %d", objectBlendShapes->blendShape[i].blendShapeVertexCount);

				for (int j = 0; j < objectBlendShapes->blendShape[i].blendShapeVertices.size(); j++)
				{
					FBXSDK_printf("\t\tBlend Shape|%d|\tVtx|%d|  X: %f\n", i, j, objectBlendShapes->blendShape[i].blendShapeVertices[j].x);
					FBXSDK_printf("\t\tBlend Shape|%d|\tVtx|%d|  Y: %f\n", i, j, objectBlendShapes->blendShape[i].blendShapeVertices[j].y);
					FBXSDK_printf("\t\tBlend Shape|%d|\tVtx|%d|  Z: %f\n", i, j, objectBlendShapes->blendShape[i].blendShapeVertices[j].z);
					FBXSDK_printf("\t\tBlend Shape|%d|\tVtx|%d| NX: %f\n", i, j, objectBlendShapes->blendShape[i].blendShapeVertices[j].nx);
					FBXSDK_printf("\t\tBlend Shape|%d|\tVtx|%d| NY: %f\n", i, j, objectBlendShapes->blendShape[i].blendShapeVertices[j].ny);
					FBXSDK_printf("\t\tBlend Shape|%d|\tVtx|%d| NZ: %f\n", i, j, objectBlendShapes->blendShape[i].blendShapeVertices[j].nz);
				}
			}
			printf("\n");
		}
	}
	else
	{
		printf("\n\nThere's no Blend Shape\n\n");
	}

	//Groups
	if (groups.size() != 0)
	{
		printf("Print Groups? Y/N:\t");
		std::cin >> answer;
		if (answer == 'Y' || answer == 'y')
		{
			for (int i = 0; i < groups.size(); i++)
			{
				FBXSDK_printf("\n\tGroup Name: %s\n", groups[i].groupName);
				FBXSDK_printf("\tChild Count:   %d\n", groups[i].childCount);

				for (int j = 0; j < groups[i].children.size(); j++)
				{
					FBXSDK_printf("\n\tChild|%d| Name: %s\n", i, groups[i].children[j].childName);
				}
			}

			printf("\n");
		}
	}
	else
	{
		printf("\n\nThere's no Groups\n\n");
	}


	//Custom Maya Attribute
	if (customMayaAttribute.size() != 0)
	{
		printf("Print Custom Maya Attribute? Y/N:\t");
		std::cin >> answer;
		if (answer == 'Y' || answer == 'y')
		{
			printf("\n");
			for (int i = 0; i < customMayaAttribute.size(); i++)
			{
				FBXSDK_printf("\tMesh Type: %d\n", customMayaAttribute[i].meshType);
			}
			printf("\n");
		}
	}
	else
	{
		printf("\n\nThere's no Custom Maya Attribute\n\n");
	}

	//Light
	if (exportLight.size() != 0)
	{
		printf("Print Lights? Y/N:\t");
		std::cin >> answer;
		if (answer == 'Y' || answer == 'y')
		{
			for (int i = 0; i < exportLight.size(); i++)
			{
				FBXSDK_printf("\n\tLight Type: ");
				switch (exportLight[i].type)
				{
				case '0':
					FBXSDK_printf("Point Light\n");
					break;
				case '1':
					FBXSDK_printf("Directional Light\n");
					break;
				case '2':
					FBXSDK_printf("Spotlight\n");
					break;
				default:
					FBXSDK_printf("Unknown\n");
					break;
				}

				FBXSDK_printf("\tColor:   R: %.2f G: %.2f B: %.2f\n", exportLight[i].color[0], exportLight[i].color[1], exportLight[i].color[2]);
				FBXSDK_printf("\tIntensity:  %.2f\n", exportLight[i].intensity);
				FBXSDK_printf("\tInner Cone:  %.2f\n", exportLight[i].innerCone);
				FBXSDK_printf("\tOuter Cone: %.2f\n", exportLight[i].outerCone);
			}
			printf("\n");
		}
	}
	else
	{
		printf("\n\n\tThere's no Light\n\n");
	}

	if (exportCamera.size() != 0)
	{
		printf("Print Camera? Y/N:\t");
		std::cin >> answer;
		getchar();
		if (answer == 'Y' || answer == 'y')
		{
			for (int i = 0; i < exportCamera.size(); i++)
			{
				FBXSDK_printf("\n\tPosition:     X: %.2f Y: %.2f Z: %.2f\n", exportCamera[i].position[0], exportCamera[i].position[1], exportCamera[i].position[2]);
				FBXSDK_printf("\tUp Vector:      X: %.2f Y: %.2f Z: %.2f\n", exportCamera[i].up[0], exportCamera[i].up[1], exportCamera[i].up[2]);
				FBXSDK_printf("\tForward Vector: X: %.2f Y: %.2f Z: %.2f\n", exportCamera[i].forward[0], exportCamera[i].forward[1], exportCamera[i].forward[2]);
				FBXSDK_printf("\n\tRoll:         %f\n", exportCamera[i].roll);
				FBXSDK_printf("\tAspect Width:   %f\n", exportCamera[i].aspectWidth);
				FBXSDK_printf("\tAspect Height:  %f\n", exportCamera[i].aspectHeight);
				FBXSDK_printf("\tField of View:  %f\n", exportCamera[i].fov);
				FBXSDK_printf("\tNear Plane:     %f\n", exportCamera[i].nearPlane);
				FBXSDK_printf("\tFar Plane:      %f\n", exportCamera[i].farPlane);
			}
			printf("\n");
		}
	}
	else
	{
		printf("\n\n\tThere's no Camera\n\n");
	}
}
//----------------------------------------------------------------------------------------------------------------------------------------------------------------
void Converter::loadCustomMayaAttributes(FbxNode * currentNode)
{
	unsigned int attributeValue;
	std::string attributeName = "";

	FbxProperty prop = currentNode->FindProperty(CUSTOM_ATTRIBUTE, false);
	if (prop.IsValid())
	{
		CustomMayaAttributes tempCustom;
		attributeName = prop.GetName();
		attributeValue = prop.Get<int>();
		
		//FBXSDK_printf("Custom Attribute: %s\n", attributeName.c_str());
		//FBXSDK_printf("Value Of Attribute: %d\n", attributeValue);
		tempCustom.meshType = prop.Get<int>();
		customMayaAttribute.push_back(tempCustom);
		counter.customMayaAttributeCount++;
	}
}
//----------------------------------------------------------------------------------------------------------------------------------------------------------------
void Converter::createCustomFile()
{
	size_t len = strlen(meshName);
	ret = new char[len + 2];
	strcpy(ret, meshName);
	ret[len - 3] = 'l';
	ret[len - 2] = 'e';
	ret[len - 1] = 'a';
	ret[len] = 'p';
	ret[len + 1] = '\0';
	meshName = ret;
	std::ofstream outfile(meshName, std::ofstream::binary);

	outfile.write((const char*)&counter, sizeof(Counter));

	for (int i = 0; i < meshInfo.size(); i++)
	{
		outfile.write((const char*)&meshInfo[i], sizeof(MeshInfo));
	}

	for (int i = 0; i < vertices.size(); i++)
	{
		outfile.write((const char*)&vertices[i], sizeof(VertexInformation));
	}

	for (int i = 0; i < matInfo.size(); i++)
	{
		outfile.write((const char*)&matInfo[i], sizeof(MaterialInformation));
	}

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

	//Morph animation/Blend shapes
	outfile.write((const char*)objectBlendShapes, 2 * sizeof(float));
	for (int i = 0; i < counter.blendShapeCount; i++)
	{
		outfile.write((const char*)&objectBlendShapes->blendShape[i].blendShapeVertexCount, sizeof(int));
		outfile.write((const char*)objectBlendShapes->blendShape[i].blendShapeVertices.data(), sizeof(BlendShapeVertex) * objectBlendShapes->blendShape[i].blendShapeVertexCount);
	}
	outfile.write((const char*)objectBlendShapes->keyframes.data(), sizeof(BlendShapeKeyframe)*objectBlendShapes->keyFrameCount);

	for (int i = 0; i < groups.size(); i++)
	{
		outfile.write((const char*)&groups[i], sizeof(Group));
	}

	for (int i = 0; i < customMayaAttribute.size(); i++)
	{
		outfile.write((const char*)&customMayaAttribute[i], sizeof(CustomMayaAttributes));
	}
	
	for (int i = 0; i < exportLight.size(); i++)
	{
		outfile.write((const char*)&exportLight[i], sizeof(Light));
	}
	
	for (int i = 0; i < exportCamera.size(); i++)
	{
		outfile.write((const char*)&exportCamera[i], sizeof(Camera));
	}

	outfile.close();

	if (textureCount != 0)
	{
		std::ifstream src(textureName, std::ios::binary);
		std::ofstream dst("NewColors.png", std::ios::binary);
		dst << src.rdbuf();
	}
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
		FbxString outputString = "Animation Stack Name: ";
		outputString += animStack->GetName();
		outputString += "\n";
		//FBXSDK_printf(outputString);
		
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
				outputString = "Current Animation Layer: ";
				outputString += j;
				outputString += "\n";
				//FBXSDK_printf(outputString);

				getAnimation(currentAnimLayer, node);
			}
		}
	}
	//printInformation();
}
//----------------------------------------------------------------------------------------------------------------------------------------------------------------
void Converter::getAnimation(FbxAnimLayer* animLayer, FbxNode* node)
{
	int modelCount;
	FbxString outputString;

	outputString = "   Node/Joint Name: ";
	outputString += node->GetName();
	outputString += "\n";
	//FBXSDK_printf(outputString);

	getAnimationChannels(node, animLayer);
	//FBXSDK_printf("\n");

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
		//jointInformation = new JointInformation[1];
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

		//keyFrame = new KeyFrame[keyCount];
		//KeyFrame keyFrame;
		for (int j = 0; j < keyCount; j++)
		{
			keyTime = animCurve->KeyGetTime(j).GetSecondDouble();
			//STORE: KeyFrame float time
			//keyFrame.time = keyTime;

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

			//keyFrame.keyFrameData = tempKeyFrameData;
			jointInformation.keyFrames.push_back(tempKeyFrameData);
		}
		animationInfo->joints.push_back(jointInformation);
		animationInfo->nrOfJoints += 1;
	}
}
//----------------------------------------------------------------------------------------------------------------------------------------------------------------
void Converter::printInformation()
{
	std::cout << "Animation Name: " << animationInfo->animationName << std::endl;
	std::cout << "Keyframe Count: " << animationInfo->keyFrameCount << std::endl;
	std::cout << "Nr of joints: " << animationInfo->nrOfJoints << std::endl << std::endl;
	for (int i = 0; i < animationInfo->nrOfJoints; i++)
	{
		std::cout << "Joint name: " << animationInfo->joints[i].jointName << std::endl;
		std::cout << "Parent name: " << animationInfo->joints[i].parentName << std::endl;
		for (int j = 0; j < animationInfo->joints[i].keyFrames.size(); j++)
		{
			std::cout << "Keyframe[" << j << "]" << std::endl;
			std::cout << "Time: " << animationInfo->joints[i].keyFrames[j].time << std::endl;
			std::cout << "TX: " << animationInfo->joints[i].keyFrames[j].position[0] << std::endl;
			std::cout << "TY: " << animationInfo->joints[i].keyFrames[j].position[1] << std::endl;
			std::cout << "TZ: " << animationInfo->joints[i].keyFrames[j].position[2] << std::endl;

			std::cout << "RX: " << animationInfo->joints[i].keyFrames[j].rotation[0] << std::endl;
			std::cout << "RY: " << animationInfo->joints[i].keyFrames[j].rotation[1] << std::endl;
			std::cout << "RZ: " << animationInfo->joints[i].keyFrames[j].rotation[2] << std::endl;

			std::cout << "SX: " << animationInfo->joints[i].keyFrames[j].scaling[0] << std::endl;
			std::cout << "SY: " << animationInfo->joints[i].keyFrames[j].scaling[1] << std::endl;
			std::cout << "SZ: " << animationInfo->joints[i].keyFrames[j].scaling[2] << std::endl << std::endl;
		}
	}
}
//----------------------------------------------------------------------------------------------------------------------------------------------------------------

/*void Converter::loadLevel(FbxNode * currentNode)
{
	printf("\n\t|| Node: %s\n", currentNode->GetName());

	mesh = currentNode->GetMesh();
	light = currentNode->GetLight();
	camera = currentNode->GetCamera();

	if (currentNode)
	{
		if (mesh && !isPartOf(currentNode->GetName()))
		{
			LevelObject lvlObj = LevelObject();
			FbxDouble3 tempTranslation = currentNode->LclTranslation.Get();
			FbxDouble3 tempRotation = currentNode->LclRotation.Get();
			// Save position
			lvlObj.x = (float)tempTranslation[0];
			lvlObj.y = (float)tempTranslation[1];
			lvlObj.z = (float)tempTranslation[2];
			// Save rotation
			lvlObj.rotationX = (float)tempTranslation[0];
			lvlObj.rotationY = (float)tempTranslation[1];
			lvlObj.rotationZ = (float)tempTranslation[2];

			//FBXSDK_printf("\t|| Translation: %f %f %f\n", tempTranslation[0], tempTranslation[1], tempTranslation[2]);
			//FBXSDK_printf("\t|| Rotation: %f %f %f\n", tempRotation[0], tempRotation[1], tempRotation[2]);

			// Save ID
			unsigned int attributeValue;
			//std::string attributeName = "";

			FbxProperty prop = currentNode->FindProperty(TYPE_ID, false);
			if (prop.IsValid())
			{
				//attributeName = prop.GetName();
				attributeValue = prop.Get<int>();

				//FBXSDK_printf("|| Mesh ID: %s\n", attributeName.c_str());
				//FBXSDK_printf("\t|| ID: %d\n", attributeValue);
				lvlObj.id = prop.Get<int>();
			}
			levelObjects.push_back(lvlObj);
			counter.levelObjectCount++;
		}
		//Load Cameras
		if (camera)
		{

		}

		//Load Lights
		if (light)
		{

		}
	}
	else
	{
		printf("Access violation: Node not found\n\n");
		exit(-2);
	}
}*/

/*void Converter::createCustomLevelFile()
{
	size_t len = strlen(meshName);
	ret = new char[len + 2];
	strcpy(ret, meshName);
	ret[len - 3] = 'l';
	ret[len - 2] = 'e';
	ret[len - 1] = 'a';
	ret[len] = 'p';
	ret[len + 1] = '\0';
	meshName = ret;

	std::ofstream outfile(meshName, std::ofstream::binary);

	outfile.write((const char*)&counter, sizeof(Counter));

	for (int i = 0; i < levelObjects.size(); i++)
	{
		outfile.write((const char*)&levelObjects[i], sizeof(LevelObject));
	}

	std::cout << "Number of level objects: " << counter.levelObjectCount << std::endl;

	outfile.close();
}*/

/*bool Converter::isPartOf(const char * nodeName)
{
	std::string nodeString;
	&nodeString.assign(nodeName);
	std::string findString = "_BBox";
	std::size_t found = nodeString.find(findString);
	
	if (found != std::string::npos)
	{
		return true;
	}

	return false;
}*/

/*void Converter::loadBbox(FbxNode* currentNode)
{
	BoundingBox bBox;
	FbxMesh* bBoxMesh = currentNode->GetMesh();
	FbxVector4* holder = bBoxMesh->GetControlPoints();
	FbxDouble3 tempTranslation = currentNode->LclTranslation.Get();
	//FbxDouble3 tempRotation = currentNode->LclRotation.Get();
	//FbxDouble3 tempScaling = currentNode->LclScaling.Get();
	//FBXSDK_printf("\t|| Translation: %f %f %f\n", tempTranslation[0], tempTranslation[1], tempTranslation[2]);
	int polyCount = bBoxMesh->GetPolygonCount();

	//Vertex* vert = new Vertex[counter.vertexCount];
	std::vector<VertexInformation> vert;
	std::vector<FbxVector4> position;

	bool ItIsFalse = false;

	bBoxMesh->GetPolygonVertices();
	//bBoxMesh->b

	int i = 0;
	for (int polygonIndex = 0; polygonIndex < polygonCount; polygonIndex++)
	{
		for (int vertexIndex = 0; vertexIndex < bBoxMesh->GetPolygonSize(polygonIndex); vertexIndex++)
		{

			VertexInformation temp;
			//Positions
			position.push_back(holder[bBoxMesh->GetPolygonVertex(polygonIndex, vertexIndex)]);
			//Add translation to vertex position
			temp.x = (float)position[i][0] + (float)tempTranslation[0];
			temp.y = (float)position[i][1] + (float)tempTranslation[1];
			temp.z = (float)position[i][2] + (float)tempTranslation[2];

			vert.push_back(temp);
			//FBXSDK_printf("\t|%d|Vertex: %f %f %f\n", i, (float)position[i][0], (float)position[i][1], (float)position[i][2]);
			//FBXSDK_printf("\t|%d|Vertex: %f %f %f\n", i, vert[i].x, vert[i].y, vert[i].z);

			i++;
		}
	}
	//FBXSDK_printf("\n");

	bBox.minVector[0] = vert[0].x;
	bBox.minVector[1] = vert[0].y;
	bBox.minVector[2] = vert[0].z;

	bBox.maxVector[0] = vert[0].x;
	bBox.maxVector[1] = vert[0].y;
	bBox.maxVector[2] = vert[0].z;

	for (int i = 1; i < vert.size(); i++)
	{
		//Min
		if (bBox.minVector[0] > vert[i].x)
			bBox.minVector[0] = vert[i].x;
		if (bBox.minVector[1] > vert[i].y)
			bBox.minVector[1] = vert[i].y;
		if (bBox.minVector[2] > vert[i].z)
			bBox.minVector[2] = vert[i].z;

		//Max
		if (bBox.maxVector[0] < vert[i].x)
			bBox.maxVector[0] = vert[i].x;
		if (bBox.maxVector[1] < vert[i].y)
			bBox.maxVector[1] = vert[i].y;
		if (bBox.maxVector[2] < vert[i].z)
			bBox.maxVector[2] = vert[i].z;
	};

	//Center
	bBox.center[0] = (bBox.minVector[0] + bBox.maxVector[0]) * 0.5f;
	bBox.center[1] = (bBox.minVector[1] + bBox.maxVector[1]) * 0.5f;
	bBox.center[2] = (bBox.minVector[2] + bBox.maxVector[2]) * 0.5f;

	//FBXSDK_printf("\t| |maxVector: %f %f %f\n", bBox.maxVector[0], bBox.maxVector[1], bBox.maxVector[2]);
	//FBXSDK_printf("\t| |minVector: %f %f %f\n", bBox.minVector[0], bBox.minVector[1], bBox.minVector[2]);
	//FBXSDK_printf("\t| |center: %f %f %f\n\n", bBox.center[0], bBox.center[1], bBox.center[2]);

	vBBox.push_back(bBox);

	vert.clear();
	position.clear();

	counter.boundingBoxCount++;
}*/
