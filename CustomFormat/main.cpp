#include <fbxsdk.h>
#include "Converter.h"
#include <crtdbg.h>


int main()
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	
	Converter converter("Snowman.fbx");

	converter.importMesh();

	return 0;
}