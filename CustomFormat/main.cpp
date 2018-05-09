#include <fbxsdk.h>
#include "Converter.h"
#include <crtdbg.h>

int main()
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	
	Converter Cliffside_4("Cliffside_4.fbx");
	Cliffside_4.importMesh();

	Cliffside_4.isLevel = false; // change this when loading levels


	getchar();
	return 0;
}