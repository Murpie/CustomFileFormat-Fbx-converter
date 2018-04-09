#include "Converter.h"



Converter::Converter()
{
	manager = FbxManager::Create();
	settings = FbxIOSettings::Create(manager, IOSROOT);
	manager->SetIOSettings(settings);
	ourScene = FbxScene::Create(manager, "");
	importer = FbxImporter::Create(manager, "");

	const char* filename = "testShapes.fbx";

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

	loadMesh(rootNode);
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

		FBXSDK_printf("Polygon Count: %d\n", polygonCount);

		//Normals
		FbxLayerElementNormal* normalElement = mesh->GetElementNormal();
		/*if (normalElement)
		{
			for (int vertexIndex = 0; vertexIndex < mesh->GetControlPointsCount(); vertexIndex++)
			{
				int normalIndex = 0;

				if (normalElement->GetReferenceMode() == FbxGeometryElement::eDirect)
				{
					normalIndex = vertexIndex;
				}

				if (normalElement->GetReferenceMode() == FbxGeometryElement::eIndexToDirect)
				{
					normalIndex = normalElement->GetIndexArray().GetAt(vertexIndex);
				}

				FbxVector4 normal = normalElement->GetDirectArray().GetAt(normalIndex);

				FBXSDK_printf("Normals for vertex[%d]: %f %f %f %f \n", vertexIndex, normal[0], normal[1], normal[2], normal[3]);

			}
		}*/

		//GetPolygon
		int vertexId = 0;
		char header[100];
		FbxVector4* controlPoints = mesh->GetControlPoints();

		for (int i = 0; i < polygonCount; i++)
		{
			FBXSDK_printf("\n\nPolygon: %d\n", i);
			int polygonSize = mesh->GetPolygonSize(i);

			for (int j = 0; j < polygonSize; j++)
			{
				int polygonVertexIndex = mesh->GetPolygonVertex(i, j);
				switch (j)
				{
				case 0:
					FBXSDK_printf("X: %f\t", controlPoints[polygonVertexIndex]);
					break;
				case 1:
					FBXSDK_printf("Y: %f\t", controlPoints[polygonVertexIndex]);
					break;
				case 2:
					FBXSDK_printf("Z: %f\t", controlPoints[polygonVertexIndex]);
					break;
				case 3:
					FBXSDK_printf("???: %f\t", controlPoints[polygonVertexIndex]);
					break;
				default:
					break;
				}
			}

			/*for (int k = 0; k < mesh->GetElementUVCount(); k++)
			{
				FbxGeometryElementUV* uv = mesh->GetElementUV(k);
				FBXSDK_printf("UV: %d\n", k);
				switch (uv->GetReferenceMode())
				{
				case FbxGeometryElement::eByControlPoint:
					switch (uv->GetReferenceMode())
					{
					case FbxGeometryElement::eDirect:
						FBXSDK_printf("U: &f\tV: %f", uv->GetDirectArray().GetAt(polygonVertexIndex[0]), )
					default:
						break;
					}
				default:
					break;
				}
			}*/
		}
	}
}

void Converter::printName(FbxMesh* meshName)
{
	if (meshName)
	{
		printf("Name: %s\n\n", mesh->GetName());
	}
}
