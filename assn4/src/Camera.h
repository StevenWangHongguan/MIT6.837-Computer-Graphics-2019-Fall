#ifndef CAMERA_H
#define CAMERA_H

#include "Ray.h"

#include <vecmath.h>
#include <float.h>
#include <cmath>

class Camera
{
public:
    virtual ~Camera() {}

    // Generate rays for each screen-space coordinate
    virtual Ray generateRay(const Vector2f &point) = 0;
    virtual float getTMin() const = 0;
};

/// Fill in functions and add more fields if necessary
class PerspectiveCamera : public Camera
{
public:
    PerspectiveCamera(const Vector3f &center,
        const Vector3f &direction,
        const Vector3f &up,
        float angleradians) :
        _center(center),
        _direction(direction.normalized()),
        _up(up),
        _angle(angleradians)
    {
        _horizontal = Vector3f::cross(direction, up).normalized();
        //modify
        _d = 1.0f / (float)std::tan(_angle / 2.0f); // ������Ļ��λ�ã����������Զ��
    }

    virtual Ray generateRay(const Vector2f &point) override
    {
        // BEGIN STARTER 
        // �������п����Ż� ����ÿ������һ�����߶�Ҫ����һ���������Ļ�ľ��� ���Ƕ����ˣ�
        // float d = 1.0f / (float)std::tan(_angle / 2.0f); // ������Ļ��λ�ã����������Զ��

        // d * _direction ������Ļ���ĵ�λ�� Ȼ���ٸ������ص�ĺ���ƫ���� ����������ߣ�����������ά������ӣ�
        // ǰ��û�м���_centerԭ��λ�õ�ԭ���� ���������һ���������� ���ջ��ǻ��ټ�ȥһ��_center ����û��Ҫ��
        // �൱�ڰ��������ԭ�������㷽�� �ͷ������κ�λ�ü��㷽�� �����һ����
        Vector3f newDir = _d * _direction + point[0] * _horizontal + point[1] * _up; 
        newDir = newDir.normalized();

        return Ray(_center, newDir);
        // END STARTER
    }

    virtual float getTMin() const override
    {
        return 0.0f;
    }

private:
    Vector3f _center;
    Vector3f _direction;
    Vector3f _up;
    float _angle;
    float _d;
    Vector3f _horizontal;
};

#endif //CAMERA_H
