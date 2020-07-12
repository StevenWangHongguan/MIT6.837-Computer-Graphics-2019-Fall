#include "gl.h"
#include <GLFW/glfw3.h>

#include <cmath>
#include <vector>
#include <cassert>
#include <iostream>
#include <sstream>
#include <fstream>

#include <vecmath.h>
#include "starter0_util.h"
#include "recorder.h"
#include "teapot.h"

using namespace std;


// Globals
uint32_t program;

// This is the list of points (3D vectors)
vector<Vector3f> vecv;

// This is the list of normals (also 3D vectors)
vector<Vector3f> vecn;

// This is the list of faces (indices into vecv and vecn)
vector<vector<unsigned>> vecf;

// You will need more global variables to implement color and position changes
float colorValue = 0.0;
float lightPosVerticalOffset = 0.0f;
float lightPosHorizonalOffset = 0.0f;
int modelRotateValueY = 0;

void keyCallback(GLFWwindow* window, int key,
    int scancode, int action, int mods)
{
    if (action == GLFW_RELEASE) { // only handle PRESS and REPEAT
        return;
    }

    // Special keys (arrows, CTRL, ...) are documented
    // here: http://www.glfw.org/docs/latest/group__keys.html
    if (key == GLFW_KEY_ESCAPE) {
        printf("Unhandled key press %d\n", key);
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    } /*else if (key == 'A') {
        printf("Key A pressed\n");
    }*/ else if(key == GLFW_KEY_C) {
        colorValue += 0.1;
    }
    else if (key == GLFW_KEY_W) {
        lightPosVerticalOffset += 0.5;
    }
    else if (key == GLFW_KEY_S) {
        lightPosVerticalOffset -= 0.5;
    }
    else if (key == GLFW_KEY_A) {
        lightPosHorizonalOffset -= 0.5;
    }
    else if (key == GLFW_KEY_D) {
        lightPosHorizonalOffset += 0.5;
    }
    else if (key == GLFW_KEY_R) {
        modelRotateValueY -= 10.0;
    }
    else if (key == GLFW_KEY_Q) {
        modelRotateValueY += 10.0;
    }
    else {
        printf("Unhandled key press %d\n", key);
    }
}

void drawTriangle()
{
    // set a reasonable upper limit for the buffer size
    GeometryRecorder rec(1024);
    rec.record(Vector3f(0.0, 0.0, 0.0), // Position
        Vector3f(0.0, 0.0, 1.0));// Normal

    rec.record(Vector3f(3.0, 0.0, 0.0),
        Vector3f(0.0, 0.0, 1.0));

    rec.record(Vector3f(3.0, 3.0, 0.0),
        Vector3f(0.0, 0.0, 1.0));
    rec.draw();
}

void drawTeapot()
{
    // set the required buffer size exactly.
    GeometryRecorder rec(teapot_num_faces * 3);
    for (int idx : teapot_indices) {
        Vector3f position(teapot_positions[idx * 3 + 0],
            teapot_positions[idx * 3 + 1],
            teapot_positions[idx * 3 + 2]);

        Vector3f normal(teapot_normals[idx * 3 + 0],
            teapot_normals[idx * 3 + 1],
            teapot_normals[idx * 3 + 2]);

        rec.record(position, normal);
    }
    rec.draw();
}

void drawObjMesh() {
    // draw obj mesh here
    // read vertices and face indices from vecv, vecn, vecf
    GeometryRecorder rec(vecf.size() * 3);
    for (auto faceData : vecf)
    {
        unsigned int a, b, c, d, e, f, g, h, i;
        a = faceData[0], b = faceData[1], c = faceData[2];
        d = faceData[3], e = faceData[4], f = faceData[5];
        g = faceData[6], h = faceData[7], i = faceData[8];
        // ��˳�����ÿһ����Ԫ���������� ��������λ�úͶ��㷨����
        rec.record(vecv[a - 1], vecn[c - 1]);
        rec.record(vecv[d - 1], vecn[f - 1]);
        rec.record(vecv[g - 1], vecn[i - 1]);
    }
    rec.draw();
}

// This function is responsible for displaying the object.
void drawScene()
{
    drawObjMesh();
    //drawTeapot();
}

void setViewport(GLFWwindow* window)
{
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    // make sure the viewport is square-shaped.
    if (width > height) {
        int offsetx = (width - height) / 2;
        glViewport(offsetx, 0, height, height);
    } else {
        int offsety = (height - width) / 2;
        glViewport(0, offsety, width, width);
    }
}

GLfloat* colorInterpolation(GLfloat* color1, GLfloat* color2, float ratio)
{
    GLfloat* newColor = new GLfloat[4]; //��̬�����ڴ� ��ʹ�þ�̬����ľֲ����� �뿪����֮����Զ����� �ͷ��ʲ��������Ԫ����
    newColor[0] = (1 - ratio) * color1[0] + ratio * color2[0]; //���Բ�ֵ
    newColor[1] = (1 - ratio) * color1[1] + ratio * color2[1]; //���Բ�ֵ
    newColor[2] = (1 - ratio) * color1[2] + ratio * color2[2]; //���Բ�ֵ
    newColor[3] = (1 - ratio) * color1[3] + ratio * color2[3]; //���Բ�ֵ
    return newColor;
}

void updateCameraUniforms()
{
    // Set up a perspective view, with square aspect ratio
    float fovy_radians = deg2rad(50.0f);
    float nearz = 1.0f;
    float farz = 100.0f;
    float aspect = 1.0f;
    Matrix4f P = Matrix4f::perspectiveProjection( // projection matrix
        fovy_radians, aspect, nearz, farz);

    // See https://www.opengl.org/sdk/docs/man/html/glUniform.xhtml
    // for the many version of glUniformXYZ()
    // Returns -1 if uniform not found.
    int loc = glGetUniformLocation(program, "P");
    glUniformMatrix4fv(loc, 1, false, P);

    Vector3f eye(0.0, 0.0, 7.0f);
    Vector3f center(0.0, 0.0, 0.0);
    Vector3f up(0.0, 1.0f, -0.2f);
    Matrix4f V = Matrix4f::lookAt(eye, center, up); //view matrix
    loc = glGetUniformLocation(program, "V");
    glUniformMatrix4fv(loc, 1, false, V);
    loc = glGetUniformLocation(program, "camPos");
    glUniform3fv(loc, 1, eye);

    // Make sure the model is centered in the viewport
    // We translate the model using the "Model" matrix
    Matrix4f M = Matrix4f::rotateY(deg2rad(modelRotateValueY)) * Matrix4f::translation(0, -1.0, 0); //model matrix
    loc = glGetUniformLocation(program, "M");
    glUniformMatrix4fv(loc, 1, false, M);

    // Transformation matrices act differently
    // on vectors than on points.
    // The inverse-transpose is what we want.
    Matrix4f N = M.inverse().transposed();
    loc = glGetUniformLocation(program, "N");
    glUniformMatrix4fv(loc, 1, false, N);
}

void updateMaterialUniforms()
{
    // Here are some colors you might use - feel free to add more
    GLfloat diffColors[4][4] = { 
    { 0.5f, 0.5f, 0.9f, 1.0f },
    { 0.9f, 0.5f, 0.5f, 1.0f },
    { 0.5f, 0.9f, 0.3f, 1.0f },
    { 0.3f, 0.8f, 0.9f, 1.0f } };

    int colorValueInt = floor(colorValue);
    float colorValueFloat = colorValue - colorValueInt;
    GLfloat* resultColor = colorInterpolation(diffColors[colorValueInt % 4], diffColors[(colorValueInt + 1) % 4], colorValueFloat);

    // Here we use the first color entry as the diffuse color
    int loc = glGetUniformLocation(program, "diffColor");
    glUniform4fv(loc, 1, resultColor);

    // Define specular color and shininess
    GLfloat specColor[] = { 0.2f, 0.2f, 0.2f, 1.0f };
    GLfloat shininess[] = { 10.0f };

    // Note that the specular color and shininess can stay constant
    loc = glGetUniformLocation(program, "specColor");
    glUniform4fv(loc, 1, specColor);
    loc = glGetUniformLocation(program, "shininess");
    glUniform1f(loc, shininess[0]);

    delete resultColor;
}

void updateLightUniforms()
{
    // Light Position
    GLfloat lightPos[] = { 2.0f, 3.0f, 5.0f, 1.0f };
    int loc = glGetUniformLocation(program, "lightPos");
    lightPos[0] += lightPosHorizonalOffset;
    lightPos[1] += lightPosVerticalOffset;
    glUniform4fv(loc, 1, lightPos);
    // Light Color
    GLfloat lightDiff[] = { 120.0, 120.0, 120.0, 1.0 };
    loc = glGetUniformLocation(program, "lightDiff");
    glUniform4fv(loc, 1, lightDiff);
}

void loadInput()
{
    // load the OBJ file here
    string objFilePath = "../data/garg.obj";
    ifstream inputFile;
    inputFile.open(objFilePath);
    string line;
    while (getline(inputFile, line))
    {
        istringstream ss(line);
        string type;
        ss >> type;
        if (type == "v")
        {
            Vector3f vertexPos;
            ss >> vertexPos[0] >> vertexPos[1] >> vertexPos[2];
            vecv.push_back(vertexPos);
        }
        else if (type == "vn")
        {
            Vector3f vertexNormal;
            ss >> vertexNormal[0] >> vertexNormal[1] >> vertexNormal[2];
            vecn.push_back(vertexNormal);
        }
        else if (type == "f")
        {
            unsigned int a, b, c, d, e, f, g, h, i;
            char slash;
            ss >> a >> slash >> b >> slash >> c;
            ss >> d >> slash >> e >> slash >> f;
            ss >> g >> slash >> h >> slash >> i;
            vector<unsigned int> faceData = { a,b,c,d,e,f,g,h,i };
            vecf.push_back(faceData);
        }
    }
}

void fixKeyInputValue()
{
    modelRotateValueY %= 360;
}

// Main routine.
// Set up OpenGL, define the callbacks and start the main loop
int main(int argc, char** argv)
{
    
    loadInput();

    GLFWwindow* window = createOpenGLWindow(1280, 960, "a0");
    
    // setup the keyboard event handler
    glfwSetKeyCallback(window, keyCallback);

    // glEnable() and glDisable() control parts of OpenGL's
    // fixed-function pipeline, such as rasterization, or
    // depth-buffering. What happens if you remove the next line?
    glEnable(GL_DEPTH_TEST);

    // The program object controls the programmable parts
    // of OpenGL. All OpenGL programs define a vertex shader
    // and a fragment shader.
    program = compileProgram(c_vertexshader, c_fragmentshader);
    if (!program) {
        printf("Cannot compile program\n");
        return -1;
    }

    glUseProgram(program);

    // Main Loop
    while (!glfwWindowShouldClose(window)) {
        // Clear the rendering window
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        setViewport(window);

        fixKeyInputValue();

        updateCameraUniforms();
        updateLightUniforms();
        updateMaterialUniforms();

        // Draw to back buffer
        drawScene();

        // Make back buffer visible
        glfwSwapBuffers(window);

        // Check if any input happened during the last frame
        glfwPollEvents();
    }

    // All OpenGL resource that are created with
    // glGen* or glCreate* must be freed.
    glDeleteProgram(program);

    glfwTerminate(); // destroy the window
    return 0;
}

