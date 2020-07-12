#ifndef OBJECT3D_H
#define OBJECT3D_H

#include "Ray.h"
#include "Material.h"

#include <string>

class Object3D {
public:
    Object3D() {
        material = NULL;
    }

    virtual ~Object3D() {}

    Object3D(Material *material) {
        this->material = material;
    }

    std::string getType() const {
        return type;
    }
    Material *getMaterial() const {
        return material;
    }

    // tmin�Ǵ�����л�ȡ�� ����Perspective��� tmin = 0 
    // Ӧ����ͨ��tmin�жϽ���λ�����ǰ��������� ��Ϊ�п�����Щ�������׶�� ��̫ǰ���λ���ǿ�������
    virtual bool intersect(const Ray &r, float tmin, Hit &h) const = 0;
    virtual bool intersectShadowRay(const Ray& r, float tmin, Hit& h) const { return false; } // ���Բ�����Ϊ���麯�� �����������඼Ҫʵ��

    std::string   type;
    Material     *material;
    bool isMesh = false; // ��ʼ�������� ���������������
    bool isTriangle = false;
    /*std::vector<Triangle> & getTriangles() {
        return std::vector<Triangle>{};
    }*/
};


class Sphere : public Object3D {
public:
    // default contstructor: unit ball at origin ��ԭ��ĵ�λС��
    Sphere() {
        _center = Vector3f(0.0, 0.0, 0.0);
        _radius = 1.0f;
    }

    Sphere(const Vector3f &center,
           float radius,
           Material *material) :
        Object3D(material), // �ò��������츸��
        _center(center),
        _radius(radius) {
    }

    virtual bool intersect(const Ray &r, float tmin, Hit &h) const override;

private:
    Vector3f _center;
    float    _radius;
};

class Group : public Object3D {
public:
    // Return true if intersection found
    virtual bool intersect(const Ray &r, float tmin, Hit &h) const override;
    virtual bool intersectShadowRay(const Ray& r, float tmin, Hit& h) const override;

    // Add object to group
    void addObject(Object3D *obj);

    // Return number of objects in group
    int getGroupSize() const;
private:
    std::vector<Object3D *> m_members;
};

// TODO: Implement Plane representing an infinite plane
// Choose your representation, add more fields and fill in the functions
class Plane : public Object3D 
{
//public:
//    Plane(const Vector3f &normal, float d, Material *m);
//
//    virtual bool intersect(const Ray &r, float tmin, Hit &h) const override;
//private:
//    float _d;
//    Vector3f _normal;
//    Material *_m;

// ƽ���ʾ�� n��P + D = 0;
public:
    Plane()
    {
       _normal = Vector3f(0.0, 0.0, 1.0);
       _offset = 0.0;
    }

    Plane(const Vector3f& normal, float offset, Material* material) :
        _normal(normal), _offset(offset), Object3D(material) {}

    virtual bool intersect(const Ray& r, float tmin, Hit& h) const override;

private:
    Vector3f _normal;
    float _offset; // ��ʽ�е�D
};


// Add more fields as necessary, but do not remove getVertex and getNormal
// as they are currently called by the Octree for optimization
class Triangle : public Object3D {
public:
    Triangle(const Vector3f &a,
             const Vector3f &b,
             const Vector3f &c,
             const Vector3f &na,
             const Vector3f &nb,
             const Vector3f &nc,
             Material *m) :
        Object3D(m) {
        _v[0] = a;
        _v[1] = b;
        _v[2] = c;
        _normals[0] = na;
        _normals[1] = nb;
        _normals[2] = nc;
        material = m;
        isTriangle = true;
    }

    virtual bool intersect(const Ray &ray, float tmin, Hit &hit) const override;

    const Vector3f &getVertex(int index) const {
        assert(index < 3);
        return _v[index];
    }

    const Vector3f &getNormal(int index) const {
        assert(index < 3);
        return _normals[index];
    }

private:
    Vector3f _v[3];
    Vector3f _normals[3];
    Material *material;
};


// TODO implement this class
// So that the intersect function first transforms the ray
// Add more fields as necessary

// �����Ŀ���� ��һ��object3D�ڵ� �����ཻʱ ��ת��һ�����ߵ����ľֲ��ռ� �ٽ��н�����㣨������Ҫ��ת��������ռ䣩
class Transform : public Object3D 
{
//public:
//    Transform(const Matrix4f &m, Object3D *obj);
//
//    virtual bool intersect(const Ray &r, float tmin, Hit &h) const override;
//
//private:
//    Object3D *_object; //un-transformed object
//    Matrix4f M;
public:
    Transform(const Matrix4f M, Object3D* child) : _M(M), _child(child) {};
    virtual bool intersect(const Ray& r, float tmin, Hit& h) const override;
private:
    Object3D* _child;
    Matrix4f _M;
};


#endif
