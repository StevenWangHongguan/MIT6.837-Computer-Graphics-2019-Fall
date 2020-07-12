#ifndef CLOTHSYSTEM_H
#define CLOTHSYSTEM_H

#include <vector>

#include "particlesystem.h"

class ClothSystem : public ParticleSystem
{
    ///ADD MORE FUNCTION AND FIELDS HERE
public:
    ClothSystem(bool hasWind);

    // evalF is called by the integrator at least once per time step
    std::vector<Vector3f> evalF(std::vector<Vector3f> state) override;

    // draw is called once per frame
    void draw(GLProgram& ctx);

    // debug helper
    void drawSpring(GLProgram& ctx);
    void drawSpring1(GLProgram& ctx);
    void drawSpring2(GLProgram& ctx);
    void drawSpring3(GLProgram& ctx);

    // suface
    void drawSurface(GLProgram& ctx);

    // calulate forces
    Vector3f calculateStructuralForces(const std::vector<Vector3f>& state, int i, int j);
    Vector3f calculateShearForces(const std::vector<Vector3f>& state, int i, int j);
    Vector3f calculateFlexionForces(const std::vector<Vector3f>& state, int i, int j);

    // calculate per-vertice normal
    Vector3f calculateVerticeNormal(const std::vector<Vector3f>& normals, int i, int j);

    // inherits
    // std::vector<Vector3f> m_vVecState;
private:
    bool _hasWind;
};


#endif
