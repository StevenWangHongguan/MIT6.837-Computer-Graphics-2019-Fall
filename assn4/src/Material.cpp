#include "Material.h"

//float clamp(const Vector3f &L, const Vector3f &N)
//{
//    float dotProd = Vector3f::dot(L, N);
//    return dotProd > 0 ? dotProd : 0;
//}

float clamp(const Vector3f& L, const Vector3f& N) // 光线方向在表面法线上的投影
{
    return Vector3f::dot(L, N) > 0 ? Vector3f::dot(L, N) : 0;
}

Vector3f Material::shade(const Ray &ray,
                         const Hit &hit,
                         const Vector3f &dirToLight,
                         const Vector3f &lightIntensity) 
{
    // Diffuse term
    Vector3f Idiffuse = clamp(dirToLight, hit.getNormal()) * lightIntensity * _diffuseColor;
    // Specular term
    // See L13 slides for perfect reflection vector formula.
    Vector3f R = ray.getDirection() - 2 * (Vector3f::dot(ray.getDirection(), hit.getNormal())) * hit.getNormal();
    Vector3f Ispecular = pow(clamp(dirToLight, R), _shininess) * lightIntensity * _specularColor;
    return Idiffuse + Ispecular;


    /*Vector3f L = dirToLight.normalized();
    Vector3f N = hit.getNormal().normalized();*/
    //// Diffuse term
    //float diffuseShading = clamp(L, N);
    //Vector3f IDiffuse = diffuseShading * lightIntensity * _diffuseColor;
    //// Specular term
    //Vector3f E = -ray.getDirection().normalized();
    // //See L13 slides for perfect reflection vector formula.
    //Vector3f R = (-E + (2 * (Vector3f::dot(E, N)) * N)).normalized();
    //Vector3f ISpecular = pow(clamp(L, R), _shininess) * lightIntensity * _specularColor;
    //return IDiffuse + ISpecular;
}
