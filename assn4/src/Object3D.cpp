#include "Object3D.h"

static float CalDeterminant
   (float a00, float a01, float a02,
    float a10, float a11, float a12,
    float a20, float a21, float a22)
{
    return a00 * a11 * a22 + a01 * a12 * a20 + a02 * a21 * a10
        - a02 * a11 * a20 - a01 * a10 * a22 - a00 * a21 * a12;
}

bool Sphere::intersect(const Ray &r, float tmin, Hit &h) const {
    // BEGIN STARTER

    // We provide sphere intersection code for you.
    // You should model other intersection implementations after this one.

    // Locate intersection point ( 2 pts )
    const Vector3f &rayOrigin = r.getOrigin(); //Ray origin in the world coordinate
    const Vector3f &dir = r.getDirection();

    Vector3f origin = rayOrigin - _center;      //Ray origin in the sphere coordinate

    float a = dir.absSquared(); // 如果光源方向是正交化过的 这个a就是1
    float b = 2 * Vector3f::dot(dir, origin);
    float c = origin.absSquared() - _radius * _radius;

    // no intersection
    if (b * b - 4 * a * c < 0) {
        return false;
    }

    float d = sqrt(b * b - 4 * a * c);

    float tplus = (-b + d) / (2.0f * a); // ＋根
    float tminus = (-b - d) / (2.0f * a); // -根

    // the two intersections are at the camera back 表示两个交点都在相机前 没有解
    if ((tplus < tmin) && (tminus < tmin)) {
        return false;
    }

    float t = 10000;
    // the two intersections are at the camera front 表示两个交点都在相机前
    if (tminus > tmin) {
        t = tminus;
    }

    // one intersection at the front. one at the back  一个交点在前一个在后 表示在球体里
    if ((tplus > tmin) && (tminus < tmin)) {
        t = tplus;
    }

    if (t < h.getT()) { // 更新hit缓存中的最近交点
        Vector3f normal = r.pointAtParameter(t) - _center; // 球体表面的法线
        normal = normal.normalized(); // 正交化
        h.set(t, this->material, normal);
        return true;
    }
    // END STARTER
    return false;
}

// Add object to group
void Group::addObject(Object3D *obj) 
{                                                                                           
    m_members.push_back(obj);
}

// Return number of objects in group
int Group::getGroupSize() const 
{         
    return (int)m_members.size();
}

bool Group::intersect(const Ray &r, float tmin, Hit &h) const {
    // BEGIN STARTER
    // we implemented this for you
    bool hit = false;
    for (Object3D *o : m_members) {
        if (o->intersect(r, tmin, h)) {
            hit = true;
        }
    }
    return hit;
    // END STARTER
}

bool Group::intersectShadowRay(const Ray& r, float tmin, Hit& h) const {
    // 不需要遍历所有的物体并找到最近交点 只要有一个物体相交 就返回true
    // 因为在每一个物体的Intersect方法实现中 都传递了一个tmin 只有大于tmin的结果 才算有交点
    // 即排除了交点在光源之后的情况
    for (Object3D* o : m_members) {
        if (o->intersect(r, tmin, h)) {
            return true;
        }
    }
    return false;
}

// old implement of Plane
//Plane::Plane(const Vector3f &normal, float d, Material *m) : Object3D(m) {
//    _d = d;
//    _normal = normal;
//    _m = m;
//}
//bool Plane::intersect(const Ray &r, float tmin, Hit &h) const {
//    // See L9 - Raycasting slides, page 49, for derivation.
//    float t = (_d - Vector3f::dot(_normal, r.getOrigin())) / Vector3f::dot(_normal, r.getDirection().normalized());
//    if (t > h.getT() || t < tmin) return false;
//    h.set(t, _m, _normal);
//    return true;
//}

bool Plane::intersect(const Ray& r, float tmin, Hit& h) const
{
    // See L9 - Raycasting slides, page 49, for derivation.
    float denominator = Vector3f::dot(_normal, r.getDirection());  // 这里为啥要正交化 方向应该是没有大小的 而且光速应该都保持一致才对 而且在构造一个ray的时候 就应该正交化
    if (denominator == 0) return false;  // 光线与平面平行 不存在交点

    float t = (_offset - Vector3f::dot(_normal, r.getOrigin())) / denominator;  // 这里的_offset等于公式里的-D，因为D值与平面偏移的方向是相反的，比如D=2，表示平面向负方向偏移2个单位，D=-2,
    if (t < tmin || t > h.getT()) return false;
    h.set(t, this->material, _normal); // 这里为啥又默认平面的法线是正交化之后的？
    return true;
}

bool Triangle::intersect(const Ray &r, float tmin, Hit &h) const {
    // See L10 - Raycasting II slides, page 16.
    /*Matrix3f A(
        _v[0] - _v[1], _v[0] - _v[2], r.getDirection()
    );
    Vector3f B = _v[0] - r.getOrigin();
    Vector3f X = A.inverse() * B;*/
    // Barycentric ratios
    /*float alpha = 1 - X[0] - X[1];
    float beta = X[0];
    float gamma = X[1];
    float t = X[2];
    if (t > h.getT() || t < tmin || alpha < 0 || beta < 0 || gamma < 0) return false;
    h.set(t, material, (alpha * _normals[0] + beta * _normals[1] + gamma * _normals[2]).normalized());
    return true;*/

    float ax = _v[0].x(), ay = _v[0].y(), az = _v[0].z();
    float bx = _v[1].x(), by = _v[1].y(), bz = _v[1].z();
    float cx = _v[2].x(), cy = _v[2].y(), cz = _v[2].z();
    
    float ox = r.getOrigin().x(), oy = r.getOrigin().y(), oz = r.getOrigin().z();
    float dx = r.getDirection().x(), dy = r.getDirection().y(), dz = r.getDirection().z();

    float A = CalDeterminant(ax - bx, ax - cx, dx, ay - by, ay - cy, dy, az - bz, az - cz, dz);
    float Beta = CalDeterminant(ax - ox, ax - cx, dx, ay - oy, ay - cy, dy, az - oz, az - cz, dz) / A;
    float Gama = CalDeterminant(ax - bx, ax - ox, dx, ay - by, ay - oy, dy, az - bz, az - oz, dz) / A;
    float t = CalDeterminant(ax - bx, ax - cx, ax - ox, ay - by, ay - cy, ay - oy, az - bz, az - cz, az - oz) / A;

    if (Beta + Gama <= 1 && Beta >= 0 && Gama >= 0 && t > tmin)
    {
        if (t < h.getT())
        {
            //Vector3f normal = (Vector3f::cross(_v[1] - _v[0], _v[2] - _v[0])).normalized();
            Vector3f normal = ((1 - Beta - Gama) * _normals[0] + Beta * _normals[1] + Gama * _normals[2]).normalized();
            h.set(t, material, normal);
            return true;
        }
    }
    return false;
}

//Transform::Transform(const Matrix4f &m,
//                     Object3D *obj) : _object(obj) {
//    M = m;
//}

//bool Transform::intersect(const Ray &r, float tmin, Hit &h) const {
//
//    //// Move ray into object coordinate space
//    //Matrix4f worldToLocal = M.inverse();
//    //Vector3f rayOriginLocal = (worldToLocal * Vector4f(r.getOrigin(), 1)).xyz();
//    //Vector3f rayDirectionLocal = (worldToLocal * Vector4f(r.getDirection(), 0)).xyz();
//    //Ray rLocal = Ray(rayOriginLocal, rayDirectionLocal);
//
//    //// Check for intersection.
//    //if(_object -> intersect(rLocal, tmin, h)) {
//    //    Vector3f normal = (worldToLocal.transposed() * Vector4f(h.getNormal().normalized(), 0)).xyz().normalized();
//    //    h.set(h.getT(), h.getMaterial(), normal);
//    //    return true;
//    //} else {
//    //    return false;
//    //}
//
//
//}

bool Transform::intersect(const Ray& r, float tmin, Hit& h) const
{
    Vector4f newRayOrigin = _M.inverse() * Vector4f(r.getOrigin(), 1.0f);
    Vector4f newRayDirection = _M.inverse() * Vector4f(r.getDirection(), 0.0f); // 方向没有正交化
    Ray newRay(newRayOrigin.xyz(), newRayDirection.xyz()); // 物体坐标空间的光线

    bool hit = _child->intersect(newRay, tmin, h);
    if (hit)
    {
        Vector4f normal = _M.inverse().transposed() * Vector4f(h.getNormal(), 0.0f);
        h.set(h.getT(), h.getMaterial(), normal.xyz().normalized());
        return true;
    }
    else 
        return false;
}
