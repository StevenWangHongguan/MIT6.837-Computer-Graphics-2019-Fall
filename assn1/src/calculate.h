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

int calculatePermutation(int m, int n) //����Cnm��n����m���ϣ�
{
	int upNumber = factorial(n);
	int downNumber = factorial(m) * factorial(n - m);
	return upNumber / downNumber;
}

int* calculateNthTermExpansion(int n)
{
	// n�� n+1��
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
	// �����P���Ƶ㼯���Ѿ������˱պϵĿ��Ƶ�(���׼��������󼸸�����һ�µ� �ճ����㼴�ɣ�
	// һ��ѭ�� ����ÿһ�����Ƶ��ڶ���ʽ�е�Ȩ��ֵ 
	int n = int(P.size() - 1); // n+1�����Ƶ���n��
	for (int i = 0;i <= n;i++) //n+1�����Ƶ�(P.size())��Ȩ��
	{
		resultValue += constArray[i] * pow(1 - t, n - i) * pow(t, i) * P[i]; // �ۼ�ÿһ�����Ƶ��Ȩ��
	}
	return resultValue;
}
Vector3f calculateT(const vector< Vector3f >& P, float t, int* constArray) // ��Щ��������Ҫ��׼��
{
	Vector3f resultValue;
	int n = int(P.size() - 1);
	for (int i = 0;i <= n;i++)
	{
		float tPartFirstDerivative; // t���ֵ�һ�׵���

		int fixExponent1 = n - i - 1;
		int fixExponent2 = i - 1;
		// 0�ĸ��η�û������ �����t=0��t=1�ı߽�����
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