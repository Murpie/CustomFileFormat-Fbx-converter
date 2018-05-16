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

	//
	counter.boundingBoxCount = 0;
	counter.levelObjectCount = 0;
}
//----------------------------------------------------------------------------------------------------------------------------------------------------------------
Converter::~Converter()
{
	delete vertices;
	delete []objectBlendShapes;
	delete matInfo;
	delete ret;
	delete customMayaAttribute;
	delete exportCamera;
	delete exportLight;

	delete[] animationInfo;

	//delete[] keyFrameData;

	ourScene->Destroy();
	settings->Destroy();
	manager->Destroy();

	vBBox.clear();
	levelObjects.clear();
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
			/*if (isLevel)
			{
				loadLevel(rootNode->GetChild(i));
			}
			else if (isPartOf(rootNode->GetChild(i)->GetName()))
			{
				loadBbox(rootNode->GetChild(i));
			}
			else*/

			exportFile(rootNode->GetChild(i));
		}
	}
	
	exportAnimation(ourScene, rootNode);
	createCustomFile();

	//Create the Custom File
	/*if (isLevel)
		createCustomLevelFile();*/
	//else

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
	group = currentNode;

	if (currentNode)
	{
		//Load in Vertex data
		if (mesh)
		{
			loadVertex(mesh, currentNode);
		}

		//Load Material & Texture File information
		if (mesh)
		{
			loadMaterial(currentNode);
		}

		//Load Maya Custom Attributes
		if (mesh)
		{
			loadBlendShape(mesh, ourScene);
			loadCustomMayaAttributes(currentNode);
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
		//Groups?
		if (group)
		{
			loadGroups(group);
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
	meshInfo = new MeshInfo[1];						//There will always just be a single mesh, for now

	if (meshInfo)
	{
		FbxDouble3 tempTranslation = currentNode->LclTranslation.Get();
		FbxDouble3 tempRotation = currentNode->LclRotation.Get();
		FbxDouble3 tempScaling = currentNode->LclScaling.Get();

		for (int i = 0; i < COLOR_RANGE; i++)
		{
			meshInfo->globalTranslation[i] = tempTranslation[i];
			meshInfo->globalRotation[i] = tempRotation[i];
			meshInfo->globalScaling[i] = tempScaling[i];
		}

		//FBXSDK_printf("Translation: %f %f %f\n", meshInfo->globalTranslation[0], meshInfo->globalTranslation[1], meshInfo->globalTranslation[2]);
		//FBXSDK_printf("Rotation: %f %f %f\n", meshInfo->globalRotation[0], meshInfo->globalRotation[1], meshInfo->globalRotation[2]);
		//FBXSDK_printf("Scaling: %f %f %f\n\n", meshInfo->globalScaling[0], meshInfo->globalScaling[1], meshInfo->globalScaling[2]);
	}
	delete meshInfo;
}
//----------------------------------------------------------------------------------------------------------------------------------------------------------------
void Converter::loadVertex(FbxMesh* currentMesh, FbxNode* currentNode)
{
	polygonCount = currentMesh->GetPolygonCount();

	//Vertices
	controlPoints = currentMesh->GetControlPoints();

	counter.vertexCount = polygonCount * 3;

	vertices = new VertexInformation[counter.vertexCount];

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
			//Positions
			pos.push_back(controlPoints[currentMesh->GetPolygonVertex(polygonIndex, vertexIndex)]);

			//Normals
			currentMesh->GetPolygonVertexNormal(polygonIndex, vertexIndex, temp);
			norm.push_back(temp);

			vertexBinormal = currentMesh->GetElementBinormal();
			if (vertexBinormal)
			{
				foundBinormal = true;
				for (int lVertexIndex = 0; lVertexIndex < currentMesh->GetControlPointsCount(); lVertexIndex++)
				{
					int lNormalIndex = 0;
					//reference mode is direct, the normal index is same as vertex index.
					//get normals by the index of control vertex
					if (vertexBinormal->GetReferenceMode() == FbxGeometryElement::eDirect)
						lNormalIndex = lVertexIndex;
					//reference mode is index-to-direct, get normals by the index-to-direct
					if (vertexBinormal->GetReferenceMode() == FbxGeometryElement::eIndexToDirect)
						lNormalIndex = vertexBinormal->GetIndexArray().GetAt(lVertexIndex);
					//Got normals of each vertex.
					FbxVector4 lNormal = vertexBinormal->GetDirectArray().GetAt(lNormalIndex);
					//FBXSDK_printf("Binormals for vertex[%d]: %f %f %f %f \n", lVertexIndex, lNormal[0], lNormal[1], lNormal[2], lNormal[3]);
					//add your custom code here, to output normals or get them into a list, such as KArrayTemplate<FbxVector4>
					//. . .
					biNorm.push_back(lNormal);
				}

				vertices[i].bnx = (float)biNorm[i][0];
				vertices[i].bny = (float)biNorm[i][1];
				vertices[i].bnz = (float)biNorm[i][2];
			}

			vertexTangent = currentMesh->GetElementTangent();
			if (vertexTangent)
			{
				foundTangent = true;
				for (int lVertexIndex = 0; lVertexIndex < currentMesh->GetControlPointsCount(); lVertexIndex++)
				{
					int lNormalIndex = 0;
					//reference mode is direct, the normal index is same as vertex index.
					//get normals by the index of control vertex
					if (vertexTangent->GetReferenceMode() == FbxGeometryElement::eDirect)
						lNormalIndex = lVertexIndex;
					//reference mode is index-to-direct, get normals by the index-to-direct
					if (vertexTangent->GetReferenceMode() == FbxGeometryElement::eIndexToDirect)
						lNormalIndex = vertexTangent->GetIndexArray().GetAt(lVertexIndex);
					//Got normals of each vertex.
					FbxVector4 lNormal = vertexTangent->GetDirectArray().GetAt(lNormalIndex);
					//FBXSDK_printf("Tangents for vertex[%d]: %f %f %f %f \n", lVertexIndex, lNormal[0], lNormal[1], lNormal[2], lNormal[3]);
					//add your custom code here, to output normals or get them into a list, such as KArrayTemplate<FbxVector4>
					//. . .
					tangent.push_back(lNormal);
				}

				vertices[i].tx = (float)tangent[i][0];
				vertices[i].ty = (float)tangent[i][1];
				vertices[i].tz = (float)tangent[i][2];
			}

			//UVs
			FbxStringList uvSetNamesList;
			currentMesh->GetUVSetNames(uvSetNamesList);
			const char* uvNames = uvSetNamesList.GetStringAt(0);
			currentMesh->GetPolygonVertexUV(polygonIndex, vertexIndex, uvNames, tempUv, ItIsFalse);
			uv.push_back(tempUv);

			vertices[i].x = (float)pos[i][0];
			vertices[i].y = (float)pos[i][1];
			vertices[i].z = (float)pos[i][2];

			vertices[i].nx = (float)norm[i][0];
			vertices[i].ny = (float)norm[i][1];
			vertices[i].nz = (float)norm[i][2];

			vertices[i].u = (float)uv[i][0];
			vertices[i].v = (float)uv[i][1];

			//Weights
			loadWeights(currentNode, i);

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

			//lString = nullptr;
			//lString = "            Ambient: ";
			//lString += (float)ambient.mRed;
			//lString += " (red), ";
			//lString += (float)ambient.mGreen;
			//lString += " (green), ";
			//lString += (float)ambient.mBlue;
			//lString += " (blue), ";
			//lString += "";
			//lString += "\n\n";
			//FBXSDK_printf(lString);

			//lString = nullptr;
			//lString = "            Diffuse: ";
			//lString += (float)diffuse.mRed;
			//lString += " (red), ";
			//lString += (float)diffuse.mGreen;
			//lString += " (green), ";
			//lString += (float)diffuse.mBlue;
			//lString += " (blue), ";
			//lString += "";
			//lString += "\n\n";
			//FBXSDK_printf(lString);


			//lString = nullptr;
			//lString = "            Emissive: ";
			//lString += (float)emissive.mRed;
			//lString += " (red), ";
			//lString += (float)emissive.mGreen;
			//lString += " (green), ";
			//lString += (float)emissive.mBlue;
			//lString += " (blue), ";
			//lString += "";
			//lString += "\n\n";
			//FBXSDK_printf(lString);

			//lString = nullptr;
			//FbxString lFloatValue = (float)transparency;
			//lFloatValue = transparency <= -HUGE_VAL ? "-INFINITY" : lFloatValue.Buffer();
			//lFloatValue = transparency >= HUGE_VAL ? "INFINITY" : lFloatValue.Buffer();
			//lString = "            Opacity: ";
			//lString += lFloatValue;
			//lString += "";
			//lString += "\n\n";
			//FBXSDK_printf(lString);

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
	matInfo = new MaterialInformation[materialCount];

	//Colormaps
	for (int k = 0; k < COLOR_RANGE; k++)
	{
		switch (k)
		{
		case 0:
			matInfo->ambient[k] = (float)ambient.mRed;
			matInfo->diffuse[k] = (float)diffuse.mRed;
			matInfo->emissive[k] = (float)emissive.mRed;
			break;
		case 1:
			matInfo->ambient[k] = (float)ambient.mGreen;
			matInfo->diffuse[k] = (float)diffuse.mGreen;
			matInfo->emissive[k] = (float)emissive.mGreen;
			break;
		case 2:
			matInfo->ambient[k] = (float)ambient.mBlue;
			matInfo->diffuse[k] = (float)diffuse.mBlue;
			matInfo->emissive[k] = (float)emissive.mBlue;
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
			matInfo->textureFilePath[i] = textureName[i];
		}
	}

	//Opacity
	matInfo->opacity = (float)transparency;
}
void Converter::loadBlendShape(FbxMesh* currentMesh, FbxScene* scene)
{
	FbxAnimStack* animStack = scene->GetSrcObject<FbxAnimStack>();

	int animLayers = animStack->GetMemberCount<FbxAnimLayer>();

	FbxString lOutputString;

	int blendShapeDeformerCount = mesh->GetDeformerCount(FbxDeformer::eBlendShape);

	objectBlendShapes = new BlendShapes[blendShapeDeformerCount];

	lOutputString = "	Contains: ";
	if (animLayers == 0)
		lOutputString += "no layers";

	if (animLayers)
	{
		lOutputString += animLayers;
		lOutputString += " Animation Layer\n";
	}
	//FBXSDK_printf(lOutputString);

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

	FbxVector4 position;
	FbxVector4 upVector;
	FbxVector4 forwardVector;
	float roll;
	
	float aspectWidth, aspectHeight;
	float fov;
	float nearPlane, farPlane;

	if (camera)
	{
		exportCamera = new Camera[1];

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
			exportCamera->position[i] = position[i];
			exportCamera->up[i] = upVector[i];
			exportCamera->forward[i] = forwardVector[i];
		}

		exportCamera->roll = roll;
		exportCamera->aspectWidth = aspectWidth;
		exportCamera->aspectHeight = aspectHeight;
		exportCamera->fov = fov;
		exportCamera->nearPlane = nearPlane;
		exportCamera->farPlane = farPlane;

	/*	FBXSDK_printf("\tPosition: %.2f %.2f %.2f\n", position[0], position[1], position[2]);
		FBXSDK_printf("\tUp: %.2f %.2f %.2f\n", upVector[0], upVector[1], upVector[2]);
		FBXSDK_printf("\tLook At: %.2f %.2f %.2f\n", forwardVector[0], forwardVector[1], forwardVector[2]);
		FBXSDK_printf("\tRoll: %.2f\n", roll);
		FBXSDK_printf("\tAspect Ratio: %.fx%.f\n", aspectWidth, aspectHeight);
		FBXSDK_printf("\tField of View: %.f\n", fov);
		FBXSDK_printf("\tNear Plane: %.2f\n\tFar Plane: %.2f\n\n", nearPlane, farPlane);*/
	}
}
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------
void Converter::loadGroups(FbxNode* currentNode)
{
	if ((std::string)currentNode->GetName() != "RootNode")
	{
		groups = new Group[1];

		const char* tempGroupName = currentNode->GetName();
		for (int i = 0; i < strlen(tempGroupName) + 1; i++)
		{
			groups->groupName[i] = tempGroupName[i];
		}

		//std::cout << "GroupName:" << groups->groupName << std::endl << "Nr of children: " << currentNode->GetChildCount() << endl;

		int childrenSize = 0;
		for (int i = 0; i < currentNode->GetChildCount(); i++)
		{
			const char* tempChildrenName = currentNode->GetChild(i)->GetName();
			for (int j = 0; j < strlen(tempChildrenName) + 1; j++)
			{
				groups->childName[i][j] = tempChildrenName[j];
			}
			childrenSize++;
		}
		groups->childCount = childrenSize;

		/*for (int i = 0; i < groups->childCount; i++)
			std::cout << groups->childName[i] << std::endl;*/

		delete groups;
	}
}
//----------------------------------------------------------------------------------------------------------------------------------------------------------------
void Converter::loadLights(FbxLight* currentLight)
{
	exportLight = new Light[1];

	FbxString lightType = currentLight->LightType.Get();
	FbxDouble3 lightColor = currentLight->Color.Get();
	FbxFloat intensity = currentLight->Intensity.Get();
	FbxFloat innerCone = currentLight->InnerAngle.Get();
	FbxFloat outerCone = currentLight->OuterAngle.Get();

	for (int i = 0; i < strlen(lightType); i++)
	{
		exportLight->type[i] = lightType[i];
	}

	for (int i = 0; i < COLOR_RANGE; i++)
	{
		exportLight->color[i] = lightColor[i];
	}

	exportLight->intensity = intensity;
	exportLight->innerCone = innerCone;
	exportLight->outerCone = outerCone;

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
}
//----------------------------------------------------------------------------------------------------------------------------------------------------------------
void Converter::loadWeights(FbxNode* currentNode, int vertexIndex)
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
				vertices[vertexIndex].weight[j] = store[j].weight;
				vertices[vertexIndex].weightID[j] = store[j].ID;
			}
		}

		if (nrOfWeights < 4)
		{
			if (nrOfWeights == 3)
			{
				vertices[vertexIndex].weight[3] = -1;
				vertices[vertexIndex].weightID[3] = -1;
			}
			else if (nrOfWeights == 2)
			{
				vertices[vertexIndex].weight[2] = -1;
				vertices[vertexIndex].weightID[2] = -1;
				vertices[vertexIndex].weight[3] = -1;
				vertices[vertexIndex].weightID[3] = -1;
			}
			else if (nrOfWeights == 1)
			{
				vertices[vertexIndex].weight[1] = -1;
				vertices[vertexIndex].weightID[1] = -1;
				vertices[vertexIndex].weight[2] = -1;
				vertices[vertexIndex].weightID[2] = -1;
				vertices[vertexIndex].weight[3] = -1;
				vertices[vertexIndex].weightID[3] = -1;
			}
			else
			{
				vertices[vertexIndex].weight[0] = -1;
				vertices[vertexIndex].weightID[0] = -1;
				vertices[vertexIndex].weight[1] = -1;
				vertices[vertexIndex].weightID[1] = -1;
				vertices[vertexIndex].weight[2] = -1;
				vertices[vertexIndex].weightID[2] = -1;
				vertices[vertexIndex].weight[3] = -1;
				vertices[vertexIndex].weightID[3] = -1;
			}
		}

		store.clear();
	}
}
void Converter::printInfo()
{
	//Prints
	for (int i = 0; i < counter.vertexCount; i++)
	{
		FBXSDK_printf("\t|%d|Vertex: %f %f %f\n", i, vertices[i].x, vertices[i].y, vertices[i].z);
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
//----------------------------------------------------------------------------------------------------------------------------------------------------------------
void Converter::loadCustomMayaAttributes(FbxNode * currentNode)
{
	customMayaAttribute = new CustomMayaAttributes[1];

	unsigned int attributeValue;
	std::string attributeName = "";

	FbxProperty prop = currentNode->FindProperty(CUSTOM_ATTRIBUTE, false);
	if (prop.IsValid())
	{
		attributeName = prop.GetName();
		attributeValue = prop.Get<int>();
		
		//FBXSDK_printf("Custom Attribute: %s\n", attributeName.c_str());
		//FBXSDK_printf("Value Of Attribute: %d\n", attributeValue);
		customMayaAttribute->meshType = prop.Get<int>();
	}
	this->counter.customMayaAttributeCount++;
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
	outfile.write((const char*)vertices, sizeof(VertexInformation)*counter.vertexCount);
	//outfile.write((const char*)meshInfo, sizeof(MeshInfo));
	outfile.write((const char*)matInfo, sizeof(MaterialInformation));

	// write the fixed part (blendShapeCount, keyFramecount)
	outfile.write((const char*)objectBlendShapes, 2 * sizeof(float));
	for (int i = 0; i < counter.blendShapeCount; i++)
	{
		outfile.write((const char*)&objectBlendShapes->blendShape[i].blendShapeVertexCount, sizeof(int));
		outfile.write((const char*)objectBlendShapes->blendShape[i].blendShapeVertices.data(), sizeof(BlendShapeVertex)*objectBlendShapes->blendShape[i].blendShapeVertexCount);
	}
	outfile.write((const char*)objectBlendShapes->keyframes.data(), sizeof(BlendShapeKeyframe)*objectBlendShapes->keyFrameCount);

	outfile.write((const char*)animationInfo->animationName, sizeof(char) * 9);
	outfile.write((const char*)&animationInfo->keyFrameCount, sizeof(int));
	outfile.write((const char*)&animationInfo->nrOfJoints, sizeof(int));
	for (int i = 0; i < animationInfo->nrOfJoints; i++)
	{
		size_t jLen = strlen(animationInfo->joints[i].jointName);
		size_t pLen = strlen(animationInfo->joints[i].parentName);
		outfile.write((const char*)&animationInfo->joints[i].jointName, sizeof(char) * 100);
		outfile.write((const char*)&animationInfo->joints[i].parentName, sizeof(char) * 100);
		//outfile.write((const char*)&animationInfo->joints[i].localTransformMatrix, sizeof(float) * 16);
		//outfile.write((const char*)&animationInfo->joints[i].bindPoseMatrix, sizeof(float) * 16);

		outfile.write((const char*)animationInfo->joints[i].keyFrames.data(), sizeof(KeyFrame)*animationInfo->keyFrameCount);
	}

	// for each blendshape in "blendshape"
	//   write blendshapevertexcount
	//   write pointer to blendshapevertex (source address), count, size is sizeof(blendshapevertex)
	// for each keyframe in keyframes
	//   write all keyframes (pointer to the first one, and the count, and the size of each)


	//outfile.write((const char*)objectBlendShapes, (sizeof(BlendShapes)*counter.blendShapeCount) + (sizeof(objectBlendShapes->blendShape)*objectBlendShapes->blendShapeCount) + (sizeof(objectBlendShapes->blendShape[0].blendShapeVertices)*objectBlendShapes->blendShape[0].blendShapeVertexCount) + (sizeof(objectBlendShapes->keyframes)*objectBlendShapes->keyFrameCount));
	//outfile.write((const char*)meshInfo, sizeof(MeshInfo));
	/*for (int i = 0; i < vBBox.size(); i++)
	{
		outfile.write((const char*)&vBBox[i], sizeof(BoundingBox));
	}*/
	//outfile.write((const char*)groups, sizeof(Group));

	//outfile.write((const char*)matInfo, sizeof(MaterialInformation));
	//outfile.write((const char*)customMayaAttribute, sizeof(CustomMayaAttributes));

	//size_t aLen = strlen(animationInfo->animationName);
	//animationInfo->animationName[len + 1] += '\0';

	outfile.close();

	if (textureCount != 0)
	{
		std::ifstream src(textureName, std::ios::binary);
		std::ofstream dst("NewColors.png", std::ios::binary);
		dst << src.rdbuf();
	}
}

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

		outputString = "   contains ";
		if (animLayers == 0)
			outputString += "no layers";
		if (animLayers)
		{
			outputString += animLayers;
			outputString += " Animation layer(s)\n\n";
		}
		//FBXSDK_printf(outputString);

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
	//printInformation();
}

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

void Converter::loadLevel(FbxNode * currentNode)
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
}

void Converter::createCustomLevelFile()
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
}

bool Converter::isPartOf(const char * nodeName)
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
}

void Converter::loadBbox(FbxNode* currentNode)
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
}
