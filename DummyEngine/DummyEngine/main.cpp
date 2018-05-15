#include "LeapImporter.h"
//int main(int argc, char** argv)
int main()
{

	/*fprintf(stderr, "arguments: %d\n", argc);
	fprintf(stderr, "arguments: %s\n", argv[1]);
	fflush(NULL);
	getchar();*/
	LeapImporter importer;

	LeapMesh* mesh = importer.getMesh("GroupTest.leap");
	cout << mesh->group->groupName << endl << mesh->group->childCount << endl;
	for (int i = 0; i < mesh->group->childCount; i++)
	{
		cout << "Children Names: " << i << " " << mesh->group->childName[i] << endl;
	}
	//int vertexCount = mesh->getVertexCount();
	//unsigned int customMeshType = (int)mesh->customMayaAttribute;

	//printf("%d\n", vertexCount);

	//for (int i = 0; i < vertexCount; i++)
	//{
	//	printf("Vertex: %d \nPositions: %f %f %f\n", i, mesh->vertices[i].x, mesh->vertices[i].y, mesh->vertices[i].z);
	//	printf("Normals: %f %f %f\n", mesh->vertices[i].nx, mesh->vertices[i].ny, mesh->vertices[i].nz);
	//	printf("UVs: %f %f\n\n", mesh->vertices[i].u, mesh->vertices[i].v);
	//}
	//for (int i = 0; i < mesh->boundingBoxes.size(); i++)
	//{
	//	printf("BBox minVector: %f %f %f\n", mesh->boundingBoxes[i]->minVector[0], mesh->boundingBoxes[i]->minVector[1], mesh->boundingBoxes[i]->minVector[2]);
	//	printf("BBox maxVector: %f %f %f\n", mesh->boundingBoxes[i]->maxVector[0], mesh->boundingBoxes[i]->maxVector[1], mesh->boundingBoxes[i]->maxVector[2]);
	//	printf("BBox center: %f %f %f\n\n", mesh->boundingBoxes[i]->center[0], mesh->boundingBoxes[i]->center[1], mesh->boundingBoxes[i]->center[2]);
	//}
	//printf("CustomMayaAttribute: %d", mesh->customMayaAttribute);

	importer.deleteObject(mesh);

	int vertexCount = mesh->getVertexCount();

	std::cout << "Animation name: " << mesh->animation->animationName << std::endl;
	std::cout << "Nr of joints: " << mesh->animation->nrOfJoints << std::endl;
	std::cout << "Keyframe count: " << mesh->animation->keyFrameCount << std::endl << std::endl;

	for (int i = 0; i <  mesh->animation->nrOfJoints; i++)
	{
		std::cout << "Joint name: " << mesh->animation->joints[i].jointName << std::endl;
		std::cout << "Parent name: " << mesh->animation->joints[i].parentName << std::endl;
		for (int j = 0; j <  mesh->animation->keyFrameCount; j++)
		{
			std::cout << "Keyframe[" << j << "]" << std::endl;
			std::cout << "Time: " << mesh->animation->joints[i].keyFrames[j].time << std::endl;
			std::cout << "TX: " << mesh->animation->joints[i].keyFrames[j].position[0];
			std::cout << " | TY: " << mesh->animation->joints[i].keyFrames[j].position[1];
			std::cout << " | TZ: " << mesh->animation->joints[i].keyFrames[j].position[2] << std::endl;

			std::cout << "RX: " << mesh->animation->joints[i].keyFrames[j].rotation[0];
			std::cout << " | RY: " << mesh->animation->joints[i].keyFrames[j].rotation[1];
			std::cout << " | RZ: " << mesh->animation->joints[i].keyFrames[j].rotation[2] << std::endl;

			std::cout << "SX: " << mesh->animation->joints[i].keyFrames[j].scaling[0];
			std::cout << " | SY: " << mesh->animation->joints[i].keyFrames[j].scaling[1];
			std::cout << " | SZ: " << mesh->animation->joints[i].keyFrames[j].scaling[2] << std::endl << std::endl;
		}
	}

	//for (int i = 0; i < vertexCount; i++)
	//{
	//	printf("Vertex: %d \nPositions: %f %f %f\n", i, mesh->vertices[i].x, mesh->vertices[i].y, mesh->vertices[i].z);
	//	printf("Normals: %f %f %f\n", mesh->vertices[i].nx, mesh->vertices[i].ny, mesh->vertices[i].nz);
	//	printf("UVs: %f %f\n\n", mesh->vertices[i].u, mesh->vertices[i].v);
	//}

	//void Converter::printInformation()
	//{
	//	std::cout << "Animation Name: " << animationInfo->animationName << std::endl;
	//	std::cout << "Keyframe Count: " << animationInfo->keyFrameCount << std::endl;
	//	std::cout << "Nr of joints: " << animationInfo->nrOfJoints << std::endl << std::endl;
	//	for (int i = 0; i < animationInfo->nrOfJoints; i++)
	//	{
	//		std::cout << "Joint name: " << animationInfo->joints[i].jointName << std::endl;
	//		std::cout << "Parent name: " << animationInfo->joints[i].parentName << std::endl;
	//		for (int j = 0; j < animationInfo->joints[i].keyFrames.size(); j++)
	//		{
	//			std::cout << "Keyframe[" << j << "]" << std::endl;
	//			std::cout << "Time: " << animationInfo->joints[i].keyFrames[j].time << std::endl;
	//			std::cout << "TX: " << animationInfo->joints[i].keyFrames[j].position[0] << std::endl;
	//			std::cout << "TY: " << animationInfo->joints[i].keyFrames[j].position[1] << std::endl;
	//			std::cout << "TZ: " << animationInfo->joints[i].keyFrames[j].position[2] << std::endl;

	//			std::cout << "RX: " << animationInfo->joints[i].keyFrames[j].rotation[0] << std::endl;
	//			std::cout << "RY: " << animationInfo->joints[i].keyFrames[j].rotation[1] << std::endl;
	//			std::cout << "RZ: " << animationInfo->joints[i].keyFrames[j].rotation[2] << std::endl;

	//			std::cout << "SX: " << animationInfo->joints[i].keyFrames[j].scaling[0] << std::endl;
	//			std::cout << "SY: " << animationInfo->joints[i].keyFrames[j].scaling[1] << std::endl;
	//			std::cout << "SZ: " << animationInfo->joints[i].keyFrames[j].scaling[2] << std::endl << std::endl;
	//		}
	//	}
	//}

//	printf("|| Size: %d\n", level->levelObjects.size());

	//for (int i = 0; i < level->levelObjects.size(); i++)
	//{
	//	printf("|| Translation: \n");
	//	printf("|| %f\n", level->levelObjects[i]->x);
	//	printf("|| %f\n", level->levelObjects[i]->y);
	//	printf("|| %f\n", level->levelObjects[i]->z);
	//	printf("|| ID: \n");
	//	printf("|| %d\n\n", level->levelObjects[i]->id);
	//}
	//
	std::getchar();
	//importer.deleteObject(level);
	return 0;
}