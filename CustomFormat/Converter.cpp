#include "Converter.h"



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

void Converter::loadMesh(const char* fileName)
{
	FbxImporter* importer = FbxImporter::Create(manager, "");

	if (!importer->Initialize(fileName, -1, manager->GetIOSettings()))
	{
		printf("Call to fbximporter::initialize failed.\n");
		printf("Error returned: %s\n\n", importer->GetStatus().GetErrorString());
		getchar();
		exit(-1);
	}

	rootNode = ourScene->GetRootNode();

	if (rootNode)
	{
		if (rootNode->GetChildCount() > 0)
		{
			printf("Several childs were found\n\n");
			for (int i = 0; i < rootNode->GetChildCount(); i++)
			{
				printName(rootNode->GetChild(i));
			}
		}
		else
		{
			printf("A single child was found\n\n");
			printName(rootNode);
		}
	}
	else
	{
		printf("Error: Node not found\n\n");
	}
}

void Converter::printName(FbxNode* nodeName)
{
	mesh = nodeName->GetMesh();

	if (mesh)
	{
		printf("Name: %s\n\n", mesh->GetName());
		printf("Hello");
	}
	else
	{
		printf("Error: Mesh not loaded!\n\n");
	}
}
