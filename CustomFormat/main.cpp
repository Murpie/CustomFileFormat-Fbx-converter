#include <fbxsdk.h>
#include "Converter.h"
#include <crtdbg.h>

int main()
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	
	Converter converter("ValleyProps.fbx");

	converter.isLevel = true; // change this when loading levels

	converter.importMesh();

	getchar();
	return 0;
}