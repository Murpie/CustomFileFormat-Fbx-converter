#include <fbxsdk.h>
#include "Converter.h"
#include <crtdbg.h>
//Lucas
int main()
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	
	//Converter converter("CamLight.fbx");
	Converter converter("FBXcustomAttribute.fbx");

	converter.importMesh();

	getchar();
	return 0;
}