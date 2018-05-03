#include <fbxsdk.h>
#include "Converter.h"
#include <crtdbg.h>
//Leon
int main()
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	


	converter.isLevel = false; // change this when loading levels
	Converter converter("GroupTest.fbx");

	converter.importMesh();

	getchar();
	return 0;
}