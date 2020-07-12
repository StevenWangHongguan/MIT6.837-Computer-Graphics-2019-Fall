#include "mesh.h"

#include "vertexrecorder.h"

using namespace std;

void Mesh::load( const char* filename )
{
	// 4.1. load() should populate bindVertices, currentVertices, and faces

	// Add your code here.
	ifstream inputFile;
	inputFile.open(filename);

	string line;
	while (getline(inputFile, line))
	{
		istringstream ss(line);
		string type;
		Vector3f vertice;
		Tuple3u face;

		ss >> type;
		if (type == "v")
		{
			ss >> vertice[0] >> vertice[1] >> vertice[2];
			bindVertices.push_back(vertice);
		}
		else if (type == "f")
		{
			ss >> face[0] >> face[1] >> face[2];
			faces.push_back(face);
		}
		
	}

	// make a copy of the bind vertices as the current vertices
	currentVertices = bindVertices;
}

void Mesh::draw()
{
	// 4.2 Since these meshes don't have normals
	// be sure to generate a normal per triangle.
	// Notice that since we have per-triangle normals
	// rather than the analytical normals from
	// assignment 1, the appearance is "faceted".
	
	//currentFacesNormals.clear(); // ���߷�����ʵʱ�����
	// �ȼ���ÿ����Ԫ�ķ���
	/*for (auto face:faces)
	{
		Vector3f faceNormal = (Vector3f::cross(currentVertices[face[1] - 1] - currentVertices[face[0] - 1],
			currentVertices[face[2] - 1] - currentVertices[face[1] - 1])).normalized();
		currentFacesNormals.push_back(faceNormal);
	}*/

	VertexRecorder rec;
	for(auto face : faces)
	{
		// ��Ч���Ǳ������ķ���
		/*Vector3f vertice1, vertice2, vertice3;
		vertice1 = currentVertices[face[0] - 1];
		vertice2 = currentVertices[face[1] - 1]; 
		vertice3 = currentVertices[face[2] - 1];

		Vector3f vector1_2 = vertice2 - vertice1;
		Vector3f vector2_3 = vertice3 - vertice2;
		Vector3f normal = (Vector3f::cross(vector1_2, vector2_3)).normalized();

		rec.record(vertice1, normal);
		rec.record(vertice2, normal);
		rec.record(vertice3, normal);*/

		// ����ķ���Ч�ʸ���һ��

		// per-face normals ����CPU�ļ���
		Vector3f normal = (Vector3f::cross(currentVertices[face[1] - 1] - currentVertices[face[0] - 1], 
			currentVertices[face[2] - 1] - currentVertices[face[1] - 1])).normalized();
		rec.record(currentVertices[face[0] - 1], normal);
		rec.record(currentVertices[face[1] - 1], normal);
		rec.record(currentVertices[face[2] - 1], normal);

		// per-vertice normals ��ʵҲ��ʹ��ÿ����Ԫ�ķ����� �����ǽ�����Ԫ�ķ�������ƽ������һ�ζ���(�����������
		//Tuple3u face = faces[i];
		//Vector3f normal;
		//if (i > 0 && i < faces.size() - 1) // ��һ�������һ����Ԫ���⴦��
		//{
		//	normal = (currentFacesNormals[i - 1] + currentFacesNormals[i] + currentFacesNormals[i + 1]) / 3;
		//}
		//else if (i == 0)
		//{
		//	normal = (currentFacesNormals[i] + currentFacesNormals[i + 1]) / 2;
		//}
		//else
		//{
		//	normal = (currentFacesNormals[i - 1] + currentFacesNormals[i]) / 2;
		//}
		//rec.record(currentVertices[face[0] - 1], normal);
		//rec.record(currentVertices[face[1] - 1], normal);
		//rec.record(currentVertices[face[2] - 1], normal);

		// vertice-shader implement SSD ����GPU�ļ���
		/*Vector3f normal = (Vector3f::cross(bindVertices[face[1] - 1] - bindVertices[face[0] - 1],
			bindVertices[face[2] - 1] - bindVertices[face[1] - 1])).normalized();
		rec.record(bindVertices[face[0] - 1], normal);
		rec.record(bindVertices[face[1] - 1], normal);
		rec.record(bindVertices[face[2] - 1], normal);*/

	}
	rec.draw();
}

void Mesh::loadAttachments( const char* filename, int numJoints )
{
	// 4.3. Implement this method to load the per-vertex attachment weights
	// this method should update m_mesh.attachments
	ifstream inputFile;
	inputFile.open(filename);

	string line;
	while (getline(inputFile, line))
	{
		istringstream ss(line);
		vector<float> attachment(numJoints - 1); // ÿһ�������ܵ���һ����Ӱ��ĳ̶� ���໥�����ӳ̶�
		for (int i = 0; i < numJoints - 1; i++)
		{
			ss >> attachment[i];
		}
		attachments.push_back(attachment);
	}

	// ��Ȩ�����ݴ��ݸ���ɫ��
}
 