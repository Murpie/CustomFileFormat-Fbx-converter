#include "Converter.h"



Converter::Converter()
{
	manager = FbxManager::Create();
	settings = FbxIOSettings::Create(manager, IOSROOT);
	manager->SetIOSettings(settings);
	ourScene = FbxScene::Create(manager, "");
	importer = FbxImporter::Create(manager, "");

	const char* filename = "wierdBox.fbx";

	if (!importer->Initialize(filename, -1, manager->GetIOSettings()))
	{
		printf("Call to fbximporter::initialize failed.\n");
		printf("Error returned: %s\n\n", importer->GetStatus().GetErrorString());
		getchar();
		exit(-1);
	}

	importer->Import(ourScene);
	importer->Destroy();




	rootNode = ourScene->GetRootNode();

	//loadMesh(rootNode);
	exportFile();
}


Converter::~Converter()
{
	manager->Destroy();
}

void Converter::loadMesh(FbxNode* node)
{
	for (int i = 0; i < rootNode->GetChildCount(); i++)
	{
		FbxNode* child = node->GetChild(i);
		FBXSDK_printf("\nCurrent Mesh Node: %s\t", child->GetName());
		FbxMesh* mesh = child->GetMesh();
		int polygonCount = mesh->GetPolygonCount();

		FBXSDK_printf("\nPolygon Count: %d\n", polygonCount);

		//Normals
		FbxLayerElementNormal* normalElement = mesh->GetElementNormal();

		//Vertices
		FbxVector4* controlPoints = mesh->GetControlPoints();

		for (int i = 0; i < polygonCount; i++)
		{
			FBXSDK_printf("\n\nPolygon: %d\n", i);
			int normalIndex = 0;

			//Print vertices
			FBXSDK_printf("X: %f\t", controlPoints[i][0]);
			FBXSDK_printf("Y: %f\t", controlPoints[i][1]);
			FBXSDK_printf("Z: %f\t\n", controlPoints[i][2]);

			if (normalElement->GetMappingMode() == FbxGeometryElement::eByControlPoint)
			{
				if (normalElement->GetReferenceMode() == FbxGeometryElement::eDirect)
				{
					normalIndex = i;
				}

				if (normalElement->GetReferenceMode() == FbxGeometryElement::eIndexToDirect)
				{
					normalIndex = normalElement->GetIndexArray().GetAt(i);
				}

				FbxVector4 normal = normalElement->GetDirectArray().GetAt(normalIndex);

				FBXSDK_printf("NX: %f\t", normal[0]);
				FBXSDK_printf("NY: %f\t", normal[1]);
				FBXSDK_printf("NZ: %f\t\n", normal[2]);
			}
			else if (normalElement->GetMappingMode() == FbxGeometryElement::eByPolygonVertex)
			{
				int polygonSize = mesh->GetPolygonSize(i);

				if (normalElement->GetReferenceMode() == FbxGeometryElement::eDirect)
				{
					normalIndex = i;
				}

				if (normalElement->GetReferenceMode() == FbxGeometryElement::eIndexToDirect)
				{
					normalIndex = normalElement->GetIndexArray().GetAt(i);
				}

				FbxVector4 normal = normalElement->GetDirectArray().GetAt(normalIndex);
				FBXSDK_printf("PNX: %f\t", normal[0]);
				FBXSDK_printf("PNY: %f\t", normal[1]);
				FBXSDK_printf("PNZ: %f\t\n", normal[2]);
			}
		}
	}
}

void Converter::exportFile()
{

}

void Converter::printName(FbxMesh* meshName)
{
	if (meshName)
	{
		printf("Name: %s\n\n", mesh->GetName());
	}
}
