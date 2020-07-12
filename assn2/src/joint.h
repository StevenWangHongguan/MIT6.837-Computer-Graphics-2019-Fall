#ifndef JOINT_H
#define JOINT_H

#include <vector>
#include <vecmath.h>

struct Joint
{
	Matrix4f transform; // transform relative to its parent
	std::vector< Joint* > children; // list of children

	// This matrix transforms world space into joint space for the initial ("bind") configuration of the joints.
	// 这个矩阵转换世界空间到关节空间 用来初始化绑定姿势的关节配置
	Matrix4f bindWorldToJointTransform;

	// This matrix maps joint space into world space for the *current* configuration of the joints.
	// 这个矩阵对当前的关节配置 从关节空间映射到世界空间 
	Matrix4f currentJointToWorldTransform;
};

#endif
