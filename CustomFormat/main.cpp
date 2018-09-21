#include <fbxsdk.h>
#include "Converter.h"
#include <crtdbg.h>

int main()
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	
	Converter converter("cubeSpin.fbx");

	converter.isLevel = false; // change this when loading levels
	converter.importMesh();

	//getchar();
	std::cout << "Done, press any key";
	getchar();
	return 0;
}