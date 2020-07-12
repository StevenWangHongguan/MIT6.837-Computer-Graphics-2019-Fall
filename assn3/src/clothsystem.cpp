#include "clothsystem.h"
#include "camera.h"
#include "vertexrecorder.h"

 // your system should at least contain 8x8 particles.
const int W = 8;
const int H = 8;

const float UNIT_LENGTH = 0.5f; // 两相邻质点之间的距离

const Vector3f originOffset(1.5f, 2.0f, 0.0f); // 起始位置修正

// 力学相关常量
const float m = 0.03f;
const float g = 9.8f;
const float drag = 0.01f;

// 弹簧常量设定
const float struturalLen = UNIT_LENGTH;
const float shearLen = UNIT_LENGTH * float(sqrt(2.0));
const float flextionLen = 2 * struturalLen;

const float struturalStiff = 25.0f;
const float shearStiff = 10.0f;
const float flextionStiff = 2.0f;


int indexOf(int i, int j)
{
    return (i * W + j) * 2;
}

ClothSystem::ClothSystem(bool hasWind): _hasWind(hasWind)
{
    // TODO 5. Initialize m_vVecState with cloth particles. 
    // You can again use rand_uniform(lo, hi) to make things a bit more interesting
    Vector3f particlePos;
    Vector3f particleVel(0.0f);
    for (int i = 0; i < H; i++)
    {
        for (int j = 0; j < W; j++)
        {
            particlePos = { j * UNIT_LENGTH, -i * UNIT_LENGTH, 0.0f };
            particlePos += originOffset;
            m_vVecState.emplace_back(particlePos);
            m_vVecState.emplace_back(particleVel);
        }
    }
}

std::vector<Vector3f> ClothSystem::evalF(std::vector<Vector3f> state)
{
    std::vector<Vector3f> f(state.size());
    // TODO 5. implement evalF
    // - gravity
    // - viscous drag
    // - structural springs
    // - shear springs
    // - flexion springs
    Vector3f externalForces, structuralForces, shearFoces, flextionForces, F;
    for (int k = 0; k < state.size() - 1; k += 2)
    {
        f[k] = state[k + 1]; // 速度

        int index = k / 2;
        int i = index / W;
        int j = index % W;

        if (i == 0 && j == 0 || i == 0 && j == W - 1)
        {
            F = Vector3f(0.0f);
        }
        else
        {
            externalForces = m * g * Vector3f(0.0f, -1.0f, 0.0f) + -drag * state[k + 1];
            if (_hasWind)
            {
                Vector3f wind(0.0f, 0.0f, rand_uniform(-0.8f, 0.0f));
                externalForces += wind;
            }
            structuralForces = calculateStructuralForces(state, i, j);
            shearFoces = calculateShearForces(state, i, j);
            flextionForces = calculateFlexionForces(state, i, j);
            F = externalForces + structuralForces + shearFoces + flextionForces;
        }

        f[k + 1] = F / m; // 加速度
    }


    return f;
}


void ClothSystem::draw(GLProgram& gl)
{
    //TODO 5: render the system 
    //         - ie draw the particles as little spheres
    //         - or draw the springs as little lines or cylinders
    //         - or draw wireframe mesh

    const Vector3f CLOTH_COLOR(0.9f, 0.9f, 0.9f);
    gl.updateMaterial(CLOTH_COLOR);

    // EXAMPLE for how to render cloth particles.
    //  - you should replace this code.
    /*float w = 0.2f;
    Vector3f O(0.4f, 1, 0);
    gl.updateModelMatrix(Matrix4f::translation(O));
    drawSphere(0.04f, 8, 8);
    gl.updateModelMatrix(Matrix4f::translation(O + Vector3f(w, 0, 0)));
    drawSphere(0.04f, 8, 8);
    gl.updateModelMatrix(Matrix4f::translation(O + Vector3f(w, -w, 0)));
    drawSphere(0.04f, 8, 8);
    gl.updateModelMatrix(Matrix4f::translation(O + Vector3f(0, -w, 0)));
    drawSphere(0.04f, 8, 8);*/

    for (int i = 0; i < m_vVecState.size() - 1; i += 2)
    {
        gl.updateModelMatrix(Matrix4f::translation(m_vVecState[i]));
        drawSphere(0.04f, 8, 8);
    }

    drawSpring(gl);

    // EXAMPLE: This shows you how to render lines to debug the spring system.
    //
    //          You should replace this code.
    //
    //          Since lines don't have a clearly defined normal, we can't use
    //          a regular lighting model.
    //          GLprogram has a "color only" mode, where illumination
    //          is disabled, and you specify color directly as vertex attribute.
    //          Note: enableLighting/disableLighting invalidates uniforms,
    //          so you'll have to update the transformation/material parameters
    //          after a mode change.

    //gl.disableLighting();
    //gl.updateModelMatrix(Matrix4f::identity()); // update uniforms after mode change
    //VertexRecorder rec;
    //rec.record(O, CLOTH_COLOR);
    //rec.record(O + Vector3f(w, 0, 0), CLOTH_COLOR);
    //rec.record(O, CLOTH_COLOR);
    //rec.record(O + Vector3f(0, -w, 0), CLOTH_COLOR);
    //rec.record(O + Vector3f(w, 0, 0), CLOTH_COLOR);
    //rec.record(O + Vector3f(w, -w, 0), CLOTH_COLOR);
    //rec.record(O + Vector3f(0, -w, 0), CLOTH_COLOR);
    //rec.record(O + Vector3f(w, -w, 0), CLOTH_COLOR);
    //glLineWidth(3.0f);
    //rec.draw(GL_LINES);
    //gl.enableLighting(); // reset to default lighting model
    //// EXAMPLE END
}

int indexOfQuad(int i, int j) // 该方法可以通过i,j找到该点对应的右下角的四边形索引
{
    return i * (W - 1) + j;
}

void ClothSystem::drawSurface(GLProgram& gl)
{
    const Vector3f CLOTH_COLOR(0.9f, 0.9f, 0.9f);
    gl.updateMaterial(CLOTH_COLOR);

    gl.updateModelMatrix(Matrix4f::identity());

    VertexRecorder rec;
    Vector3f vertice1, vertice2, vertice3, vertice4; // 三角形三个顶点 逆时针 一个网格4个点 重复使用
    Vector3f normal1, normal2, normal3, normal4; // 一个四边形4个法线
    Vector3f normal;
    std::vector<Vector3f> normals; // 二次计算 计算每个顶点法线 平滑着色

    for (int i = 0; i < H - 1; i++) // 8x8的网格 有7x7个四边形(每个四边形有两个三角形)
    {
        for (int j = 0; j < W - 1; j++)
        {
            vertice1 = m_vVecState[indexOf(i, j)];
            vertice2 = m_vVecState[indexOf(i + 1, j)];
            vertice3 = m_vVecState[indexOf(i + 1, j + 1)];
            vertice4 = m_vVecState[indexOf(i, j + 1)];

            // 左下方三角形
            normal = Vector3f::cross(vertice2 - vertice1, vertice3 - vertice2).normalized();
            normals.emplace_back(normal);

            // 右上方三角形
            normal = Vector3f::cross(vertice4 - vertice3, vertice1 - vertice4).normalized();
            normals.emplace_back(normal);
        }
    }

    // 计算每顶点法线 顶点在四个斜对角可能有6个相邻的面 最后记得标准化
    for (int i = 0; i < H - 1; i++)
    {
        for (int j = 0; j < W - 1; j++)
        {
            vertice1 = m_vVecState[indexOf(i, j)];
            vertice2 = m_vVecState[indexOf(i + 1, j)];
            vertice3 = m_vVecState[indexOf(i + 1, j + 1)];
            vertice4 = m_vVecState[indexOf(i, j + 1)];

            normal1 = calculateVerticeNormal(normals, i, j);
            normal2 = calculateVerticeNormal(normals, i + 1, j);
            normal3 = calculateVerticeNormal(normals, i + 1, j + 1);
            normal4 = calculateVerticeNormal(normals, i, j + 1);

            rec.record(vertice1, normal1, CLOTH_COLOR);
            rec.record(vertice2, normal2, CLOTH_COLOR);
            rec.record(vertice3, normal3, CLOTH_COLOR);

            rec.record(vertice1, normal1, CLOTH_COLOR);
            rec.record(vertice3, normal3, CLOTH_COLOR);
            rec.record(vertice4, normal4, CLOTH_COLOR);
        }
    }

    rec.draw(GL_TRIANGLES);
}

void ClothSystem::drawSpring(GLProgram& gl)
{
    drawSpring1(gl);
    drawSpring2(gl);
}

// 结构弹簧
void ClothSystem::drawSpring1(GLProgram& gl)
{
    const Vector3f SPRING1_COLOR(0.9f, 0.9f, 0.9f);
    gl.updateMaterial(SPRING1_COLOR);

    gl.disableLighting();
    gl.updateModelMatrix(Matrix4f::identity());
    VertexRecorder rec;

    Vector3f currentPos, rightPos, downPos;
    for (int i = 0; i < H; i++)
    {
        for (int j = 0; j < W; j++)
        {
            currentPos = m_vVecState[indexOf(i, j)];
            if (j + 1 < W)
            {
                rightPos = m_vVecState[indexOf(i, j + 1)];
                rec.record(currentPos, SPRING1_COLOR);
                rec.record(rightPos, SPRING1_COLOR);
            }
            if (i + 1 < H)
            {
                downPos = m_vVecState[indexOf(i + 1, j)];
                rec.record(currentPos, SPRING1_COLOR);
                rec.record(downPos, SPRING1_COLOR);
            }
            glLineWidth(UNIT_LENGTH);
            rec.draw(GL_LINES);
        }
    }

    gl.enableLighting();
}
// 剪切弹簧
void ClothSystem::drawSpring2(GLProgram& gl)
{
    const Vector3f SPRING1_COLOR(0.9f, 0.9f, 0.9f);
    gl.updateMaterial(SPRING1_COLOR);

    gl.disableLighting();
    gl.updateModelMatrix(Matrix4f::identity());
    VertexRecorder rec;

    Vector3f currentPos, leftdownPos, rightdownPos;
    for (int i = 0; i < H; i++)
    {
        for (int j = 0; j < W; j++)
        {
            currentPos = m_vVecState[indexOf(i, j)];
            if (j - 1 >= 0 && i + 1 < H)
            {
                leftdownPos = m_vVecState[indexOf(i + 1, j - 1)];
                rec.record(currentPos, SPRING1_COLOR);
                rec.record(leftdownPos, SPRING1_COLOR);
            }
            if (j + 1 < W && i + 1 < H)
            {
                rightdownPos = m_vVecState[indexOf(i + 1, j + 1)];
                rec.record(currentPos, SPRING1_COLOR);
                rec.record(rightdownPos, SPRING1_COLOR);
            }
            glLineWidth(UNIT_LENGTH);
            rec.draw(GL_LINES);
        }
    }

    gl.enableLighting();
}
// 弯曲弹簧
void ClothSystem::drawSpring3(GLProgram& gl)
{
    const Vector3f SPRING1_COLOR(0.9f, 0.9f, 0.9f);
    gl.updateMaterial(SPRING1_COLOR);

    gl.disableLighting();
    gl.updateModelMatrix(Matrix4f::identity());
    VertexRecorder rec;

    Vector3f currentPos, rightPos, downPos;
    Vector3f Xoffset = { UNIT_LENGTH / 5, 0.0f, 0.0f };
    Vector3f Yoffset = { 0.0f, - UNIT_LENGTH / 5, 0.0f };
    for (int i = 0; i < H; i++)
    {
        for (int j = 0; j < W; j++)
        {
            currentPos = m_vVecState[indexOf(i, j)];
            if (j + 2 < W)
            {
                rightPos = m_vVecState[indexOf(i, j + 2)];
                rec.record(currentPos + Xoffset, SPRING1_COLOR);
                rec.record(rightPos + Xoffset, SPRING1_COLOR);
            }
            if (i + 2 < H)
            {
                downPos = m_vVecState[indexOf(i + 2, j)];
                rec.record(currentPos + Yoffset, SPRING1_COLOR);
                rec.record(downPos + Yoffset, SPRING1_COLOR);
            }
            glLineWidth(UNIT_LENGTH);
            rec.draw(GL_LINES);
        }
    }

    gl.enableLighting();
}


// 以下这些全局变量 其实应该声明为static
static Vector3f struturalForces, shearForces, FlextionForces;
static Vector3f current, up, down, left, right, upLeft, upRight, downLeft, downRight;  // 一次声明 减少每次构造Vector3f的开销

// 结构力 上下左右
Vector3f ClothSystem::calculateStructuralForces(const std::vector<Vector3f>& state, int i, int j)
{
    struturalForces = { 0.0f, 0.0f, 0.0f};

    current = state[indexOf(i, j)];
    if (i - 1 >= 0)
    {
        up = state[indexOf(i - 1, j)];
        struturalForces += -struturalStiff * ((current - up).abs() - struturalLen) * (current - up).normalized();
    }
    if (i + 1 < H)
    {
        down = state[indexOf(i + 1, j)];
        struturalForces += -struturalStiff * ((current - down).abs() - struturalLen) * (current - down).normalized();
    }
    if (j - 1 >= 0)
    {
        left = state[indexOf(i, j - 1)];
        struturalForces += -struturalStiff * ((current - left).abs() - struturalLen) * (current - left).normalized();
    }
    if (j + 1 < W)
    {
        right = state[indexOf(i, j + 1)];
        struturalForces += -struturalStiff * ((current - right).abs() - struturalLen) * (current - right).normalized();
    }

    return struturalForces;
}
// 剪切力 左上右上左下右下
Vector3f ClothSystem::calculateShearForces(const std::vector<Vector3f>& state, int i, int j)
{
    shearForces = { 0.0f, 0.0f, 0.0f };

    current = state[indexOf(i, j)];
    if (i - 1 >= 0 && j - 1 >= 0)
    {
        upLeft = state[indexOf(i - 1, j - 1)];
        shearForces += -shearStiff * ((current - upLeft).abs() - shearLen) * (current - upLeft).normalized();
    }
    if (i - 1 >= 0 && j + 1 < W)
    {
        upRight = state[indexOf(i - 1, j + 1)];
        shearForces += -shearStiff * ((current - upRight).abs() - shearLen) * (current - upRight).normalized();
    }
    if (i + 1 < H && j - 1 >= 0)
    {
        downLeft = state[indexOf(i + 1, j - 1)];
        shearForces += -shearStiff * ((current - downLeft).abs() - shearLen) * (current - downLeft).normalized();
    }
    if (i + 1 < H && j + 1 < W)
    {
        downRight = state[indexOf(i + 1, j + 1)];
        shearForces += -shearStiff * ((current - downRight).abs() - shearLen) * (current - downRight).normalized();
    }

    return shearForces;
}
// 弯曲力 上下左右
Vector3f ClothSystem::calculateFlexionForces(const std::vector<Vector3f>& state, int i, int j)
{
    FlextionForces = { 0.0f, 0.0f, 0.0f };

    current = state[indexOf(i, j)];
    if (i - 2 >= 0)
    {
        up = state[indexOf(i - 2, j)];
        FlextionForces += -flextionStiff * ((current - up).abs() - flextionLen) * (current - up).normalized();
    }
    if (i + 2 < H)
    {
        down = state[indexOf(i + 2, j)];
        FlextionForces += -flextionStiff * ((current - down).abs() - flextionLen) * (current - down).normalized();
    }
    if (j - 2 >= 0)
    {
        left = state[indexOf(i, j - 2)];
        FlextionForces += -flextionStiff * ((current - left).abs() - flextionLen) * (current - left).normalized();
    }
    if (j + 2 < W)
    {
        right = state[indexOf(i, j + 2)];
        FlextionForces += -flextionStiff * ((current - right).abs() - flextionLen) * (current - right).normalized();
    }

    return FlextionForces;
}


static Vector3f n1, n2, n3, n4, n5, n6;
static int quadIndex, normal1Index, normal2Index; // 四边形的索引 以此找到对应三角形的法线索引
Vector3f ClothSystem:: calculateVerticeNormal(const std::vector<Vector3f>& normals, int i, int j)
{
    Vector3f normal(0.0f);
    int count = 0; // 累计求和的法线数量
    // 左上角的四边形
    if (i - 1 >= 0 && j - 1 >= 0)
    {
        quadIndex = indexOfQuad(i - 1, j - 1);
        normal1Index = quadIndex * 2; // 左下方三角形
        normal2Index = quadIndex * 2 + 1; // 右上方三角形
        n1 = normals[normal1Index];
        n2 = normals[normal2Index];
        normal += n1 + n2;
        count += 2;
    }
    // 右上角的四边形
    if (i - 1 >= 0 && j + 1 < W)
    {
        quadIndex = indexOfQuad(i - 1, j);
        normal1Index = quadIndex * 2;
        n1 = normals[normal1Index];
        normal += n1;
        count++;
    }
    // 左下角的四边形
    if (i + 1 < H && j - 1 >= 0)
    {
        quadIndex = indexOfQuad(i, j - 1);
        normal2Index = quadIndex * 2 + 1;
        n2 = normals[normal2Index];
        normal += n2;
        count++;
    }
    // 右下角的四边形
    if (i + 1 < H && j + 1 < W)
    {
        quadIndex = indexOfQuad(i, j);
        normal1Index = quadIndex * 2;
        normal2Index = quadIndex * 2 + 1;
        n1 = normals[normal1Index];
        n2 = normals[normal2Index];
        normal += n1 + n2;
        count += 2;
    }
    normal /= (float)count;
    return normal.normalized();
}
