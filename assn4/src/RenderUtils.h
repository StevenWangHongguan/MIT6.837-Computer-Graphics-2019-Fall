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
		srand(static_cast <unsigned> (time(0))); // 通过调用"种子"随机数生成器scrand()可以保证每次调用生成的随机数是不一样的
	}
	static float randomFloat()
	{
		//返回精度为三位小数 0-1的数 这个方法可以指定任意精度 但是应该没有下面的写法规范
		// return (rand() % 1000 / 1000.0); 
		return static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
	}

	// 返回单位球体内的一个向量
	static Vector3f randomUnitVec3()
	{
		float a = randomFloat() * 2 - 1.0f;
		float b = randomFloat() * 2 - 1.0f;
		float c = randomFloat() * 2 - 1.0f;
		Vector3f unitVec3(a, b, c);
		return unitVec3.absSquared() <= 1.0f ? unitVec3 : unitVec3.normalized(); // 保证在单位球体内
	}

	// 通过屏幕一个像素点的位置 再细分采样
	static Vector2f sample2D(const Vector2f screenOffsets, int width, int height)
	{
		float xOffset = (randomFloat() * 2 - 1.0f) / width; // [-1,1] *  1/屏幕宽度 相当于将一个像素再横轴细分
		float yOffset = (randomFloat() * 2 - 1.0f) / height; // [-1,1] *  1/屏幕高度 相当于将一个像素再纵轴细分
		return screenOffsets + Vector2f(xOffset, yOffset);
	}

	// 通过schlick近似公式 计算反射率
	static float calReflectedRate(float cosTheta, float materialRefractedIndex)
	{
		float n1 = 1, n2 = materialRefractedIndex; // 默认为空气和其他介质 而空气的折射率近似为1
		float R0 = ((n1 - n2) * (n1 - n2)) / ((n1 + n2) * (n1 + n2));
		float R = R0 + (1 - R0) * pow(1 - cosTheta, 5);
		return R;
	}

	// 计算折射光线
	static bool hasRefractedRay(const Vector3f& rayDir, const Vector3f& normal, float relativeRefractiveIndex, Vector3f &refractedRayDir)
	{
		Vector3f I = -rayDir;
		float NdotI = Vector3f::dot(normal, I);
		float discriminant = 1 - relativeRefractiveIndex * relativeRefractiveIndex * (1 - NdotI * NdotI);
		if (discriminant > 0) // 有解 表示不是全反射
		{
			refractedRayDir = (relativeRefractiveIndex * NdotI - sqrt(discriminant)) * normal - relativeRefractiveIndex * I;
			return true;
		}
		else
			return false;
	}
};

#endif // !RENDER_UTILS_H
