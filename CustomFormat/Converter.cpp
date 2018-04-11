#include "Converter.h"
#include <fstream>
#include <iostream>
#include "MeshStructs.h"
#include <vector>

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

	loadMesh(rootNode);
	//exportFile();
	exportFile2();
}


Converter::~Converter()
{
	manager->Destroy();
}

void Converter::loadMesh(FbxNode* node)
{
	for (int i = 0; i < rootNode->GetChildCount(); i++)
	{
		child = node->GetChild(i);
		mesh = child->GetMesh();
		polygonCount = mesh->GetPolygonCount();

		//Normals
		normalElement = mesh->GetElementNormal();

		//Vertices
		controlPoints = mesh->GetControlPoints();
	}
}

void Converter::exportFile()
{
	Counter counter;
	counter.vertexCount = polygonCount * 3;
	int loop = 0;

	Vertex *vertices = new Vertex[counter.vertexCount];
	
	for (int i = 0; i < polygonCount; i++)
	{
		int normalIndex = 0;

		if (normalElement->GetMappingMode() == FbxGeometryElement::eByControlPoint)
		{
			int polygonSize = mesh->GetPolygonSize(i);

			for (int vertexIndex = 0; vertexIndex < polygonSize; vertexIndex++)
			{
				vertices[loop].x = controlPoints[i][0];
				vertices[loop].y = controlPoints[i][1];
				vertices[loop].z = controlPoints[i][2];
				std::cout << loop << " x: " << vertices[vertexIndex].x << "  y: " << vertices[vertexIndex].y << "  z: " << vertices[vertexIndex].z << std::endl;

				if (normalElement->GetReferenceMode() == FbxGeometryElement::eDirect)
				{
					normalIndex = loop;
				}

				if (normalElement->GetReferenceMode() == FbxGeometryElement::eIndexToDirect)
				{
					normalIndex = normalElement->GetIndexArray().GetAt(loop);
				}

				normal = normalElement->GetDirectArray().GetAt(normalIndex);
			
				vertices[loop].nx = normal[0];
				vertices[loop].ny = normal[1];
				vertices[loop].nz = normal[2];

				std::cout << "nx: " << vertices[vertexIndex].nx << "  ny: " << vertices[vertexIndex].ny << "  nz: " << vertices[vertexIndex].nz << std::endl << std::endl;
				loop++;
			}
		}
	}
	getchar();

	/*
	std::cout << std::endl << 0 << "   x: " << vertices[0].x << "   y: " << vertices[0].y << "   z: " << vertices[0].z << std::endl;
	std::cout << std::endl << 1 << "   x: " << vertices[1].x << "   y: " << vertices[1].y << "   z: " << vertices[1].z << std::endl;
	getchar();

	std::ofstream outfile("readTextFile.txt", std::ios::app);
	outfile.write((const char*)&counter, sizeof(Counter));
	outfile.write((const char*)vertices, sizeof(Vertex)*counter.vertexCount);
	*/

	std::ofstream outfile("testt.leap", std::ofstream::binary);

	outfile.write((const char*)&counter, sizeof(Counter));
	outfile.write((const char*)vertices, sizeof(Vertex)*counter.vertexCount);

	outfile.close();
}

void Converter::exportFile2()
{
	Counter counter;
	counter.vertexCount = polygonCount * 3;

	Vertex *vertices = new Vertex[counter.vertexCount];

	std::vector<FbxVector4> pos;

	int i = 0;
	for (int polygonIndex = 0; polygonIndex < polygonCount; polygonIndex++)
	{
		for (int vertexIndex = 0; vertexIndex < mesh->GetPolygonSize(polygonIndex); vertexIndex++)
		{
			pos.push_back(controlPoints[mesh->GetPolygonVertex(polygonIndex, vertexIndex)]);

			//std::cout << i << "  x: " << vertices[i].x << "  y: " << vertices[i].y << "  z: " << vertices[i].z << std::endl;
			//printf("Vertex[%d]: %f %f %f\n", i, (char)pos[i]);
			i++;
		}
	}
	//std::cout << pos[0] << std::endl;
	getchar();
}

void Converter::printName(FbxMesh* meshName)
{
	if (meshName)
	{
		printf("Name: %s\n\n", mesh->GetName());
	}
}
