#include <fbxsdk.h>
#include "Converter.h"
#include <crtdbg.h>

int main()
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	
	Converter converter("Campfire_bitang.fbx");

	Cliffside_4.isLevel = false; // change this when loading levels

	converter.importMesh();

	getchar();
	return 0;
}