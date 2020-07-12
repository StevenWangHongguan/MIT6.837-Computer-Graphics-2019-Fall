#ifndef JOINT_H
#define JOINT_H

#include <vector>
#include <vecmath.h>

struct Joint
{
	Matrix4f transform; // transform relative to its parent
	std::vector< Joint* > children; // list of children

	// This matrix transforms world space into joint space for the initial ("bind") configuration of the joints.
	// �������ת������ռ䵽�ؽڿռ� ������ʼ�������ƵĹؽ�����
	Matrix4f bindWorldToJointTransform;

	// This matrix maps joint space into world space for the *current* configuration of the joints.
	// �������Ե�ǰ�Ĺؽ����� �ӹؽڿռ�ӳ�䵽����ռ� 
	Matrix4f currentJointToWorldTransform;
};

#endif
