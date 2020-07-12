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
        _d = 1.0f / (float)std::tan(_angle / 2.0f); // 计算屏幕的位置（距离相机多远）
    }

    virtual Ray generateRay(const Vector2f &point) override
    {
        // BEGIN STARTER 
        // 下面这行可以优化 否则每次生成一条光线都要计算一次相机到屏幕的距离 岂不是多余了？
        // float d = 1.0f / (float)std::tan(_angle / 2.0f); // 计算屏幕的位置（距离相机多远）

        // d * _direction 计算屏幕中心的位置 然后再根据像素点的横纵偏移量 求得这条光线（想象两个三维向量相加）
        // 前面没有加上_center原点位置的原因是 我们求的是一个方向向量 最终还是会再减去一次_center 所以没必要了
        // 相当于把相机当作原点来计算方向 和放置在任何位置计算方向 结果是一样的
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
