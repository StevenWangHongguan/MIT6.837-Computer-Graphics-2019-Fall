#include "skeletalmodel.h"
#include <cassert>

#include "starter2_util.h"
#include "vertexrecorder.h"

using namespace std;

void PrintVec3(const Vector3f& vec3)
{
    cout << vec3.x() << "," << vec3.y() << "," << vec3.z() << endl;
}

SkeletalModel::SkeletalModel() {
    program = compileProgram(c_vertexshader, c_fragmentshader_light);
    if (!program) {
        printf("Cannot compile program\n");
        assert(false);
    }
}

SkeletalModel::~SkeletalModel() {
    // destructor will release memory when SkeletalModel is deleted
    while (m_joints.size()) {
        delete m_joints.back();
        m_joints.pop_back();
    }

    glDeleteProgram(program);
}

void SkeletalModel::load(const char *skeletonFile, const char *meshFile, const char *attachmentsFile)
{
    loadSkeleton(skeletonFile);
    m_mesh.load(meshFile);
    m_mesh.loadAttachments(attachmentsFile, (int)m_joints.size());

    computeBindWorldToJointTransforms(); // 只需要调用一次 因为绑定姿势只有一个
    updateCurrentJointToWorldTransforms(); // 需要调用多次 每次对骨骼进行旋转的时候 都会更新当前的变换矩阵
}

void SkeletalModel::draw(const Camera& camera, bool skeletonVisible)
{
    // draw() gets called whenever a redraw is required
    // (after an update() occurs, when the camera moves, the window is resized, etc)

    m_matrixStack.clear();

    glUseProgram(program);
    updateShadingUniforms();
    //drawCube(10.0f);
    if (skeletonVisible)
    {
        drawJoints(camera);
        drawSkeleton(camera);
    }
    else
    {
        // Tell the mesh to draw itself.
        // Since we transform mesh vertices on the CPU,
        // There is no need to set a Model matrix as uniform
        camera.SetUniforms(program, Matrix4f::identity());
        m_mesh.draw();
    }
    glUseProgram(0);
}

void SkeletalModel::updateShadingUniforms() {
    // UPDATE MATERIAL UNIFORMS 更新材质的uniform
    GLfloat diffColor[] = { 0.4f, 0.4f, 0.4f, 1 };
    GLfloat specColor[] = { 0.9f, 0.9f, 0.9f, 1 };
    GLfloat shininess[] = { 50.0f };
    int loc = glGetUniformLocation(program, "diffColor");
    glUniform4fv(loc, 1, diffColor);
    loc = glGetUniformLocation(program, "specColor");
    glUniform4fv(loc, 1, specColor);
    loc = glGetUniformLocation(program, "shininess");
    glUniform1f(loc, shininess[0]);

    // UPDATE LIGHT UNIFORMS 更新光照的uniform
    GLfloat lightPos[] = { 3.0f, 3.0f, 5.0f, 1.0f };
    loc = glGetUniformLocation(program, "lightPos");
    glUniform4fv(loc, 1, lightPos);

    GLfloat lightDiff[] = { 120.0f, 120.0f, 120.0f, 1.0f };
    loc = glGetUniformLocation(program, "lightDiff");
    glUniform4fv(loc, 1, lightDiff);
}

void SkeletalModel::loadSkeleton(const char* filename)
{
    // Load the skeleton from file here.
    ifstream inputFile;
    inputFile.open(filename);
    
    string line;
    while (getline(inputFile, line))
    {
        istringstream ss(line); // 从一行文本中获取每个字符串
        Vector3f translation;
        ss >> translation[0] >> translation[1] >> translation[2];
        int parentIndex;
        ss >> parentIndex;

        Joint* newJoint = new Joint();
        newJoint->transform = Matrix4f::translation(translation);

        m_joints.push_back(newJoint); // 顺序加入vector 所以vector中的索引下标 就是第几个节点
        if (parentIndex == -1)
            m_rootJoint = newJoint;
        else
            m_joints[parentIndex]->children.push_back(newJoint);
    }
}

void SkeletalModel::drawJoints(const Camera& camera)
{
    // Draw a sphere at each joint. You will need to add a recursive
    // helper function to traverse the joint hierarchy.
    // 为每个关节绘制一个球体 需要增加一个递归的辅助方法来遍历关节层级
    // We recommend using drawSphere( 0.025f, 12, 12 )
    // to draw a sphere of reasonable size.
    //
    // You should use your MatrixStack class. A function
    // should push it's changes onto the stack, and
    // use stack.pop() to revert the stack to the original
    // state.

    // 见作业总结的2.2
    traverseJoints(camera, m_rootJoint);

    // this is just for illustration: 下面部分只是用于演示
    // translate from top of stack, but doesn't push, since that's not
    // implemented yet.
    // Matrix4f M = m_matrixStack.top() * Matrix4f::translation(+0.5f, +0.5f, -0.5f);
    // update transformation uniforms
    // camera.SetUniforms(program, M);
    // draw
    // drawSphere(0.025f, 12, 12);
    // didn't push to stack, so no pop() needed
}

void SkeletalModel::traverseJoints(const Camera& camera, Joint* currentJoint)
{
    m_matrixStack.push(currentJoint->transform);
    //Matrix4f M = m_matrixStack.top(); 提高效率 直接用top
    camera.SetUniforms(program, m_matrixStack.top());
    drawSphere(0.025f, 12, 12);

    for (auto joint:currentJoint->children)
    {
        traverseJoints(camera, joint);
    }

    m_matrixStack.pop();
}

void SkeletalModel::drawSkeleton(const Camera& camera)
{
    // Draw cylinders between the joints. You will need to add a recursive 
    // helper function to traverse the joint hierarchy.
    // 绘制两个关节之间的骨骼 使用cylinder表示
    // We recommend using drawCylinder(6, 0.02f, <height>);
    // to draw a cylinder of reasonable diameter.

    // you can use the stack with push/pop like this
    /*m_matrixStack.push(Matrix4f::translation(+0.6f, +0.5f, -0.5f));
    camera.SetUniforms(program, m_matrixStack.top());
    drawCylinder(6, 0.02f, 0.2f);*/
    // callChildFunction();
    // m_matrixStack.pop();

    // 需要通过父节点和子节点之间的距离来决定cylinder的长度
    // 并且还要设置好旋转的角度 通过translation来设置旋转角度 
    // 其实就是要获取这个translation的方向向量 用于旋转
    traverseSkeleton(camera, m_rootJoint);
}

void SkeletalModel::traverseSkeleton(const Camera& camera, Joint* currentJoint)
{
    m_matrixStack.push(currentJoint->transform); // 层级节点的转换 存储当前转换的状态
    //Matrix4f M = m_matrixStack.top();

    for (auto joint : currentJoint->children) // 绘制从父节点到各个子节点之间的骨骼
    {
        Vector3f translation = (joint->transform).getCol(3).xyz();
        float length = translation.abs();

        // M矩阵的旋转处理
        Vector3f cylinderInitalDirection = Vector3f(0.0f, 1.0f, 0.0f); // 圆柱体初始方向竖直向上
        Vector3f rotateDiretion = Vector3f::cross(cylinderInitalDirection, translation); // 旋转轴
        // 记住点乘公式 正交化之后模为1 --- a·b = |a|·|b|·cosθ = cosθ
        float cosRotateRadians = Vector3f::dot(cylinderInitalDirection.normalized(), translation.normalized());
        float rotateRadians = acos(cosRotateRadians);
        Matrix4f rotateMatrix = m_matrixStack.top() * Matrix4f::rotation(rotateDiretion, rotateRadians); // M * ...

        camera.SetUniforms(program, rotateMatrix);
        drawCylinder(6, 0.02f, length);
    }

    for (auto joint : currentJoint->children) // 递归绘制每一个子节点下的骨骼
    {
        traverseSkeleton(camera, joint);
    }

    m_matrixStack.pop();
}

void SkeletalModel::setJointTransform(int jointIndex, float rX, float rY, float rZ)
{
    // Set the rotation part of the joint's transformation matrix based on the passed in Euler angles.
    Matrix4f rotateMatrix = Matrix4f::rotateX((rX)) * Matrix4f::rotateY((rY)) * Matrix4f::rotateZ((rZ));
    m_joints[jointIndex]->transform.setSubmatrix3x3(0, 0, rotateMatrix.getSubmatrix3x3(0, 0));

    // Matrix4f originalTransform = m_joints[jointIndex]->transform;
    // m_joints[jointIndex]->transform = originalTransform * rotateMatrix;
    // 直接右乘结果不对的原因是 正确的变换是先平移再旋转（在平移之后的局部坐标系下做旋转）
    // 如果右乘 相当于最后是先旋转了在平移；左乘更不对了，因为当前的变换需要累计前面父节点的平移变换矩阵 中间突然插上一个旋转 就有问题了
    // 直接修改旋转部分的分量才是正确的 表示在当前父节点下 先平移再旋转 应该可以理解为 把这个旋转矩阵放到累积的平移矩阵的最左边
}

void SkeletalModel::computeBindWorldToJointTransforms()
{
    // 2.3.1. Implement this method to compute a per-joint transform from
    // world-space to joint space in the BIND POSE.    
    // 计算绑定姿势中每一个关节的 从世界空间到关节空间的转换
    // 
    // Note that this needs to be computed only once since there is only
    // a single bind pose.
    // 
    // This method should update each joint's bindWorldToJointTransform.
    // You will need to add a recursive helper function to traverse the joint hierarchy.

    //m_matrixStack.clear(); // 似乎不用也行
    traverseForBindWorldToJointTransforms(m_rootJoint);

    // 传递bindWorldToJointTransform给顶点着色器（一个列表）
   /* for (int i = 0; i < m_joints.size(); i++)
    {
        string uniformName = "bindWorldToJointTransforms[" + to_string(i) + "]";
        int loc = glGetUniformLocation(program, uniformName.c_str());
        glUniformMatrix4fv(loc, 1, false, m_joints[i]->bindWorldToJointTransform);
    }*/
}

void SkeletalModel::traverseForBindWorldToJointTransforms(Joint* currentJoint)
{
    m_matrixStack.push(currentJoint->transform);
    currentJoint->bindWorldToJointTransform = m_matrixStack.top().inverse();

    for (auto joint : currentJoint->children)
    {
        traverseForBindWorldToJointTransforms(joint);
    }

    m_matrixStack.pop();
}

void SkeletalModel::updateCurrentJointToWorldTransforms()
{
    // 2.3.2. Implement this method to compute a per-joint transform from
    // joint space to world space in the CURRENT POSE.
    //
    // The current pose is defined by the rotations you've applied to the
    // joints and hence needs to be *updated* every time the joint angles change.
    //
    // This method should update each joint's currentJointToWorldTransform.
    // You will need to add a recursive helper function to traverse the joint hierarchy.

    //m_matrixStack.clear();
    traverseForCurrentJointToWorldTransforms(m_rootJoint);

    //传递currentJointToWorldTransform给顶点着色器（一个列表）
   /* for (int i = 0; i < m_joints.size(); i++)
    {
        string uniformName = "currentJointToWorldTransforms[" + to_string(i) + "]";
        int loc = glGetUniformLocation(program, uniformName.c_str());
        glUniformMatrix4fv(loc, 1, false, m_joints[i]->currentJointToWorldTransform);
    }*/
}
void SkeletalModel::traverseForCurrentJointToWorldTransforms(Joint* currentJoint)
{
    m_matrixStack.push(currentJoint->transform);
    currentJoint->currentJointToWorldTransform = m_matrixStack.top();

    for (auto joint : currentJoint->children)
    {
        traverseForCurrentJointToWorldTransforms(joint);
    }

    m_matrixStack.pop();
}

void SkeletalModel::updateMesh()
{
    // 2.3.2. This is the core of SSD.
    // Implement this method to update the vertices of the mesh
    // given the current state of the skeleton.
    // You will need both the bind pose world --> joint transforms.
    // and the current joint --> world transforms.

    // 通过绑定姿势 来计算当前姿势上的顶点位置(此处由于是通过顶点之间的变的叉乘来计算法线，所以不需要进行一个法线的变换）
    // 用到的数据有 Mesh中的currentVertices和attachments 权重当中每个顶点索引的关节下标要+1 因为根节点已经被排除了
    // 每个关节（m_joints)的bindWorldToJointTransform和currentJointToWorldTransform
    // 见作业总结4.4的公式

    // CPU上的实现
    m_mesh.currentVertices.clear();
    for (int i = 0; i < m_mesh.bindVertices.size(); i++)
    {
        // 当前顶点为m_mesh.bindVertices[i]（位置，vector3f)
        Vector3f verticePos;
        //vector<float> attachment = m_mesh.attachments[i]; // 顶点i的权重列表 优化
        for (int j = 0; j < m_mesh.attachments[i].size(); j++)
        {
            if (m_mesh.attachments[i][j] == 0)
                continue;

            // 权重列表索引j对应的关节是j+1
            verticePos += (m_mesh.attachments[i][j] * m_joints[j + 1]->currentJointToWorldTransform *
                m_joints[j + 1]->bindWorldToJointTransform * Vector4f(m_mesh.bindVertices[i], 1.0f)
                ).xyz();
        }
        m_mesh.currentVertices.emplace_back(verticePos);
    }

    // 这部分改成传递Uniforms 在GPU上计算
    // 传递对应顶点的attachment


}