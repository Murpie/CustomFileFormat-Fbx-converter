#include "Converter.h"
#include <fstream>
#include <vector>

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
	this->meshName = fileName;
}
//----------------------------------------------------------------------------------------------------------------------------------------------------------------
Converter::~Converter()
{
	delete vertices;
	delete []objectBlendShapes;
	delete matInfo;
	delete ret;

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

	//Create the Custom File
	createCustomFile();
}
//----------------------------------------------------------------------------------------------------------------------------------------------------------------
void Converter::exportFile(FbxNode* currentNode)
{
	printf("\nNode: %s\n", currentNode->GetName());

	loadGlobaltransform(currentNode);
	
	mesh = currentNode->GetMesh();
	light = currentNode->GetLight();
	camera = currentNode->GetCamera();
	
	if (currentNode)
	{
		//Load in Vertex data
		if (mesh)
		{
			loadVertex(mesh);
		}

		//Load Material & Texture File information
		if (mesh)
		{
			loadMaterial(currentNode);
		}

		if (mesh)
		{
			loadBlendShape(mesh, ourScene);
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

		FBXSDK_printf("Translation: %f %f %f\n", meshInfo->globalTranslation[0], meshInfo->globalTranslation[1], meshInfo->globalTranslation[2]);
		FBXSDK_printf("Rotation: %f %f %f\n", meshInfo->globalRotation[0], meshInfo->globalRotation[1], meshInfo->globalRotation[2]);
		FBXSDK_printf("Scaling: %f %f %f\n\n", meshInfo->globalScaling[0], meshInfo->globalScaling[1], meshInfo->globalScaling[2]);
	}
	delete meshInfo;
}
//----------------------------------------------------------------------------------------------------------------------------------------------------------------
void Converter::loadVertex(FbxMesh* currentMesh)
{
	polygonCount = currentMesh->GetPolygonCount();

	//Vertices
	controlPoints = currentMesh->GetControlPoints();

	counter.vertexCount = polygonCount * 3;

	vertices = new VertexInformation[counter.vertexCount];

	std::vector<FbxVector4> pos;
	std::vector<FbxVector4> norm;
	std::vector<FbxVector2> uv;
	FbxVector4 temp;
	FbxVector2 tempUv;

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

			/*FBXSDK_printf("\t|%d|Vertex: %f %f %f\n", i, vertices[i].x, vertices[i].y, vertices[i].z);
			FBXSDK_printf("\t|%d|Normals: %f %f %f\n", i, vertices[i].nx, vertices[i].ny, vertices[i].nz);
			FBXSDK_printf("\t|%d|UVs: %f %f\n\n", i, vertices[i].u, vertices[i].v);*/

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
			std::cout << "\nMaterial name: " << lMaterial->GetName() << std::endl << std::endl;

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

			lString = nullptr;
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
			FBXSDK_printf(lString);

			//File Texture path from Material
			FbxProperty fileTextureProp = lMaterial->FindProperty(FbxSurfaceMaterial::sDiffuse);

			if (fileTextureProp != NULL)
			{
				textureCount = fileTextureProp.GetSrcObjectCount<FbxFileTexture>();
				for (int i = 0; i < textureCount; i++)
				{
					FbxFileTexture* texture = FbxCast<FbxFileTexture>(fileTextureProp.GetSrcObject<FbxFileTexture>(i));

					textureName = texture->GetFileName();

					FBXSDK_printf("Texture file found!\nPath: %s\n", textureName);
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
	FBXSDK_printf(lOutputString);

	for (int animLayerIndex = 0; animLayerIndex < animLayers; animLayerIndex++)
	{
		FbxAnimLayer* animLayer = animStack->GetMember<FbxAnimLayer>(animLayerIndex);

		counter.blendShapeCount = blendShapeDeformerCount;
		cout << "	Number of blendshapes Animations: " << counter.blendShapeCount << endl;

		//Create Blendshapes
		

		for (int blendShapeIndex = 0; blendShapeIndex < blendShapeDeformerCount; ++blendShapeIndex)
		{
			FbxBlendShape* blendShape = (FbxBlendShape*)mesh->GetDeformer(blendShapeIndex, FbxDeformer::eBlendShape);
			int blendShapeChannelCount = blendShape->GetBlendShapeChannelCount();

			//Setting blendshapeCount for animation
			objectBlendShapes[0].blendShapeCount = blendShapeChannelCount;

			cout << "	Number of blendshapes for this Animation: " << objectBlendShapes[0].blendShapeCount << endl;

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

					FBXSDK_printf("	BlendShape Vertex: %d\n", blendshapeControlIndex);
					FBXSDK_printf("		x = %f", tempBlendShape.blendShapeVertices[blendshapeControlIndex].x);
					FBXSDK_printf(" y = %f", tempBlendShape.blendShapeVertices[blendshapeControlIndex].y);
					FBXSDK_printf(" z = %f\n", tempBlendShape.blendShapeVertices[blendshapeControlIndex].z);

					FBXSDK_printf("		nx = %f", tempBlendShape.blendShapeVertices[blendshapeControlIndex].nx);
					FBXSDK_printf(" ny = %f", tempBlendShape.blendShapeVertices[blendshapeControlIndex].ny);
					FBXSDK_printf(" nz = %f\n", tempBlendShape.blendShapeVertices[blendshapeControlIndex].nz);
				}

				objectBlendShapes[0].blendShape.push_back(tempBlendShape);

				FbxAnimCurve* animCurve = mesh->GetShapeChannel(blendShapeIndex, channelIndex, animLayer, true);
				if (animCurve)
				{
					FBXSDK_printf("	Shape Name: %s\n", channelName);
					
					float keyValue;
					char timeString[256];
					int count;

					int keyCount = animCurve->KeyGetCount();

					objectBlendShapes[0].keyFrameCount = keyCount;


					cout << "	KeyCount: " << objectBlendShapes[0].keyFrameCount << endl;

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
						FBXSDK_printf(lOutputString);
					}
				}
			}
		}
	}
	

	////Vertices
	//controlPoints = currentMesh->GetControlPoints();

	//counter.vertexCount = polygonCount * 3;
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
		position = currentNode->Position.Get();
		upVector = currentNode->UpVector.Get();
		forwardVector = currentNode->InterestPosition.Get();
		roll = currentNode->Roll.Get();
		aspectWidth = currentNode->AspectWidth.Get();
		aspectHeight = currentNode->AspectHeight.Get();
		fov = currentNode->FieldOfView.Get();
		nearPlane = currentNode->NearPlane.Get();
		farPlane = currentNode->FarPlane.Get();

		FBXSDK_printf("\tPosition: %.2f %.2f %.2f\n", position[0], position[1], position[2]);
		FBXSDK_printf("\tUp: %.2f %.2f %.2f\n", upVector[0], upVector[1], upVector[2]);
		FBXSDK_printf("\tLook At: %.2f %.2f %.2f\n", forwardVector[0], forwardVector[1], forwardVector[2]);
		FBXSDK_printf("\tRoll: %.2f\n", roll);
		FBXSDK_printf("\tAspect Ratio: %.fx%.f\n", aspectWidth, aspectHeight);
		FBXSDK_printf("\tField of View: %.f\n", fov);
		FBXSDK_printf("\tNear Plane: %.2f\n\tFar Plane: %.2f\n\n", nearPlane, farPlane);
	}
}
//----------------------------------------------------------------------------------------------------------------------------------------------------------------
void Converter::loadLights(FbxLight* currentLight)
{
	FbxString lightType = currentLight->LightType.Get();
	FbxDouble3 lightColor = currentLight->Color.Get();
	FbxFloat intensity = currentLight->Intensity.Get();
	FbxFloat innerCone = currentLight->InnerAngle.Get();
	FbxFloat outerCone = currentLight->OuterAngle.Get();

	if (lightType == "0")
	{
		FBXSDK_printf("\tLight Type: Point Light\n");
	}
	else if(lightType == "1")
	{
		FBXSDK_printf("\tLight Type: Directional Light\n");
	}
	else if(lightType == "2")
	{
		FBXSDK_printf("\tLight Type: Spotlight\n");
	}

	FBXSDK_printf("\tColor: %.3f %.3f %.3f\n", lightColor[0], lightColor[1], lightColor[2]);
	FBXSDK_printf("\tIntensity: %.2f\n", intensity);

	if (lightType == "2")
	{
		FBXSDK_printf("\tInner Cone: %.2f\n", innerCone);
		FBXSDK_printf("\tOuter Cone: %.2f\n", outerCone);
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
	outfile.write((const char*)vertices, sizeof(VertexInformation)*counter.vertexCount);
	outfile.write((const char*)meshInfo, sizeof(MeshInfo));
	outfile.write((const char*)objectBlendShapes, sizeof(BlendShapes)*counter.blendShapeCount);
	//outfile.write((const char*)matInfo, sizeof(MaterialInformation));

	outfile.close();

	if (textureCount != 0)
	{
		std::ifstream src(textureName, std::ios::binary);
		std::ofstream dst("NewColors.png", std::ios::binary);
		dst << src.rdbuf();
	}
}