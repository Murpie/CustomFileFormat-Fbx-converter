#include <fbxsdk.h>

int main()
{
	printf("hello");

	FbxManager* manager = FbxManager::Create();

	FbxScene* ourScene = FbxScene::Create(manager, "");

	FbxIOSettings* ios = FbxIOSettings::Create(manager, IOSROOT);
	manager->SetIOSettings(ios);

	FbxImporter* fbximporter = FbxImporter::Create(manager, "");

	const char* filename = "testCube.fbx";

	if (!fbximporter->Initialize(filename, -1, manager->GetIOSettings()))
	{
		printf("Call to fbximporter::initialize failed.\n");
		printf("Error returned: %s\n\n", fbximporter->GetStatus().GetErrorString());
		getchar();
		exit(-1);
	}

	return 0;
}