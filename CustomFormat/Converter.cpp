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

}

Converter::~Converter()
{
	manager->Destroy();
}

void Converter::importMesh(const char* filename)
{
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

	exportFile(rootNode);
}

void Converter::exportFile(FbxNode* currentNode)
{
	child = currentNode->GetChild(0);
	printf("Node: %s", currentNode->GetName());
	
	mesh = child->GetMesh();
	
	if (mesh)
	{
		//Vertex Information
		Counter counter;
		counter.vertexCount = polygonCount * 3;
		Vertex *vertices = new Vertex[counter.vertexCount];
		{
			polygonCount = mesh->GetPolygonCount();
			controlPoints = mesh->GetControlPoints();

			std::vector<FbxVector4> pos;
			std::vector<FbxVector4> norm;
			std::vector<FbxVector2> uv;
			FbxVector4 temp;
			FbxVector2 tempUv;

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
					mesh->GetPolygonVertexNormal(polygonIndex, vertexIndex, temp);
					norm.push_back(temp);

					//UVs
					FbxStringList uvSetNamesList;
					mesh->GetUVSetNames(uvSetNamesList);
					const char* uvNames = uvSetNamesList.GetStringAt(0);
					mesh->GetPolygonVertexUV(polygonIndex, vertexIndex, uvNames, tempUv, ItIsFalse);
					uv.push_back(tempUv);

					printf("Vertex[%d]: %f %f %f\n", i, pos[i][0], pos[i][1], pos[i][2]);
					printf("Normal[%d]: %f %f %f\n", i, norm[i][0], norm[i][1], norm[i][2]);
					printf("UV[%d]:     %f %f\n\n", i, uv[i][0], uv[i][1]);

					vertices[i].x = pos[i][0];
					vertices[i].y = pos[i][1];
					vertices[i].z = pos[i][2];

					vertices[i].nx = norm[i][0];
					vertices[i].ny = norm[i][1];
					vertices[i].nz = norm[i][2];

					vertices[i].u = uv[i][0];
					vertices[i].v = uv[i][1];

					i++;
				}
			}
		}

		//Material & Texture Information
		

		std::ofstream outfile("testt.leap", std::ofstream::binary);

		outfile.write((const char*)&counter, sizeof(Counter));
		outfile.write((const char*)vertices, sizeof(Vertex)*counter.vertexCount);

		outfile.close();
	}
}
