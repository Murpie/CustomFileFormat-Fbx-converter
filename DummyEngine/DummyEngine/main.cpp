#include "LeapImporter.h"
//int main(int argc, char** argv)
int main()
{
	LeapImporter importer;
	LeapMesh* mesh = importer.getMesh("cubeSpin.ssp");

	char answer;
	
	//Counter
	printf("Print Counter? Y/N:\t");
	std::cin >> answer;
	getchar();
	if (answer == 'Y' || answer == 'y')
	{
		printf("\n\tVertex Count:\t\t%d\n", mesh->counterReader.vertexCount);
		printf("\tMesh Count:\t\t%d\n", mesh->counterReader.meshCount);
		printf("\tCustom Attribute Count:\t%d\n", mesh->counterReader.customMayaAttributeCount);
		printf("\tMaterial Count:\t\t%d\n\n", mesh->counterReader.matCount);
	}
	
	
	//Global Transform
	printf("Print Global Tranform? Y/N:\t");
	std::cin >> answer;
	getchar();
	if (answer == 'Y' || answer == 'y')
	{
		for (int i = 0; i < mesh->counterReader.meshCount; i++)
		{
			printf("\t%s\n", mesh[i].transform->meshName);
			printf("\tGlobal Transform:	X: %f Y: %f Z: %f\n\n", mesh[i].transform->globalTranslation[0], mesh[i].transform->globalTranslation[1], mesh[i].transform->globalTranslation[2]);
		}
	}

	//Vertex Information
	printf("Printf VertexInformation? Y/N:\t");
	std::cin >> answer;
	getchar();
	if (answer == 'Y' || answer == 'y')
	{
		if (mesh->counterReader.vertexCount != 0)
		{
			for (int i = 0; i < mesh->counterReader.vertexCount; i++)
			{
				printf("\n\t|%d|Vertex:    X: %f Y: %f Z: %f\n", i, mesh->vertices[i].x, mesh->vertices[i].y, mesh->vertices[i].z);
				printf("\t|%d|Normals:   X: %f Y: %f Z: %f\n", i, mesh->vertices[i].nx, mesh->vertices[i].ny, mesh->vertices[i].nz);
				printf("\t|%d|Binormals: X: %f Y: %f Z: %f\n", i, mesh->vertices[i].bnx, mesh->vertices[i].bny, mesh->vertices[i].bnz);
				printf("\t|%d|Tangents:  X: %f Y: %f Z: %f\n", i, mesh->vertices[i].tx, mesh->vertices[i].ty, mesh->vertices[i].tz);

				for (int j = 0; j < 4; j++)
				{
					printf("\t|%d|WeightID|%d|: %f\n", i, mesh->vertices[i].weightID[j], mesh->vertices[i].weight[j]);
				}
				printf("\n");
			}
		}
		else
		{
			printf("There's no Vertex Information\n\n");
		}
	}

	//Material Information
	printf("Print Material Information? Y/N:\t");
	std::cin >> answer;
	getchar();
	if (answer == 'Y' || answer == 'y')
	{
		if (mesh->counterReader.matCount != 0)
		{
			for (int i = 0; i < mesh->counterReader.matCount; i++)
			{
				printf("\n\tAmbient:  %.2f %.2f %.2f\n", mesh->material[i].ambient[0], mesh->material[i].ambient[1], mesh->material[i].ambient[2]);
				printf("\tDiffuse:  %.2f %.2f %.2f\n", mesh->material[i].diffuse[0], mesh->material[i].diffuse[1], mesh->material[i].diffuse[2]);
				printf("\tEmissive: %.2f %.2f %.2f\n", mesh->material[i].emissive[0], mesh->material[i].emissive[1], mesh->material[i].emissive[2]);
				printf("\tOpacity:  %.2f\n", mesh->material[i].opacity);
				printf("\tTexture File Path: %s\n\n", mesh->material[i].textureFilePath);
			}
		}
		else
		{
			printf("There's no Material Information\n\n");
		}
	}

	//Skeleton Animation
	printf("Print Skeleton Animation? Y/N:\t");
	std::cin >> answer;
	getchar();
	if (answer == 'Y' || answer == 'y')
	{
		if (mesh->animation->nr_of_keyframes > 1)
		{
			printf("\n\tAnimation Name: %s\n", mesh->animation->animation_name);
			printf("\tKey Frame Count: %d\n", mesh->animation->nr_of_keyframes);
			printf("\tJoint Count: %d\n", mesh->animation->nr_of_joints);
			//printf("\tCurrent Time: %d\n", mesh->animation->current_time);
			//printf("\tMax Time: %d\n", mesh->animation->max_time);
			std::cout << "\tCurrent Time: " << mesh->animation->current_time << std::endl;
			std::cout << "\tMax Time: " << mesh->animation->max_time << std::endl;
			printf("\tIs Looping: %d\n", mesh->animation->looping);
			printf("\tIs Switching: %d\n", mesh->animation->switching);

			printf("\nPrint Joint & Keyframes? Y/N:\t");
			std::cin >> answer;
			getchar();
			if (answer == 'Y' || answer == 'y')
			{
				for (int i = 0; i < mesh->animation->nr_of_joints; i++)
				{
					printf("---------------------------------------\n");
					printf("\n\tJoint Name: %s\n", mesh->animation->joints[i].joint_name);
					printf("\tParent Name: %s\n", mesh->animation->joints[i].parent_name);
					printf("\tJoint ID: %d\n", mesh->animation->joints[i].joint_id);
					printf("\tParent ID: %d\n", mesh->animation->joints[i].parent_id);

					printf("\n\tLocal Transform Matrix:\n");
					printf("\t\t%.4f  %.4f  %.4f  %.4f\n", mesh->animation->joints[i].local_transform_matrix[0][0], mesh->animation->joints[i].local_transform_matrix[0][1], mesh->animation->joints[i].local_transform_matrix[0][2], mesh->animation->joints[i].local_transform_matrix[0][3]);
					printf("\t\t%.4f  %.4f  %.4f  %.4f\n", mesh->animation->joints[i].local_transform_matrix[1][0], mesh->animation->joints[i].local_transform_matrix[1][1], mesh->animation->joints[i].local_transform_matrix[1][2], mesh->animation->joints[i].local_transform_matrix[1][3]);
					printf("\t\t%.4f  %.4f  %.4f  %.4f\n", mesh->animation->joints[i].local_transform_matrix[2][0], mesh->animation->joints[i].local_transform_matrix[2][1], mesh->animation->joints[i].local_transform_matrix[2][2], mesh->animation->joints[i].local_transform_matrix[2][3]);
					printf("\t\t%.4f  %.4f  %.4f  %.4f\n", mesh->animation->joints[i].local_transform_matrix[3][0], mesh->animation->joints[i].local_transform_matrix[3][1], mesh->animation->joints[i].local_transform_matrix[3][2], mesh->animation->joints[i].local_transform_matrix[3][3]);

					printf("\n\tBind Pose Matrix:\n");
					printf("\t\t%.4f  %.4f  %.4f  %.4f\n", mesh->animation->joints[i].bind_pose_matrix[0][0], mesh->animation->joints[i].bind_pose_matrix[0][1], mesh->animation->joints[i].bind_pose_matrix[0][2], mesh->animation->joints[i].bind_pose_matrix[0][3]);
					printf("\t\t%.4f  %.4f  %.4f  %.4f\n", mesh->animation->joints[i].bind_pose_matrix[1][0], mesh->animation->joints[i].bind_pose_matrix[1][1], mesh->animation->joints[i].bind_pose_matrix[1][2], mesh->animation->joints[i].bind_pose_matrix[1][3]);
					printf("\t\t%.4f  %.4f  %.4f  %.4f\n", mesh->animation->joints[i].bind_pose_matrix[2][0], mesh->animation->joints[i].bind_pose_matrix[2][1], mesh->animation->joints[i].bind_pose_matrix[2][2], mesh->animation->joints[i].bind_pose_matrix[2][3]);
					printf("\t\t%.4f  %.4f  %.4f  %.4f\n", mesh->animation->joints[i].bind_pose_matrix[3][0], mesh->animation->joints[i].bind_pose_matrix[3][1], mesh->animation->joints[i].bind_pose_matrix[3][2], mesh->animation->joints[i].bind_pose_matrix[3][3]);

					printf("\n\tJoint Translation: \t%.3f %.3f %.3f\n", mesh->animation->joints[i].translation[0], mesh->animation->joints[i].translation[1], mesh->animation->joints[i].translation[2]);
					printf("\tJoint Rotation: \t%.3f %.3f %.3f\n", mesh->animation->joints[i].rotation[0], mesh->animation->joints[i].rotation[1], mesh->animation->joints[i].rotation[2]);
					printf("\tJoint Scale: \t%.3f %.3f %.3f\n\n", mesh->animation->joints[i].scale[0], mesh->animation->joints[i].scale[1], mesh->animation->joints[i].scale[2]);

					for (int j = 0; j < mesh->animation->nr_of_keyframes; j++)
					{
						printf("\t\tKey|%d|\tTime: %.3f", j, mesh->animation->joints[i].keyFrames[j].time);
						printf("\tPosition: %.3f %.3f %.3f", mesh->animation->joints[i].keyFrames[j].position[0], mesh->animation->joints[i].keyFrames[j].position[1], mesh->animation->joints[i].keyFrames[j].position[2]);
						printf("\tRotation: %.3f %.3f %.3f", mesh->animation->joints[i].keyFrames[j].rotation[0], mesh->animation->joints[i].keyFrames[j].rotation[1], mesh->animation->joints[i].keyFrames[j].rotation[2]);
						printf("\tScaling: %.3f %.3f %.3f\n", mesh->animation->joints[i].keyFrames[j].scaling[0], mesh->animation->joints[i].keyFrames[j].scaling[1], mesh->animation->joints[i].keyFrames[j].scaling[2]);
					}
					printf("---------------------------------------\n");
				}
			}
		}
		else
		{
			printf("There's no Skeleton Animation\n\n");
		}
		printf("\n");
	}

	//Blend Shapes
	/*printf("Print BlendShapes? Y/N:\t");
	std::cin >> answer;
	getchar();
	if (answer == 'Y' || answer == 'y')
	{
		if (mesh->counterReader.blendShapeCount != 0)
		{
			printf("\n\tBlend Shape Count: %d", mesh->blendShapes->blendShapeCount);
			printf("\tKey Frame Count:     %d", mesh->blendShapes->keyFrameCount);

			for (int i = 0; i < mesh->blendShapes->blendShapeCount; i++)
			{
				printf("\tBlend Shape Vertex Count: %d", mesh->blendShapes->blendShape[i].blendShapeVertexCount);

				for (int j = 0; j < mesh->blendShapes->blendShape[i].blendShapeVertexCount; j++)
				{
					printf("\t\tBlend Shape|%d|\tVtx|%d|\tX: %.4f\tY: %.4f\tZ: %.4f\n", i, j, mesh->blendShapes->blendShape[i].blendShapeVertices[j].x, mesh->blendShapes->blendShape[i].blendShapeVertices[j].y, mesh->blendShapes->blendShape[i].blendShapeVertices[j].z);
					printf("\t\tBlend Shape|%d|\tVtx|%d|\tNX: %.4f\tNY: %.4f\tNZ: %.4f\n\n", i, j, mesh->blendShapes->blendShape[i].blendShapeVertices[j].nx, mesh->blendShapes->blendShape[i].blendShapeVertices[j].ny, mesh->blendShapes->blendShape[i].blendShapeVertices[j].nz);
				}
			}
		}
		else
		{
			printf("There's no Blend Shape\n\n");
		}
		printf("\n");
	}*/

	//Groups
	/*printf("Print Groups? Y/N:\t");
	std::cin >> answer;
	getchar();
	if (answer == 'Y' || answer == 'y')
	{
		if (mesh->group->childCount != 0)
		{
			printf("\n\tGroup Name: %s\n", mesh->group->groupName);
			printf("\tChild Count:   %d\n", mesh->group->childCount);

			for (int i = 0; i < mesh->group->childCount; i++)
			{
				printf("\n\tChild|%d| Name: %s\n", i, mesh->group->children[i].childName);
			}
		}
		else
		{
			printf("There's no Group\n\n");
		}
		printf("\n");
	}*/

	//Custom Maya Attribute
	printf("Print Custom Maya Attribute? Y/N:\t");
	std::cin >> answer;
	getchar();
	if (answer == 'Y' || answer == 'y')
	{
		printf("\n");
		if (mesh->counterReader.customMayaAttributeCount != 0)
		{
			for (int i = 0; i < mesh->counterReader.customMayaAttributeCount; i++)
			{
				printf("\tParticle Pivot X: %f\n", mesh->customMayaAttribute[i].particlePivot[0]);
				printf("\tParticle Pivot Y: %f\n", mesh->customMayaAttribute[i].particlePivot[1]);
				printf("\tParticle Pivot Z: %f\n", mesh->customMayaAttribute[i].particlePivot[2]);
			}
		}
		else
		{
			printf("There's no Custom Attribute\n\n");
		}
	}

	//Light
	/*printf("Print Lights? Y/N:\t");
	std::cin >> answer;
	getchar();
	if (answer == 'Y' || answer == 'y')
	{
		if (mesh->counterReader.lightCount)
		{
			for (int i = 0; i < mesh->counterReader.lightCount; i++)
			{
				printf("\n\tLight Type: ");
				switch (mesh->light[i].type)
				{
				case '0':
					printf("Point Light\n");
					break;
				case '1':
					printf("Directional Light\n");
					break;
				case '2':
					printf("Spotlight\n");
					break;
				default:
					printf("Unknown\n");
					break;
				}

				printf("\tColor:   R: %.2f G: %.2f B: %.2f\n", mesh->light[i].color[0], mesh->light[i].color[1], mesh->light[i].color[2]);
				printf("\tIntensity:  %.2f\n", mesh->light[i].intensity);
				printf("\tInner Cone:  %.2f\n", mesh->light[i].innerCone);
				printf("\tOuter Cone: %.2f\n", mesh->light[i].outerCone);
			}
		}
		else
		{
			printf("There's no Light\n\n");
		}
		printf("\n");
	}*/

	//Camera
	/*printf("Print Camera? Y/N:\t");
	std::cin >> answer;
	getchar();
	if (answer == 'Y' || answer == 'y')
	{
		if (mesh->counterReader.cameraCount != 0)
		{
			for (int i = 0; i < mesh->counterReader.cameraCount; i++)
			{
				printf("\n\tPosition:\tX: %.2f Y: %.2f Z: %.2f\n", mesh->camera[i].position[0], mesh->camera[i].position[1], mesh->camera[i].position[2]);
				printf("\tUp Vector:\tX: %.2f Y: %.2f Z: %.2f\n", mesh->camera[i].up[0], mesh->camera[i].up[1], mesh->camera[i].up[2]);
				printf("\tForward Vector:\tX: %.2f Y: %.2f Z: %.2f\n", mesh->camera[i].forward[0], mesh->camera[i].forward[1], mesh->camera[i].forward[2]);
				printf("\n\tRoll:\t%f\n", mesh->camera[i].roll);
				printf("\tAspect Width:\t%f\n", mesh->camera[i].aspectWidth);
				printf("\tAspect Height:\t%f\n", mesh->camera[i].aspectHeight);
				printf("\tField of View:\t%f\n", mesh->camera[i].fov);
				printf("\tNear Plane:\t%f\n", mesh->camera[i].nearPlane);
				printf("\tFar Plane:\t%f\n", mesh->camera[i].farPlane);
			}
		}
		else
		{
			printf("There's no Camera\n\n");
		}
		printf("\n");
	}*/

	printf("Done, press any key");
	getchar();
	//importer.deleteObject(level);
	return 0;
}