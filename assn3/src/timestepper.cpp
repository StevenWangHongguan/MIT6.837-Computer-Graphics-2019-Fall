#include "timestepper.h"
#include <iostream>
#include <cstdio>

void ForwardEuler::takeStep(ParticleSystem* particleSystem, float stepSize)
{
   //TODO: See handout 3.1 
	std::vector<Vector3f> oldState = particleSystem->getState();
	std::vector<Vector3f> derivatives = particleSystem->evalF(oldState);
	std::vector<Vector3f> newState;
	Vector3f temp;
	for (int i = 0; i < oldState.size(); i++)
	{
		temp = oldState[i] + stepSize * derivatives[i]; // ǰ��ŷ����
		newState.emplace_back(temp);
	}
	particleSystem->setState(newState);

}

void Trapezoidal::takeStep(ParticleSystem* particleSystem, float stepSize)
{
   //TODO: See handout 3.1 
	std::vector<Vector3f> state0 = particleSystem->getState(); // X(t)
	std::vector<Vector3f> derivatives0 = particleSystem->evalF(state0); // f0

	Vector3f temp;

	std::vector<Vector3f> state1; // X(t+h)
	for (int i = 0; i < state0.size(); i++) 
	{
		temp = state0[i] + stepSize * derivatives0[i]; // ǰ��ŷ���� ������һ��״̬
		state1.emplace_back(temp);
	}
	std::vector<Vector3f> derivatives1 = particleSystem->evalF(state1); // f1

	std::vector<Vector3f> newState;
	for (int i = 0; i < state0.size(); i++)
	{
		temp = state0[i] + stepSize * (derivatives0[i] + derivatives1[i]) / 2.0; // ǰ��ŷ����
		newState.emplace_back(temp);
	}
	particleSystem->setState(newState);
}


void RK4::takeStep(ParticleSystem* particleSystem, float stepSize)
{
	Vector3f temp;

	std::vector<Vector3f> s1 = particleSystem->getState();
	std::vector<Vector3f> k1 = particleSystem->evalF(s1);

	std::vector<Vector3f> s2;
	for (int i = 0; i < s1.size(); i++)
	{
		temp = s1[i] + stepSize / 2 * k1[i]; // ǰ��ŷ���� ������һ��״̬ ʹ��k1
		s2.emplace_back(temp);
	}
	std::vector<Vector3f> k2 = particleSystem->evalF(s2);

	std::vector<Vector3f> s3;
	for (int i = 0; i < s1.size(); i++)
	{
		temp = s1[i] + stepSize / 2 * k2[i]; // ǰ��ŷ���� ������һ��״̬ ʹ��k2
		s3.emplace_back(temp);
	}
	std::vector<Vector3f> k3 = particleSystem->evalF(s3);

	std::vector<Vector3f> s4;
	for (int i = 0; i < s1.size(); i++)
	{
		temp = s1[i] + stepSize * k3[i]; // ǰ��ŷ���� ������һ��״̬ ʹ��k3
		s4.emplace_back(temp);
	}
	std::vector<Vector3f> k4 = particleSystem->evalF(s4);

	std::vector<Vector3f> newState;
	for (int i = 0; i < s1.size(); i++)
	{
		temp = s1[i] + stepSize / 6 * (k1[i] + 2 * k2[i] + 2 * k3[i] + k4[i]); // �㷨����
		newState.emplace_back(temp);
	}

	particleSystem->setState(newState);
}

