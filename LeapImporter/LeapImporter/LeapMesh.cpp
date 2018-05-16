#include "LeapMesh.h"

LeapMesh::LeapMesh()
{

	//transform = new MeshInfo();
	//material = new MaterialInformation();
	//joints = new JointInformation();
	//animation = new AnimationInformation();
	//keyFrame = new KeyFrame();
	//keyFrameData = new KeyFrameData();
	//blendShape = new BlendShape();
	//blendShapeVertices = new BlendShapeVertex();
	//group = new Group();
	//customMayaAttribute = new CustomMayaAttributes();
	//customMayaAttribute->meshType = 0;
}

LeapMesh::LeapMesh(const char* fileName)
{
	loader(fileName);
}

LeapMesh::~LeapMesh()
{
	for (BoundingBox* bbox_ptr : boundingBoxes)
	{
		delete bbox_ptr;
	}
	boundingBoxes.clear();

	//delete customMayaAttribute;
}

void LeapMesh::loader(const char* fileName)
{
	
	std::ifstream infile(fileName, std::ifstream::binary);

	infile.read((char*)&counterReader, sizeof(Counter));
	
	vertices = new VertexInformation[counterReader.vertexCount];

	infile.read((char*)vertices, counterReader.vertexCount * sizeof(VertexInformation));

	for (int i = 0; i < counterReader.boundingBoxCount; i++)
	{
		BoundingBox* bbox = new BoundingBox();
		boundingBoxes.push_back(bbox);
		infile.read((char*)boundingBoxes[i], sizeof(BoundingBox));
	}

	customMayaAttribute = new CustomMayaAttributes;

	infile.read((char*)&customMayaAttribute, sizeof(CustomMayaAttributes));

	if (infile.is_open())
	{
		infile.close();
	}

	return;
}
