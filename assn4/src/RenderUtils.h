//#pragma once
#ifndef RENDER_UTILS_H
#define RENDER_UTILS_H

#include <cmath>
#include <vecmath.h>
#include <ctime>
#include <cstdlib>

class RenderUtils
{
public:
	static void initRandomSeed()
	{
		srand(static_cast <unsigned> (time(0))); // ͨ������"����"�����������scrand()���Ա�֤ÿ�ε������ɵ�������ǲ�һ����
	}
	static float randomFloat()
	{
		//���ؾ���Ϊ��λС�� 0-1���� �����������ָ�����⾫�� ����Ӧ��û�������д���淶
		// return (rand() % 1000 / 1000.0); 
		return static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
	}

	// ���ص�λ�����ڵ�һ������
	static Vector3f randomUnitVec3()
	{
		float a = randomFloat() * 2 - 1.0f;
		float b = randomFloat() * 2 - 1.0f;
		float c = randomFloat() * 2 - 1.0f;
		Vector3f unitVec3(a, b, c);
		return unitVec3.absSquared() <= 1.0f ? unitVec3 : unitVec3.normalized(); // ��֤�ڵ�λ������
	}

	// ͨ����Ļһ�����ص��λ�� ��ϸ�ֲ���
	static Vector2f sample2D(const Vector2f screenOffsets, int width, int height)
	{
		float xOffset = (randomFloat() * 2 - 1.0f) / width; // [-1,1] *  1/��Ļ��� �൱�ڽ�һ�������ٺ���ϸ��
		float yOffset = (randomFloat() * 2 - 1.0f) / height; // [-1,1] *  1/��Ļ�߶� �൱�ڽ�һ������������ϸ��
		return screenOffsets + Vector2f(xOffset, yOffset);
	}

	// ͨ��schlick���ƹ�ʽ ���㷴����
	static float calReflectedRate(float cosTheta, float materialRefractedIndex)
	{
		float n1 = 1, n2 = materialRefractedIndex; // Ĭ��Ϊ�������������� �������������ʽ���Ϊ1
		float R0 = ((n1 - n2) * (n1 - n2)) / ((n1 + n2) * (n1 + n2));
		float R = R0 + (1 - R0) * pow(1 - cosTheta, 5);
		return R;
	}

	// �����������
	static bool hasRefractedRay(const Vector3f& rayDir, const Vector3f& normal, float relativeRefractiveIndex, Vector3f &refractedRayDir)
	{
		Vector3f I = -rayDir;
		float NdotI = Vector3f::dot(normal, I);
		float discriminant = 1 - relativeRefractiveIndex * relativeRefractiveIndex * (1 - NdotI * NdotI);
		if (discriminant > 0) // �н� ��ʾ����ȫ����
		{
			refractedRayDir = (relativeRefractiveIndex * NdotI - sqrt(discriminant)) * normal - relativeRefractiveIndex * I;
			return true;
		}
		else
			return false;
	}
};

#endif // !RENDER_UTILS_H
