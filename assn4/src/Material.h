#ifndef MATERIAL_H
#define MATERIAL_H

#include <cassert>
#include <vecmath.h>

#include "Ray.h"
#include "Image.h"
#include "Vector3f.h"

#include <string>

class Material
{
  public:
    Material(const Vector3f &diffuseColor, 
             const Vector3f &specularColor = Vector3f::ZERO, 
             float shininess = 0,
             const Vector3f &transparentColor = Vector3f::ZERO,
             float refractedIndex = 1.0) :
        _diffuseColor(diffuseColor),
        _specularColor(specularColor),
        _shininess(shininess),
        _transparentColor(transparentColor),
        _refractedIndex(refractedIndex)
    { }

    const Vector3f & getDiffuseColor() const {
        return _diffuseColor;
    }

    const Vector3f & getSpecularColor() const {
        return _specularColor;
    }

    const Vector3f& getTransparentColor() const {
        return _transparentColor;
    }
    
    const float getRefractedIndex() const {
        return _refractedIndex;
    }

    bool isSpecular() const {
        return _specularColor != Vector3f::ZERO;
    }

    bool isTransparent() const {
        return _transparentColor != Vector3f::ZERO;
    }

    Vector3f shade(const Ray &ray,
        const Hit &hit,
        const Vector3f &dirToLight,
        const Vector3f &lightIntensity);

protected:

    Vector3f _diffuseColor;
    Vector3f _specularColor;
    Vector3f _transparentColor;
    float _shininess;
    float _refractedIndex;
};

#endif // MATERIAL_H
