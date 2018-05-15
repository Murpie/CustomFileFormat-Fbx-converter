#include <fbxsdk.h>
#include "Converter.h"
#include <crtdbg.h>

int main()
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	
	Converter converter("BlendShapeMesh.fbx");
	Converter converter1("HandAnimation2.fbx");

	converter.isLevel = false; // change this when loading levels
	converter.importMesh();

	converter1.isLevel = false;
	converter1.importMesh();

	getchar();
	return 0;
}