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
	delete matInfo;
	delete ret;
	delete customMayaAttribute;

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
			if (isLevel)
			{
				loadLevel(rootNode->GetChild(i));
			}
			else if (isPartOf(rootNode->GetChild(i)->GetName()))
			{
				loadBbox(rootNode->GetChild(i));
			}
			else
				exportFile(rootNode->GetChild(i));
		}
	}

	//Create the Custom File
	if (isLevel)
		createCustomLevelFile();
	else
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

		//Load Maya Custom Attributes
		if (mesh)
		{
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
void Converter::loadVertex(FbxMesh* currentMesh)
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

			FBXSDK_printf("\t|%d|Vertex: %f %f %f\n", i, vertices[i].x, vertices[i].y, vertices[i].z);
			FBXSDK_printf("\t|%d|Normals: %f %f %f\n", i, vertices[i].nx, vertices[i].ny, vertices[i].nz);
			
			if (vertexBinormal) 
				FBXSDK_printf("\t|%d|Binormals: %f %f %f\n", i, vertices[i].bnx, vertices[i].bny, vertices[i].bnz);
			if (vertexTangent)
				FBXSDK_printf("\t|%d|Tangents: %f %f %f\n", i, vertices[i].tx, vertices[i].ty, vertices[i].tz);

			FBXSDK_printf("\t|%d|UVs: %f %f\n\n", i, vertices[i].u, vertices[i].v);

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
		
		FBXSDK_printf("Custom Attribute: %s\n", attributeName.c_str());
		FBXSDK_printf("Value Of Attribute: %d\n", attributeValue);
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
	for (int i = 0; i < vBBox.size(); i++)
	{
		outfile.write((const char*)&vBBox[i], sizeof(BoundingBox));
	}
	//outfile.write((const char*)matInfo, sizeof(MaterialInformation));
	outfile.write((const char*)customMayaAttribute, sizeof(CustomMayaAttributes));

	std::cout << customMayaAttribute->meshType << std::endl;

	outfile.close();

	if (textureCount != 0)
	{
		std::ifstream src(textureName, std::ios::binary);
		std::ofstream dst("NewColors.png", std::ios::binary);
		dst << src.rdbuf();
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

			FBXSDK_printf("\t|| Translation: %f %f %f\n", tempTranslation[0], tempTranslation[1], tempTranslation[2]);
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
				FBXSDK_printf("\t|| ID: %d\n", attributeValue);
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
