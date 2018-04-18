#include "Converter.h"
#include "MeshStructs.h"
#include <fbxsdk\core\base\fbxtime.h>
#include <fstream>
#include <vector>

#pragma warning(disable : 4996)

void getAnimation(FbxAnimLayer* animLayer, FbxNode* node);
void getAnimationChannels(FbxNode* node, FbxAnimLayer* animLayer);
void displayCurveKeys(FbxAnimCurve* curve);

Converter::Converter()
{
	manager = FbxManager::Create();
	settings = FbxIOSettings::Create(manager, IOSROOT);
	manager->SetIOSettings(settings);
	scene = FbxScene::Create(manager, "");
	importer = FbxImporter::Create(manager, "");
	this->meshName = "mesh.fbx";
}

Converter::Converter(const char * fileName)
{
	manager = FbxManager::Create();
	settings = FbxIOSettings::Create(manager, IOSROOT);
	manager->SetIOSettings(settings);
	scene = FbxScene::Create(manager, "");
	importer = FbxImporter::Create(manager, "");
	this->meshName = fileName;

}

Converter::~Converter()
{
	scene->Destroy();
	settings->Destroy();
	manager->Destroy();
}

void Converter::importMesh()
{
	if (!importer->Initialize(meshName, -1, manager->GetIOSettings()))
	{
		printf("Call to fbximporter::initialize failed.\n");
		printf("Error returned: %s\n\n", importer->GetStatus().GetErrorString());
		getchar();
		exit(-1);
	}

	importer->Import(scene);
	importer->Destroy();

	rootNode = scene->GetRootNode();

	exportAnimation(scene, rootNode);
	//exportFile(rootNode);
}

void Converter::exportFile(FbxNode* currentNode)
{
	child = currentNode->GetChild(0);
	printf("Node: %s\n", currentNode->GetName());

	int materialCount = child->GetMaterialCount();
	std::cout << "Material count: " << materialCount << std::endl << std::endl;

	FbxPropertyT<FbxDouble3> lKFbxDouble3;
	FbxColor color;
	if (materialCount > 0)
	{
		for (int mat = 0; mat < materialCount; mat++)
		{
			FbxSurfaceMaterial *material = child->GetMaterial(mat);
			std::cout << "Material name: " << material->GetName() << std::endl;
			FbxSurfaceMaterial *lMaterial = child->GetMaterial(mat);

			if (lMaterial->GetClassId().Is(FbxSurfaceLambert::ClassId))
			{
				lKFbxDouble3 = ((FbxSurfaceLambert*)lMaterial)->Ambient;

			}
		}
	}

	getchar();

	mesh = child->GetMesh();
	
	if (mesh)
	{
		polygonCount = mesh->GetPolygonCount();

		//Vertices
		controlPoints = mesh->GetControlPoints();

		Counter counter;
		counter.vertexCount = polygonCount * 3;

		Vertex* vertices = new Vertex[counter.vertexCount];

		std::vector<FbxVector4> pos;
		std::vector<FbxVector4> norm;
		std::vector<FbxVector2> uv;
		FbxVector4 tempNorm;
		FbxVector2 tempUV;
	

		bool ItIsFalse = false;
		printf("\nMesh: %s\n", child->GetName());

		int i = 0;
		for (int polygonIndex = 0; polygonIndex < polygonCount; polygonIndex++)
		{
			for (int vertexIndex = 0; vertexIndex < mesh->GetPolygonSize(polygonIndex); vertexIndex++)
			{
				//Positions
				pos.push_back(controlPoints[mesh->GetPolygonVertex(polygonIndex, vertexIndex)]);

				//Normals
				mesh->GetPolygonVertexNormal(polygonIndex, vertexIndex, tempNorm);
				norm.push_back(tempNorm);

				//UVs
				FbxStringList uvSetNamesList;
				mesh->GetUVSetNames(uvSetNamesList);
				const char* uvNames = uvSetNamesList.GetStringAt(0);
				mesh->GetPolygonVertexUV(polygonIndex, vertexIndex, uvNames, tempUV, ItIsFalse);
				uv.push_back(tempUV);

				vertices[i].x = (float)pos[i][0];
				vertices[i].y = (float)pos[i][1];
				vertices[i].z = (float)pos[i][2];

				vertices[i].nx = (float)norm[i][0];
				vertices[i].ny = (float)norm[i][1];
				vertices[i].nz = (float)norm[i][2];

				vertices[i].u = (float)uv[i][0];
				vertices[i].v = (float)uv[i][1];

				i++;
			}
		}

		//Material & Texture

		//Attached the mesh's material to ourMaterial
		FbxSurfaceLambert* ourMaterial = child->GetSrcObject<FbxSurfaceLambert>(0);
		if (ourMaterial)
		{
			FBXSDK_printf("Found Material!\n");
		}
		else
		{
			FBXSDK_printf("Error: Material missing\n");
		}
		getchar();

		//Custom Creation
		size_t len = strlen(meshName);
		char* ret = new char[len + 2];
		strcpy(ret, meshName);
		ret[len - 3] = 'l';
		ret[len - 2] = 'e';
		ret[len - 1] = 'a';
		ret[len] = 'p';
		ret[len + 1] = '\0';
		meshName = ret;

		std::ofstream outfile(meshName, std::ofstream::binary);

		outfile.write((const char*)&counter, sizeof(Counter));
		outfile.write((const char*)vertices, sizeof(Vertex)*counter.vertexCount);

		outfile.close();

		delete[] vertices;
		//delete[] ret;
	}
	else
	{
		printf("Access violation: Mesh not found\n\n");
		exit(-2);
	}
}

void Converter::exportAnimation(FbxScene * scene, FbxNode* node)
{
	//GetSrcObjectCount: Returns the number of source objects with which this object connects. 
	for (int i = 0; i < scene->GetSrcObjectCount<FbxAnimStack>(); i++)
	{
		//AnimStack: The Animation stack is a collection of animation layers.
		//GetSrcObject: Returns the source object with which this object connects at the specified index.
		FbxAnimStack* animStack = scene->GetSrcObject<FbxAnimStack>(i);

		FbxString outputString = "Animation Stack Name: ";
		outputString += animStack->GetName();
		outputString += "\n";
		FBXSDK_printf(outputString);

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
		FBXSDK_printf(outputString);

		for (int j = 0; j < animLayers; j++)
		{
			//GetMember: Returns the member of the collection at the given index. 
			FbxAnimLayer* currentAnimLayer = animStack->GetMember<FbxAnimLayer>(j);
			outputString = "Current Animation Layer: ";
			outputString += j;
			outputString += "\n";
			FBXSDK_printf(outputString);

			getAnimation(currentAnimLayer, node);
		}
	}
}

void getAnimation(FbxAnimLayer* animLayer, FbxNode* node)
{
	int modelCount;
	FbxString outputString;

	outputString = "   Node/Joint Name: ";
	outputString += node->GetName();
	outputString += "\n";
	FBXSDK_printf(outputString);

	getAnimationChannels(node, animLayer);
	FBXSDK_printf("\n");

	for (modelCount = 0; modelCount < node->GetChildCount(); modelCount++)
	{
		getAnimation(animLayer, node->GetChild(modelCount));
	}
}

void getAnimationChannels(FbxNode* node, FbxAnimLayer* animLayer)
{
	//AnimCurve: An animation curve, defined by a collection of keys (FbxAnimCurveKey), and indicating how a value changes over time. 
	FbxAnimCurve* animCurve = NULL;

	//LclTranslation: This property contains the translation information of the node. 
	animCurve = node->LclTranslation.GetCurve(animLayer, FBXSDK_CURVENODE_COMPONENT_X);
	if (animCurve)
	{
		FBXSDK_printf("   TX\n");
		displayCurveKeys(animCurve);
	}
	animCurve = node->LclTranslation.GetCurve(animLayer, FBXSDK_CURVENODE_COMPONENT_Y);
	if (animCurve)
	{
		FBXSDK_printf("   TY\n");
		displayCurveKeys(animCurve);
	}
	animCurve = node->LclTranslation.GetCurve(animLayer, FBXSDK_CURVENODE_COMPONENT_Z);
	if (animCurve)
	{
		FBXSDK_printf("   TZ\n");
		displayCurveKeys(animCurve);
	}
	//----------------------------------------------------------------------------------
	animCurve = node->LclRotation.GetCurve(animLayer, FBXSDK_CURVENODE_COMPONENT_X);
	if (animCurve)
	{
		FBXSDK_printf("   RX\n");
		displayCurveKeys(animCurve);
	}
	animCurve = node->LclRotation.GetCurve(animLayer, FBXSDK_CURVENODE_COMPONENT_Y);
	if (animCurve)
	{
		FBXSDK_printf("   RY\n");
		displayCurveKeys(animCurve);
	}
	animCurve = node->LclRotation.GetCurve(animLayer, FBXSDK_CURVENODE_COMPONENT_Z);
	if (animCurve)
	{
		FBXSDK_printf("   RZ\n");
		displayCurveKeys(animCurve);
	}
}

void displayCurveKeys(FbxAnimCurve* curve)
{
	//FbxTime: Class to encapsulate time units, is just used to represent a moment. Can measure time in hour, minute, second, frame, field, residual and also combination of these units.
	FbxTime keyTime;
	float keyValue;
	char timeString[256];
	FbxString outputString;
	//KeyGetCount: Get the number of keys.
	int keyCount = curve->KeyGetCount();

	for (int i = 0; i < keyCount; i++)
	{
		//Get key value depending on current curve and LclRotation/Translation/Scale and FBXSDK_CURVENODE_COMPONENT_??
		keyValue = static_cast<float>(curve->KeyGetValue(i));
		//KeyGetTime: Returns key time (time at which this key is occurring). 
		keyTime = curve->KeyGetTime(i);

		outputString = "      Key Time: ";
		outputString += keyTime.GetTimeString(timeString, FbxUShort(256));
		outputString += ".... Key Value: ";
		outputString += keyValue;
		outputString += "\n";
		FBXSDK_printf(outputString);
	}
}
