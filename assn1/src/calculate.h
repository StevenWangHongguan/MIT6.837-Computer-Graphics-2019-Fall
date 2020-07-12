#ifndef CALCULATE_H
#define CALCULATE_H

#include <vecmath.h>
#include <vector>
#include <math.h>
#include "starter1_util.h"
using namespace std;

int factorial(int n)
{
	int number = 1;
	for (int i = 1;i <= n;i++)
	{
		number *= i;
	}
	return number;
}

int calculatePermutation(int m, int n) //计算Cnm（n在下m在上）
{
	int upNumber = factorial(n);
	int downNumber = factorial(m) * factorial(n - m);
	return upNumber / downNumber;
}

int* calculateNthTermExpansion(int n)
{
	// n阶 n+1项
	int length = n + 1;
	int* array = new int[length]();
	for (int i = 0;i <= n;i++)
	{
		array[i] = calculatePermutation(i, n);
	}
	for (int i = n / 2 + 1;i <= n;i++)
	{
		array[i] = array[n - i];
	}

	return array;
}

Vector3f calculateV(const vector< Vector3f >& P, float t, int* constArray)
{
	Vector3f resultValue;
	// 输入的P控制点集合已经包含了闭合的控制点(即首几个点和最后几个点是一致的 照常计算即可）
	// 一个循环 计算每一个控制点在多项式中的权重值 
	int n = int(P.size() - 1); // n+1个控制点是n阶
	for (int i = 0;i <= n;i++) //n+1个控制点(P.size())的权重
	{
		resultValue += constArray[i] * pow(1 - t, n - i) * pow(t, i) * P[i]; // 累计每一个控制点的权重
	}
	return resultValue;
}
Vector3f calculateT(const vector< Vector3f >& P, float t, int* constArray) // 这些向量都需要标准化
{
	Vector3f resultValue;
	int n = int(P.size() - 1);
	for (int i = 0;i <= n;i++)
	{
		float tPartFirstDerivative; // t部分的一阶导数

		int fixExponent1 = n - i - 1;
		int fixExponent2 = i - 1;
		// 0的负次方没有意义 这个在t=0和t=1的边界会出错
		if (1 - t == 0) 
			fixExponent1 = (n - i - 1) >= 0 ? (n - i - 1) : 0;
		if (t == 0)
			fixExponent2 = (i - 1) >= 0 ? (i - 1) : 0;

		tPartFirstDerivative = (-1) * (n - i) * pow(1 - t, fixExponent1) * pow(t, i) + i * pow(t, fixExponent2) * pow(1 - t, n - i);
		resultValue += constArray[i] * tPartFirstDerivative * P[i];
	}
	return resultValue.normalized();
}
Vector3f calculateN(const Vector3f& preB, const Vector3f& T)
{
	Vector3f resultValue;
	resultValue = Vector3f::cross(preB, T);
	return resultValue.normalized();
}
Vector3f calculateB(const Vector3f& T, const Vector3f& N)
{
	
	Vector3f resultValue;
	resultValue = Vector3f::cross(T, N);
	return resultValue.normalized();
}

#endif // !CALCULATE_H