#include "LeapImporter.h"
//int main(int argc, char** argv)
int main()
{
	LeapImporter importer;
	LeapLevel* level = importer.getLevel("test_level.ssp");




	//if (true)
	//{
	//	char answer;

	//	//Counter
	//	printf("Print Counter? Y/N:\t");
	//	std::cin >> answer;
	//	getchar();
	//	if (answer == 'Y' || answer == 'y')
	//	{
	//		printf("\n\tVertex Count:\t\t%d\n", mesh->counterReader.vertexCount);
	//		printf("\tMesh Count:\t\t%d\n", mesh->counterReader.meshCount);
	//		/*printf("\tBlend Shape Count:\t%d\n", mesh->counterReader.blendShapeCount);*/
	//		printf("\tCustom Attribute Count:\t%d\n", mesh->counterReader.customMayaAttributeCount);
	//		/*printf("\tLight Count:\t\t%d\n", mesh->counterReader.lightCount);
	//		printf("\tCamera Count:\t\t%d\n", mesh->counterReader.cameraCount);*/
	//		printf("\tMaterial Count:\t\t%d\n\n", mesh->counterReader.matCount);
	//	}


	//	//Global Transform
	//	printf("Print Global Tranform? Y/N:\t");
	//	std::cin >> answer;
	//	getchar();
	//	if (answer == 'Y' || answer == 'y')
	//	{
	//		for (int i = 0; i < mesh->counterReader.meshCount; i++)
	//		{
	//			printf("\t%s\n", mesh[i].transform->meshName);
	//			printf("\tGlobal Transform:	X: %f Y: %f Z: %f\n\n", mesh[i].transform->globalTranslation[0], mesh[i].transform->globalTranslation[1], mesh[i].transform->globalTranslation[2]);
	//		}
	//	}

	//	//Vertex Information
	//	printf("Printf VertexInformation? Y/N:\t");
	//	std::cin >> answer;
	//	getchar();
	//	if (answer == 'Y' || answer == 'y')
	//	{
	//		if (mesh->counterReader.vertexCount != 0)
	//		{
	//			for (int i = 0; i < mesh->counterReader.vertexCount; i++)
	//			{
	//				printf("\n\t|%d|Vertex:    X: %f Y: %f Z: %f\n", i, mesh->vertices[i].x, mesh->vertices[i].y, mesh->vertices[i].z);
	//				printf("\t|%d|Normals:   X: %f Y: %f Z: %f\n", i, mesh->vertices[i].nx, mesh->vertices[i].ny, mesh->vertices[i].nz);
	//				printf("\t|%d|Binormals: X: %f Y: %f Z: %f\n", i, mesh->vertices[i].bnx, mesh->vertices[i].bny, mesh->vertices[i].bnz);
	//				printf("\t|%d|Tangents:  X: %f Y: %f Z: %f\n", i, mesh->vertices[i].tx, mesh->vertices[i].ty, mesh->vertices[i].tz);

	//				for (int j = 0; j < 4; j++)
	//				{
	//					printf("\t|%d|WeightID|%d|: %f\n", i, mesh->vertices[i].weightID[j], mesh->vertices[i].weight[j]);
	//				}
	//				printf("\n");
	//			}
	//		}
	//		else
	//		{
	//			printf("There's no Vertex Information\n\n");
	//		}
	//	}

	//	//Material Information
	//	printf("Print Material Information? Y/N:\t");
	//	std::cin >> answer;
	//	getchar();
	//	if (answer == 'Y' || answer == 'y')
	//	{
	//		if (mesh->counterReader.matCount != 0)
	//		{
	//			for (int i = 0; i < mesh->counterReader.matCount; i++)
	//			{
	//				printf("\n\tAmbient:  %.2f %.2f %.2f\n", mesh->material[i].ambient[0], mesh->material[i].ambient[1], mesh->material[i].ambient[2]);
	//				printf("\tDiffuse:  %.2f %.2f %.2f\n", mesh->material[i].diffuse[0], mesh->material[i].diffuse[1], mesh->material[i].diffuse[2]);
	//				printf("\tEmissive: %.2f %.2f %.2f\n", mesh->material[i].emissive[0], mesh->material[i].emissive[1], mesh->material[i].emissive[2]);
	//				printf("\tOpacity:  %.2f\n", mesh->material[i].opacity);
	//				printf("\tTexture File Path: %s\n\n", mesh->material[i].textureFilePath);
	//			}
	//		}
	//		else
	//		{
	//			printf("There's no Material Information\n\n");
	//		}
	//	}

	//	//Skeleton Animation
	//	printf("Print Skeleton Animation? Y/N:\t");
	//	std::cin >> answer;
	//	getchar();
	//	if (answer == 'Y' || answer == 'y')
	//	{
	//		if (mesh->animation->keyFrameCount > 1)
	//		{
	//			printf("\n\tAnimation Name: %s\n", mesh->animation->animationName);
	//			printf("\tKey Frame Count: %d\n", mesh->animation->keyFrameCount);
	//			printf("\tJoint Count: %d\n", mesh->animation->nrOfJoints);


	//			printf("Print Joint & Keyframes? Y/N:\t");
	//			std::cin >> answer;
	//			getchar();
	//			if (answer == 'Y' || answer == 'y')
	//			{
	//				for (int i = 0; i < mesh->animation->nrOfJoints; i++)
	//				{
	//					printf("\n\tJoint Name: %s\n", mesh->animation->joints[i].jointName);
	//					printf("\tParent Name: %s\n", mesh->animation->joints[i].parentName);

	//					for (int j = 0; j < mesh->animation->keyFrameCount; j++)
	//					{
	//						printf("\t\tKey|%d|\tTime: %.3f", j, mesh->animation->joints[i].keyFrames[j].time);
	//						printf("\tPosition: %.3f %.3f %.3f", j, mesh->animation->joints[i].keyFrames[j].position[0], mesh->animation->joints[i].keyFrames[j].position[1], mesh->animation->joints[i].keyFrames[j].position[2]);
	//						printf("\tRotation: %.3f %.3f %.3f", j, mesh->animation->joints[i].keyFrames[j].rotation[0], mesh->animation->joints[i].keyFrames[j].rotation[1], mesh->animation->joints[i].keyFrames[j].rotation[2]);
	//						printf("\tScaling: %.3f %.3f %.3f\n", j, mesh->animation->joints[i].keyFrames[j].scaling[0], mesh->animation->joints[i].keyFrames[j].scaling[1], mesh->animation->joints[i].keyFrames[j].scaling[2]);
	//					}
	//				}
	//			}
	//		}
	//		else
	//		{
	//			printf("There's no Skeleton Animation\n\n");
	//		}
	//		printf("\n");
	//	}

	//	//Custom Maya Attribute
	//	printf("Print Custom Maya Attribute? Y/N:\t");
	//	std::cin >> answer;
	//	getchar();
	//	if (answer == 'Y' || answer == 'y')
	//	{
	//		printf("\n");
	//		if (mesh->counterReader.customMayaAttributeCount != 0)
	//		{
	//			for (int i = 0; i < mesh->counterReader.customMayaAttributeCount; i++)
	//			{
	//				printf("\tParticle Pivot X: %f\n", mesh->customMayaAttribute[i].particlePivot[0]);
	//				printf("\tParticle Pivot Y: %f\n", mesh->customMayaAttribute[i].particlePivot[1]);
	//				printf("\tParticle Pivot Z: %f\n", mesh->customMayaAttribute[i].particlePivot[2]);

	//				printf("\tID:               %d\n", mesh->customMayaAttribute[i].id);
	//				printf("\tHeight:           %f\n", mesh->customMayaAttribute->height);
	//				printf("\tWidth:            %f\n", mesh->customMayaAttribute->width);

	//				printf("\tParticle Pivot X: %f\n", mesh->customMayaAttribute[i].centerPivot[0]);
	//				printf("\tParticle Pivot Y: %f\n", mesh->customMayaAttribute[i].centerPivot[1]);
	//				printf("\tParticle Pivot Z: %f\n", mesh->customMayaAttribute[i].centerPivot[2]);
	//			}
	//		}
	//		else
	//		{
	//			printf("There's no Custom Attribute\n\n");
	//		}
	//	}

	//	printf("Done, press any key");
	//	getchar();
	//}

	
	return 0;
}