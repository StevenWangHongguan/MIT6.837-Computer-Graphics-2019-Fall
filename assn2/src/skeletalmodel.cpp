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

    computeBindWorldToJointTransforms(); // ֻ��Ҫ����һ�� ��Ϊ������ֻ��һ��
    updateCurrentJointToWorldTransforms(); // ��Ҫ���ö�� ÿ�ζԹ���������ת��ʱ�� ������µ�ǰ�ı任����
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
    // UPDATE MATERIAL UNIFORMS ���²��ʵ�uniform
    GLfloat diffColor[] = { 0.4f, 0.4f, 0.4f, 1 };
    GLfloat specColor[] = { 0.9f, 0.9f, 0.9f, 1 };
    GLfloat shininess[] = { 50.0f };
    int loc = glGetUniformLocation(program, "diffColor");
    glUniform4fv(loc, 1, diffColor);
    loc = glGetUniformLocation(program, "specColor");
    glUniform4fv(loc, 1, specColor);
    loc = glGetUniformLocation(program, "shininess");
    glUniform1f(loc, shininess[0]);

    // UPDATE LIGHT UNIFORMS ���¹��յ�uniform
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
        istringstream ss(line); // ��һ���ı��л�ȡÿ���ַ���
        Vector3f translation;
        ss >> translation[0] >> translation[1] >> translation[2];
        int parentIndex;
        ss >> parentIndex;

        Joint* newJoint = new Joint();
        newJoint->transform = Matrix4f::translation(translation);

        m_joints.push_back(newJoint); // ˳�����vector ����vector�е������±� ���ǵڼ����ڵ�
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
    // Ϊÿ���ؽڻ���һ������ ��Ҫ����һ���ݹ�ĸ��������������ؽڲ㼶
    // We recommend using drawSphere( 0.025f, 12, 12 )
    // to draw a sphere of reasonable size.
    //
    // You should use your MatrixStack class. A function
    // should push it's changes onto the stack, and
    // use stack.pop() to revert the stack to the original
    // state.

    // ����ҵ�ܽ��2.2
    traverseJoints(camera, m_rootJoint);

    // this is just for illustration: ���沿��ֻ��������ʾ
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
    //Matrix4f M = m_matrixStack.top(); ���Ч�� ֱ����top
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
    // ���������ؽ�֮��Ĺ��� ʹ��cylinder��ʾ
    // We recommend using drawCylinder(6, 0.02f, <height>);
    // to draw a cylinder of reasonable diameter.

    // you can use the stack with push/pop like this
    /*m_matrixStack.push(Matrix4f::translation(+0.6f, +0.5f, -0.5f));
    camera.SetUniforms(program, m_matrixStack.top());
    drawCylinder(6, 0.02f, 0.2f);*/
    // callChildFunction();
    // m_matrixStack.pop();

    // ��Ҫͨ�����ڵ���ӽڵ�֮��ľ���������cylinder�ĳ���
    // ���һ�Ҫ���ú���ת�ĽǶ� ͨ��translation��������ת�Ƕ� 
    // ��ʵ����Ҫ��ȡ���translation�ķ������� ������ת
    traverseSkeleton(camera, m_rootJoint);
}

void SkeletalModel::traverseSkeleton(const Camera& camera, Joint* currentJoint)
{
    m_matrixStack.push(currentJoint->transform); // �㼶�ڵ��ת�� �洢��ǰת����״̬
    //Matrix4f M = m_matrixStack.top();

    for (auto joint : currentJoint->children) // ���ƴӸ��ڵ㵽�����ӽڵ�֮��Ĺ���
    {
        Vector3f translation = (joint->transform).getCol(3).xyz();
        float length = translation.abs();

        // M�������ת����
        Vector3f cylinderInitalDirection = Vector3f(0.0f, 1.0f, 0.0f); // Բ�����ʼ������ֱ����
        Vector3f rotateDiretion = Vector3f::cross(cylinderInitalDirection, translation); // ��ת��
        // ��ס��˹�ʽ ������֮��ģΪ1 --- a��b = |a|��|b|��cos�� = cos��
        float cosRotateRadians = Vector3f::dot(cylinderInitalDirection.normalized(), translation.normalized());
        float rotateRadians = acos(cosRotateRadians);
        Matrix4f rotateMatrix = m_matrixStack.top() * Matrix4f::rotation(rotateDiretion, rotateRadians); // M * ...

        camera.SetUniforms(program, rotateMatrix);
        drawCylinder(6, 0.02f, length);
    }

    for (auto joint : currentJoint->children) // �ݹ����ÿһ���ӽڵ��µĹ���
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
    // ֱ���ҳ˽�����Ե�ԭ���� ��ȷ�ı任����ƽ������ת����ƽ��֮��ľֲ�����ϵ������ת��
    // ����ҳ� �൱�����������ת����ƽ�ƣ���˸������ˣ���Ϊ��ǰ�ı任��Ҫ�ۼ�ǰ�游�ڵ��ƽ�Ʊ任���� �м�ͻȻ����һ����ת ����������
    // ֱ���޸���ת���ֵķ���������ȷ�� ��ʾ�ڵ�ǰ���ڵ��� ��ƽ������ת Ӧ�ÿ������Ϊ �������ת����ŵ��ۻ���ƽ�ƾ���������
}

void SkeletalModel::computeBindWorldToJointTransforms()
{
    // 2.3.1. Implement this method to compute a per-joint transform from
    // world-space to joint space in the BIND POSE.    
    // �����������ÿһ���ؽڵ� ������ռ䵽�ؽڿռ��ת��
    // 
    // Note that this needs to be computed only once since there is only
    // a single bind pose.
    // 
    // This method should update each joint's bindWorldToJointTransform.
    // You will need to add a recursive helper function to traverse the joint hierarchy.

    //m_matrixStack.clear(); // �ƺ�����Ҳ��
    traverseForBindWorldToJointTransforms(m_rootJoint);

    // ����bindWorldToJointTransform��������ɫ����һ���б�
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

    //����currentJointToWorldTransform��������ɫ����һ���б�
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

    // ͨ�������� �����㵱ǰ�����ϵĶ���λ��(�˴�������ͨ������֮��ı�Ĳ�������㷨�ߣ����Բ���Ҫ����һ�����ߵı任��
    // �õ��������� Mesh�е�currentVertices��attachments Ȩ�ص���ÿ�����������Ĺؽ��±�Ҫ+1 ��Ϊ���ڵ��Ѿ����ų���
    // ÿ���ؽڣ�m_joints)��bindWorldToJointTransform��currentJointToWorldTransform
    // ����ҵ�ܽ�4.4�Ĺ�ʽ

    // CPU�ϵ�ʵ��
    m_mesh.currentVertices.clear();
    for (int i = 0; i < m_mesh.bindVertices.size(); i++)
    {
        // ��ǰ����Ϊm_mesh.bindVertices[i]��λ�ã�vector3f)
        Vector3f verticePos;
        //vector<float> attachment = m_mesh.attachments[i]; // ����i��Ȩ���б� �Ż�
        for (int j = 0; j < m_mesh.attachments[i].size(); j++)
        {
            if (m_mesh.attachments[i][j] == 0)
                continue;

            // Ȩ���б�����j��Ӧ�Ĺؽ���j+1
            verticePos += (m_mesh.attachments[i][j] * m_joints[j + 1]->currentJointToWorldTransform *
                m_joints[j + 1]->bindWorldToJointTransform * Vector4f(m_mesh.bindVertices[i], 1.0f)
                ).xyz();
        }
        m_mesh.currentVertices.emplace_back(verticePos);
    }

    // �ⲿ�ָĳɴ���Uniforms ��GPU�ϼ���
    // ���ݶ�Ӧ�����attachment


}