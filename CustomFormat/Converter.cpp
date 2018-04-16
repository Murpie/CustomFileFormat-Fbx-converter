#include "Converter.h"
#include "MeshStructs.h"
#include <fstream>
#include <vector>

#pragma warning(disable : 4996)

Converter::Converter()
{
	manager = FbxManager::Create();
	settings = FbxIOSettings::Create(manager, IOSROOT);
	manager->SetIOSettings(settings);
	ourScene = FbxScene::Create(manager, "");
	importer = FbxImporter::Create(manager, "");
	this->meshName = "mesh.fbx";

}

Converter::Converter(const char * fileName)
{
	manager = FbxManager::Create();
	settings = FbxIOSettings::Create(manager, IOSROOT);
	manager->SetIOSettings(settings);
	ourScene = FbxScene::Create(manager, "");
	importer = FbxImporter::Create(manager, "");
	this->meshName = fileName;
}

Converter::~Converter()
{
	ourScene->Destroy();
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

	importer->Import(ourScene);
	importer->Destroy();

	rootNode = ourScene->GetRootNode();

	exportFile(rootNode);
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

				//Material


				//printf("Vertex[%d]: %f %f %f\n", i, pos[i][0], pos[i][1], pos[i][2]);
				//printf("Normal[%d]: %f %f %f\n", i, norm[i][0], norm[i][1], norm[i][2]);
				//printf("UV[%d]:     %f %f\n\n", i, uv[i][0], uv[i][1]);

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

		/*size_t len = strlen(meshName);
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

		outfile.close();*/

		delete[] vertices;
		//delete[] ret;
	}
	else
	{
		printf("Access violation: Mesh not found\n\n");
		exit(-2);
	}
}

/*
#include <iostream>
#include <fstream>
#include <filesystem>

int main()
{
std::ifstream src("C:/Users/Elin/Downloads/Colors.png", std::ios::binary);
std::ofstream dst("C:/Users/Elin/Desktop/NewColors.png", std::ios::binary);
dst << src.rdbuf();
}
*/
