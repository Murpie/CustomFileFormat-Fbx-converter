#include <fbxsdk.h>
#include "Converter.h"

int main()
{
	Converter converter;

	converter.importMesh("wierdBox.fbx");

	getchar();
	return 0;
}