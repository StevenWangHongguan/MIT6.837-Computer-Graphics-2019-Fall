#ifndef RENDERER_H
#define RENDERER_H

#include <string>

#include "SceneParser.h"
#include "ArgParser.h"

class Hit;
class Vector3f;
class Ray;

class Renderer
{
  public:
    // Instantiates a renderer for the given scene.
    Renderer(const ArgParser &args);
    void Render();
  private:
    Vector3f traceRay(const Ray &ray, float tmin, int bounces, 
                      Hit &hit) const;
    Vector3f reflectRay(const Vector3f& specularColor, const Vector3f& rayDir, const Vector3f& normal, 
                        const Vector3f& intersectPoint, int bounces) const;
    Vector3f refractRay(const Vector3f& specularColor, const Vector3f& transparentColor, const Vector3f& rayDir, const Vector3f& normal, 
                        const Vector3f& intersectPoint, float relativeRefractiveIndex, int bounces) const;

    ArgParser _args;
    SceneParser _scene;
};

#endif // RENDERER_H
