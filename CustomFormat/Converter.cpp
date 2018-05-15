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
	delete matInfo;
	delete ret;

	delete[] animationInfo;

	//delete[] keyFrameData;

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
	
	exportAnimation(ourScene, rootNode);

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

			//FBXSDK_printf("\t|%d|Vertex: %f %f %f\n", i, vertices[i].x, vertices[i].y, vertices[i].z);
			//FBXSDK_printf("\t|%d|Normals: %f %f %f\n", i, vertices[i].nx, vertices[i].ny, vertices[i].nz);
			//FBXSDK_printf("\t|%d|UVs: %f %f\n\n", i, vertices[i].u, vertices[i].v);

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

		/*FBXSDK_printf("\tPosition: %.2f %.2f %.2f\n", position[0], position[1], position[2]);
		FBXSDK_printf("\tUp: %.2f %.2f %.2f\n", upVector[0], upVector[1], upVector[2]);
		FBXSDK_printf("\tLook At: %.2f %.2f %.2f\n", forwardVector[0], forwardVector[1], forwardVector[2]);
		FBXSDK_printf("\tRoll: %.2f\n", roll);
		FBXSDK_printf("\tAspect Ratio: %.fx%.f\n", aspectWidth, aspectHeight);
		FBXSDK_printf("\tField of View: %.f\n", fov);
		FBXSDK_printf("\tNear Plane: %.2f\n\tFar Plane: %.2f\n\n", nearPlane, farPlane);*/
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
		//FBXSDK_printf("\tLight Type: Point Light\n");
	}
	else if(lightType == "1")
	{
		//FBXSDK_printf("\tLight Type: Directional Light\n");
	}
	else if(lightType == "2")
	{
		//FBXSDK_printf("\tLight Type: Spotlight\n");
	}

	//FBXSDK_printf("\tColor: %.3f %.3f %.3f\n", lightColor[0], lightColor[1], lightColor[2]);
	//FBXSDK_printf("\tIntensity: %.2f\n", intensity);

	if (lightType == "2")
	{
		//FBXSDK_printf("\tInner Cone: %.2f\n", innerCone);
		//FBXSDK_printf("\tOuter Cone: %.2f\n", outerCone);
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
	//outfile.write((const char*)meshInfo, sizeof(MeshInfo));
	//outfile.write((const char*)matInfo, sizeof(MaterialInformation));

	//size_t aLen = strlen(animationInfo->animationName);
	//animationInfo->animationName[len + 1] += '\0';
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


