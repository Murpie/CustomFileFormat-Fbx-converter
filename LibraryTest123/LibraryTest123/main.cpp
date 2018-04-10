#include <iostream>
#include <string>

using namespace std;

#include "CustomImporter.h"

#pragma comment(lib, "Format Importer.lib")

int main() {

	CustomImporter loader;

	string meshArray[] = { "hello.obj" };
	string cameraArray[] = { "hello.obj", "hello.obj" };
	string lighthArray[] = { "hello.obj", "hello.obj" };

	float verticeX;

	for (int i = 0; i < sizeof(meshArray) / sizeof(meshArray[0]); i++)
	{
		Mesh* mesh = loader.getMesh(meshArray[i]);
		verticeX = mesh->vertices[0].x;
		//Save Info To Gameobject
		loader.deleteObject(mesh);
	}

	cout << verticeX << endl;

	Camera* camera = loader.getCamera("cameraFile");
	Light* light = loader.getLight("lightFile");
	


	getchar();

	return 0;
}