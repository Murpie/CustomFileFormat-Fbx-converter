#include <fbxsdk.h>
#include "Converter.h"

int main()
{
	Converter converter;

	const char* filename = "testShapes.fbx";

	converter.loadMesh(filename);

	getchar();
	return 0;
}