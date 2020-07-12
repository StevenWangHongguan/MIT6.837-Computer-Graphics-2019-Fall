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
	
	//currentFacesNormals.clear(); // 法线方向是实时计算的
	// 先计算每个面元的法线
	/*for (auto face:faces)
	{
		Vector3f faceNormal = (Vector3f::cross(currentVertices[face[1] - 1] - currentVertices[face[0] - 1],
			currentVertices[face[2] - 1] - currentVertices[face[1] - 1])).normalized();
		currentFacesNormals.push_back(faceNormal);
	}*/

	VertexRecorder rec;
	for(auto face : faces)
	{
		// 低效但是便于理解的方法
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

		// 下面的方法效率更高一点

		// per-face normals 基于CPU的计算
		Vector3f normal = (Vector3f::cross(currentVertices[face[1] - 1] - currentVertices[face[0] - 1], 
			currentVertices[face[2] - 1] - currentVertices[face[1] - 1])).normalized();
		rec.record(currentVertices[face[0] - 1], normal);
		rec.record(currentVertices[face[1] - 1], normal);
		rec.record(currentVertices[face[2] - 1], normal);

		// per-vertice normals 其实也是使用每个面元的法向量 不过是将该面元的法向量再平均化了一次而已(错误的做法）
		//Tuple3u face = faces[i];
		//Vector3f normal;
		//if (i > 0 && i < faces.size() - 1) // 第一个和最后一个面元特殊处理
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

		// vertice-shader implement SSD 基于GPU的计算
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
		vector<float> attachment(numJoints - 1); // 每一个顶点受到任一骨骼影响的程度 即相互的连接程度
		for (int i = 0; i < numJoints - 1; i++)
		{
			ss >> attachment[i];
		}
		attachments.push_back(attachment);
	}

	// 将权重数据传递给着色器
}
 