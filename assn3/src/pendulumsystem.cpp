#include "pendulumsystem.h"

#include <cassert>
#include "camera.h"
#include "vertexrecorder.h"

// TODO adjust to number of particles.
const int NUM_PARTICLES = 4;  // ��ס��һ���ʵ��ǹ̶������Ϸ��� ���������Ϊ0

const Vector3f DOWN(0.0f, -1.0f, 0.0f);

struct Spring
{
    float length;
    float stiffness;
};

std::vector<Spring> springs;

PendulumSystem::PendulumSystem()
{
    // TODO 4.2 Add particles for simple pendulum
    // TODO 4.3 Extend to multiple particles

    //Vector3f particlePos(0.0f, 1.0f, 0.0f); // ��ʱֻ��һ������
    //Vector3f particleVel(0.0f, -5.0f, 0.0f);
    //m_vVecState.emplace_back(particlePos);
    //m_vVecState.emplace_back(particleVel);

    Spring s;
    Vector3f particlePos;
    Vector3f particleVel(0.0f); // ÿ��������ʼ���Ǿ�ֹ��

    for (int i = 0; i < NUM_PARTICLES; i++)
    {
        // Vector3f xOffset = { rand_uniform(-1.0f, 1.0f), 0.0f, 0.0f };
        if (i == 0)
            particlePos = { 0.0f, 3.0f, 0.0f };
        else
            particlePos = m_vVecState[(i - 1) * 2] + springs[i - 1].length * DOWN; // +xOffset;  // ������һ���ʵ��λ�ú���һ�����ɵĳ��� ���㵱ǰ�ʵ��λ��

        m_vVecState.emplace_back(particlePos);
        m_vVecState.emplace_back(particleVel);   

        //if (i == NUM_PARTICLES - 1) break; // ���һ�����ɾͲ�������

        float springLen = rand_uniform(0.5f, 1.5f);
        float springstiff = rand_uniform(5.0f, 15.0f);
        s.length = springLen;
        s.stiffness = springstiff;
        springs.emplace_back(s);

    }
    // To add a bit of randomness, use e.g.
    // float f = rand_uniform(-0.5f, 0.5f);
    // in your initial conditions.
}


std::vector<Vector3f> PendulumSystem::evalF(std::vector<Vector3f> state)
{
    std::vector<Vector3f> f(state.size());
    // TODO 4.1: implement evalF

    ////  - gravity
    //const float m = 1.0f; // �ٶ�����Ϊ1kg
    //const float g = 9.8f; // ����g
    //const Vector3f down(0.0f, -1.0f, 0.0f);
    //const Vector3f originPos(0.0f, 2.0f, 0.0f);
    ////  - viscous drag
    //const float drag = 0.05f;
    ////  - springs
    //const float springLength = 1.0f;
    //const float stiffness = 8.0f; // ��� ��֪���Ƕ��ٰ�

    //Vector3f force1, force2, force3, F;
    //Vector3f particlePos, particleVel;
    //for (int i = 0; i < state.size() - 1; i+=2)
    //{
    //    particlePos = state[i];
    //    particleVel = state[i + 1];

    //    f[i] = particleVel; // �ٶ� ֱ��ʹ��״̬����洢��ֵ
    //    
    //    force1 = m * g * down;
    //    force2 = -drag * particleVel;
    //    force3 = -stiffness * ((particlePos - originPos).abs() - springLength) * (particlePos - originPos).normalized(); // ����Ҫ�ĳ������ʵ�֮��ĳ��Ⱥ;���
    //    F = force1 + force2 + force3;
    //    
    //    f[i + 1] = F / m; // ���ٶ�
    //}
    
    const float m = 0.5f;
    const float g = 9.8f;
    const float drag = 0.01f; // ճ������ϵ��

    Vector3f force1, force2, force3, F;
    Vector3f particlePos, particleVel;
    Vector3f preParticlePos, nextParticlePos;

    for (int i = 0; i < state.size() - 1; i+=2)
    {
        particlePos = state[i];
        particleVel = state[i + 1];

        f[i] = particleVel;

        if (i == 0) // ��һ���ʵ�ʼ�վ�ֹ
        {
            F = Vector3f(0.0f);
        }
        else
        {
            force1 = m * g * DOWN;
            force2 = -drag * particleVel;
            
            // �Ϸ����ɲ�������
            float stiffness1 = springs[i / 2 - 1].stiffness;
            float stringLen1 = springs[i / 2 - 1].length;
            preParticlePos = state[i - 2];
            force3 = -stiffness1 * ((particlePos - preParticlePos).abs() - stringLen1) * (particlePos - preParticlePos).normalized();
            // �·����ɲ�������
            if (i + 2 < state.size()) // ���ʵ����¶������е���
            {
                nextParticlePos = state[i + 2];
                float stiffness2 = springs[i / 2].stiffness;
                float stringLen2 = springs[i / 2].length;
                force3 += -stiffness2 * ((particlePos - nextParticlePos).abs() - stringLen2) * (particlePos - nextParticlePos).normalized();
            }

            F = force1 + force2 + force3;
        }

        f[i + 1] = F / m;
    }

    return f;
}

// render the system (ie draw the particles)
void PendulumSystem::draw(GLProgram& gl)
{
    const Vector3f PENDULUM_COLOR(0.73f, 0.0f, 0.83f);
    gl.updateMaterial(PENDULUM_COLOR);

    // example code. Replace with your own drawing  code
    /*gl.updateModelMatrix(Matrix4f::translation(SPRING_ORIGIN));
    drawSphere(0.075f, 10, 10);*/

    // TODO 4.2, 4.3    ����ʱ�� Ҫ��ѡ��positions������
    for (int i = 0; i < m_vVecState.size() - 1; i+=2)
    {
        gl.updateModelMatrix(Matrix4f::translation(m_vVecState[i]));
        drawSphere(0.075f, 10, 10);
    }

}
