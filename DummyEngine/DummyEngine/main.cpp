#include "LeapImporter.h"
//int main(int argc, char** argv)
int main()
{
	
	/*LeapImporter level_importer;

	CustomLevel* level = level_importer.getLevel("test_level.ssp");

	for (int i = 0; i < level->counterReader.levelObjectCount; i++)
	{
		printf("Mesh: %d \nPositions: %f %f %f\n", i, level->levelObjects[i]->x, level->levelObjects[i]->y, level->levelObjects[i]->z);
		printf("Rotation: %f %f %f\n", level->levelObjects[i]->rotationX, level->levelObjects[i]->rotationY, level->levelObjects[i]->rotationZ);
		printf("ID: %d\n\n", level->levelObjects[i]->id);
	}

	level_importer.deleteObject(level);

	std::getchar();
	_CrtDumpMemoryLeaks();
	return 0;*/

	LeapImporter importer;
	char answer;

	LeapMesh* mesh = importer.getMesh("robot.ssp");
	printf("=======================\n");
	printf("Loaded LeapMesh\n");
	printf("=======================\n");

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
					printf("\tTexture Name: %s\n\n", mesh->material[i].textureName);
				}
			}
			else
			{
				printf("There's no Material Information\n\n");
			}
		}

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

					printf("\tID:               %d\n", mesh->customMayaAttribute[i].id);
					printf("\tHeight:           %f\n", mesh->customMayaAttribute->height);
					printf("\tWidth:            %f\n", mesh->customMayaAttribute->width);

					printf("\tCenter Pivot X: %f\n", mesh->customMayaAttribute[i].centerPivot[0]);
					printf("\tCenter Pivot Y: %f\n", mesh->customMayaAttribute[i].centerPivot[1]);
					printf("\tCenter Pivot Z: %f\n", mesh->customMayaAttribute[i].centerPivot[2]);
				}
			}
			else
			{
				printf("There's no Custom Attribute\n\n");
			}
		}

		LeapAnimation* anim = importer.getAnimation("Robot_run.sspAnim");
		printf("=======================\n");
		printf("Loaded LeapAnimation\n");
		printf("=======================\n");

		//Skeleton Animation
		printf("Print Skeleton Animation? Y/N:\t");
		std::cin >> answer;
		getchar();
		if (answer == 'Y' || answer == 'y')
		{
			
			if (anim->animation->nr_of_keyframes > 1)
			{
				printf("\n\tAnimation Name: %s\n", anim->animation->animation_name);
				printf("\tKey Frame Count: %d\n", anim->animation->nr_of_keyframes);
				printf("\tJoint Count: %d\n", anim->animation->nr_of_joints);
				//printf("\tCurrent Time: %d\n", mesh->animation->current_time);
				//printf("\tMax Time: %d\n", mesh->animation->max_time);
				std::cout << "\tCurrent Time: " << anim->animation->current_time << std::endl;
				std::cout << "\tMax Time: " << anim->animation->max_time << std::endl;
				printf("\tIs Looping: %d\n", anim->animation->looping);
				printf("\tIs Switching: %d\n", anim->animation->switching);

				printf("\nPrint Joint & Keyframes? Y/N:\t");
				std::cin >> answer;
				getchar();
				if (answer == 'Y' || answer == 'y')
				{
					for (int i = 0; i < anim->animation->nr_of_joints; i++)
					{
						printf("---------------------------------------\n");
						printf("\n\tJoint Name: %s\n", anim->animation->joints[i].joint_name);
						printf("\tParent Name: %s\n", anim->animation->joints[i].parent_name);
						printf("\tJoint ID: %d\n", anim->animation->joints[i].joint_id);
						printf("\tParent ID: %d\n", anim->animation->joints[i].parent_id);

						printf("\n\tLocal Transform Matrix:\n");
						printf("\t\t%.4f  %.4f  %.4f  %.4f\n", anim->animation->joints[i].local_transform_matrix[0][0], anim->animation->joints[i].local_transform_matrix[0][1], anim->animation->joints[i].local_transform_matrix[0][2], anim->animation->joints[i].local_transform_matrix[0][3]);
						printf("\t\t%.4f  %.4f  %.4f  %.4f\n", anim->animation->joints[i].local_transform_matrix[1][0], anim->animation->joints[i].local_transform_matrix[1][1], anim->animation->joints[i].local_transform_matrix[1][2], anim->animation->joints[i].local_transform_matrix[1][3]);
						printf("\t\t%.4f  %.4f  %.4f  %.4f\n", anim->animation->joints[i].local_transform_matrix[2][0], anim->animation->joints[i].local_transform_matrix[2][1], anim->animation->joints[i].local_transform_matrix[2][2], anim->animation->joints[i].local_transform_matrix[2][3]);
						printf("\t\t%.4f  %.4f  %.4f  %.4f\n", anim->animation->joints[i].local_transform_matrix[3][0], anim->animation->joints[i].local_transform_matrix[3][1], anim->animation->joints[i].local_transform_matrix[3][2], anim->animation->joints[i].local_transform_matrix[3][3]);

						printf("\n\tBind Pose Matrix:\n");
						printf("\t\t%.4f  %.4f  %.4f  %.4f\n", anim->animation->joints[i].bind_pose_matrix[0][0], anim->animation->joints[i].bind_pose_matrix[0][1], anim->animation->joints[i].bind_pose_matrix[0][2], anim->animation->joints[i].bind_pose_matrix[0][3]);
						printf("\t\t%.4f  %.4f  %.4f  %.4f\n", anim->animation->joints[i].bind_pose_matrix[1][0], anim->animation->joints[i].bind_pose_matrix[1][1], anim->animation->joints[i].bind_pose_matrix[1][2], anim->animation->joints[i].bind_pose_matrix[1][3]);
						printf("\t\t%.4f  %.4f  %.4f  %.4f\n", anim->animation->joints[i].bind_pose_matrix[2][0], anim->animation->joints[i].bind_pose_matrix[2][1], anim->animation->joints[i].bind_pose_matrix[2][2], anim->animation->joints[i].bind_pose_matrix[2][3]);
						printf("\t\t%.4f  %.4f  %.4f  %.4f\n", anim->animation->joints[i].bind_pose_matrix[3][0], anim->animation->joints[i].bind_pose_matrix[3][1], anim->animation->joints[i].bind_pose_matrix[3][2], anim->animation->joints[i].bind_pose_matrix[3][3]);

						printf("\n\tJoint Translation: \t%.3f %.3f %.3f\n", anim->animation->joints[i].translation[0], anim->animation->joints[i].translation[1], anim->animation->joints[i].translation[2]);
						printf("\tJoint Rotation: \t%.3f %.3f %.3f\n", anim->animation->joints[i].rotation[0], anim->animation->joints[i].rotation[1], anim->animation->joints[i].rotation[2]);
						printf("\tJoint Scale: \t%.3f %.3f %.3f\n\n", anim->animation->joints[i].scale[0], anim->animation->joints[i].scale[1], anim->animation->joints[i].scale[2]);

						for (int j = 0; j < anim->animation->nr_of_keyframes; j++)
						{
							printf("\t\tKey|%d|\tTime: %.3f", j, anim->animation->joints[i].keyFrames[j].time);
							printf("\tPosition: %.3f %.3f %.3f", anim->animation->joints[i].keyFrames[j].position[0], anim->animation->joints[i].keyFrames[j].position[1], anim->animation->joints[i].keyFrames[j].position[2]);
							printf("\tRotation: %.3f %.3f %.3f", anim->animation->joints[i].keyFrames[j].rotation[0], anim->animation->joints[i].keyFrames[j].rotation[1], anim->animation->joints[i].keyFrames[j].rotation[2]);
							printf("\tScaling: %.3f %.3f %.3f\n", anim->animation->joints[i].keyFrames[j].scaling[0], anim->animation->joints[i].keyFrames[j].scaling[1], anim->animation->joints[i].keyFrames[j].scaling[2]);
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

		printf("=======================\n");
		printf("Done, press any key");
		getchar();

	return 0;
}