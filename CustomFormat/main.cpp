#include <fbxsdk.h>

int main()
{
	printf("hello");

	FbxManager* manager = FbxManager::Create();

	FbxScene* myScene = FbxScene::Create(manager, "");

	FbxIOSettings* ios = FbxIOSettings::Create(manager, IOSROOT);
	manager->SetIOSettings(ios);

	FbxImporter* importer = FbxImporter::Create(manager, "");

	const char* filename = "testCube.fbx";

	if (!importer->Initialize(filename, -1, manager->GetIOSettings()))
	{
		printf("Call to fbximporter::initialize failed.\n");
		printf("Error returned: %s\n\n", importer->GetStatus().GetErrorString());
		getchar();
		exit(-1);
	}

	FbxExporter* exporter = FbxExporter::Create(manager, "");

	if (!exporter->Initialize(filename, -1, manager->GetIOSettings()))
	{
		printf("Call to fbximporter::initialize failed.\n");
		printf("Error returned: %s\n\n", exporter->GetStatus().GetErrorString());
		getchar();
		exit(-2);
	}


	return 0;
}