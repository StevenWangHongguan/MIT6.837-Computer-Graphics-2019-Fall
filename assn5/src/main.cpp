#include "gl.h"
#include <GLFW/glfw3.h>

#include <cmath>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <vector>
#include <lodepng.h>
#include <map>
#include <cstdint>

#include "objparser.h"

// some utility code is tucked away in main.h
// for example, drawing the coordinate axes
// or helpers for setting uniforms.
#include "main.h"

// 4096x4096 is a pretty large texture. Extensions to shadow algorithm
// (extra credit) help lowering this memory footprint.
const int SHADOW_WIDTH = 4096;
const int SHADOW_HEIGHT = 4096;

// FUNCTION DECLARATIONS - you will implement these
void loadTextures();
void freeTextures();

void loadFramebuffer();
void freeFramebuffer();

void draw();

Matrix4f getLightView();
Matrix4f getLightProjection();

void setOtherUniforms();

// Globals here.
objparser scene;
Vector3f  light_dir;
glfwtimer timer;

std::map<std::string, unsigned int> texture_map; // ����ͨ������������ҵ���Ӧ��glTexture

GLuint fb; // ֡���������
GLuint fb_depthtex; // �������
GLuint fb_colortex; // ��ɫ����

Vector3f scene_center(0.0f, 0.5f, 0.0f); // �������� 0, 0.5, 0
float distance_light_to_scene = 20.0f; // ��Դ�������ľ���

// FUNCTION IMPLEMENTATIONS

// animate light source direction       �ƶ���Դ�ķ���
// this one is implemented for you      �Ѿ�ʵ��
void updateLightDirection() {
    // feel free to edit this
    float elapsed_s = timer.elapsed();
    //elapsed_s = 88.88f;
    float timescale = 0.3f;
    light_dir = Vector3f(2.0f * sinf((float)elapsed_s * 1.5f * timescale), 5.0f, 2.0f * cosf(2 + 1.9f * (float)elapsed_s * timescale));
    light_dir.normalize();
}


void drawScene(GLint program, Matrix4f V, Matrix4f P) {
    Matrix4f M = Matrix4f::identity();
    updateTransformUniforms(program, M, V, P);

    VertexRecorder recorder;
    for (auto b : scene.batches)
    {
        recorder.clear();

        int index = 0;
        for (int i = b.start_index; i < b.start_index + b.nindices; i++) // ��batch��startIndex=3 ����3������ ��ô����[3,6)
        {
            index = scene.indices[i];
            recorder.record(scene.positions[index], scene.normals[index], Vector3f(scene.texcoords[index], 0.0f));
        }

        material m = b.mat;
        updateMaterialUniforms(program, m.diffuse, m.ambient, m.specular, m.shininess);

        // ������
        //if (texture_map.find(m.diffuse_texture) != texture_map.end())
        {
            unsigned int gltexture = texture_map[m.diffuse_texture];
            glBindTexture(GL_TEXTURE_2D, gltexture);
        }

        recorder.draw();
    }
}

void draw() {
    // 2. DEPTH PASS
    // - bind framebuffer
    // - configure viewport
    // - compute camera matrices (light source as camera)
    // - call drawScene
    glBindFramebuffer(GL_FRAMEBUFFER, fb);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glUseProgram(program_color);
    drawScene(program_color, getLightView(), getLightProjection());  // �ⲿ��Ӧ���ǰѴӹ�Դ���򳡾��������Ϣ��Ⱦ��֡����󶨵���ͼ���� Ҳ����depth texture��fb_depthtex) �Լ�fb_colortex

    // 1. LIGHT PASS
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    int winw, winh;
    glfwGetFramebufferSize(window, &winw, &winh);
    glViewport(0, 0, winw, winh);
    glUseProgram(program_light);
    updateLightUniforms(program_light, light_dir, Vector3f(1.2f, 1.2f, 1.2f));

    setOtherUniforms(); // ������Ӱ��������� �󶨶�Ӧ����Ԫ ����light_VP����
    glActiveTexture(GL_TEXTURE1); // ��������Ԫ1 ��������shadowTex
    glBindTexture(GL_TEXTURE_2D, fb_depthtex); // �������ͼ��shadowTex������
    glActiveTexture(GL_TEXTURE0);

    // TODO IMPLEMENT drawScene
    drawScene(program_light, camera.GetViewMatrix(), camera.GetPerspective());

    // 3. DRAW DEPTH TEXTURE AS QUAD
    // drawTexturedQuad() helper in main.h is useful here.

    glViewport(0, 0, 512, 512);
    drawTexturedQuad(fb_depthtex);
    glViewport(512, 0, 512, 512);
    drawTexturedQuad(fb_colortex);
}

// Main routine.
// Set up OpenGL, define the callbacks and start the main loop
int main(int argc, char* argv[])
{
    std::string basepath = "./";
    if (argc > 2) {
        printf("Usage: %s [basepath]\n", argv[0]);
    }
    else if (argc == 2) {
        basepath = argv[1];
    }
    printf("Loading scene and shaders relative to path %s\n", basepath.c_str());

    // load scene data
    // parsing code is in objparser.cpp
    // take a look at the public interface in objparser.h
    if (!scene.parse("data/sponza_low/sponza_norm.obj")) { // basepath + "data/sponza_low/sponza_norm.obj"
        return -1;
    }

    window = createOpenGLWindow(1024, 1024, "Assignment 5");

    // setup the event handlers     // ���̡���ꡢ���ֵ��¼����
    // key handlers are defined in main.h
    // take a look at main.h to know what's in there.
    glfwSetKeyCallback(window, keyCallback);
    glfwSetMouseButtonCallback(window, mouseCallback);
    glfwSetCursorPosCallback(window, motionCallback);

    glClearColor(0.8f, 0.8f, 1.0f, 1);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);   //��

    // TODO add loadXYZ() function calls here
    loadTextures();
    loadFramebuffer();

    camera.SetDimensions(600, 600);
    camera.SetPerspective(50);
    camera.SetDistance(10);
    camera.SetCenter(Vector3f(0, 1, 0));
    camera.SetRotation(Matrix4f::rotateY(1.6f) * Matrix4f::rotateZ(0.4f));

    // set timer for animations
    timer.set();
    while (!glfwWindowShouldClose(window)) {
        setViewportWindow(window);

        // we reload the shader files each frame.
        // this shaders can be edited while the program is running
        // loadPrograms/freePrograms is implemented in main.h
        bool valid_shaders = loadPrograms(basepath);
        if (valid_shaders) {

            // draw coordinate axes
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            if (gMousePressed) {
                drawAxis();
            }

            // update animation
            updateLightDirection();

            // draw everything
            draw();
        }
        // make sure to release the shader programs.
        freePrograms();

        // Make back buffer visible
        glfwSwapBuffers(window);

        // Check if any input happened during the last frame
        glfwPollEvents();
    } // END OF MAIN LOOP

    // All OpenGL resource that are created with
    // glGen* or glCreate* must be freed.

    // TODO: add freeXYZ() function calls here
    freeFramebuffer();
    freeTextures();

    glfwDestroyWindow(window);


    return 0;	// This line is never reached.
}

void loadTextures()
{
    // ����scene.textures(һ��string->rgbImage)
    for (auto iter = scene.textures.begin(); iter != scene.textures.end(); iter++)
    {
        auto texture = iter->first;
        auto& image = iter->second;

        unsigned int gltexture;
        glGenTextures(1, &gltexture);
        glBindTexture(GL_TEXTURE_2D, gltexture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, image.w, image.h, 0, GL_RGB, GL_UNSIGNED_BYTE, image.data.data());  //!���������� �����

        glGenerateMipmap(GL_TEXTURE_2D);
        texture_map.insert(std::make_pair(texture, gltexture));
    }
}

void freeTextures()
{
    for (auto iter = texture_map.begin(); iter != texture_map.end(); iter++)
    {
        unsigned int gltexture = iter->second;
        glDeleteTextures(1, &gltexture);
    }
}

void loadFramebuffer()
{
    // ��������������� ������Ⱦʱ֡����д����ɫ�������Ϣ ��ʼ��Ϊ��
    glGenTextures(1, &fb_colortex);
    glBindTexture(GL_TEXTURE_2D, fb_colortex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glGenTextures(1, &fb_depthtex);
    glBindTexture(GL_TEXTURE_2D, fb_depthtex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr); //GL_DEPTH_COMPONENT32F?
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glBindTexture(GL_TEXTURE_2D, 0);

    glGenFramebuffers(1, &fb);
    glBindFramebuffer(GL_FRAMEBUFFER, fb);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fb_colortex, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, fb_depthtex, 0);

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        printf("ERROR, _incomplete_framebuffer\n");
        exit(-1);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0); // �󶨻�Ĭ�ϵ�֡�������
}

void freeFramebuffer()
{
    glDeleteFramebuffers(1, &fb);
    glDeleteTextures(1, &fb_colortex);
    glDeleteTextures(1, &fb_depthtex);
}

Matrix4f getLightView()
{
    Vector3f eye = scene_center + light_dir * distance_light_to_scene;  // light_dir�����������ķ���

    //Vector3f right = Vector3f::cross(-light_dir, Vector3f(0, 1, 0)).normalized(); // �������������  ��ֱ�ڹ��߷��� ָ��������ұ�
    //Vector3f up = Vector3f::cross(right, -light_dir).normalized();

    Vector3f up = Vector3f(-light_dir.y(), light_dir.x(), 0).normalized();  // ��������light_dir�����ĵ��Ϊ0 �����Ǵ�ֱ����
    return Matrix4f::lookAt(eye, scene_center, up);
}
Matrix4f getLightProjection()
{
    return Matrix4f::orthographicProjection(40, 40, 1, 30);
    /*float scale = 50;
    float clip = 10;
    return Matrix4f::orthographicProjection(scale, scale, -clip, clip);*/
}

void setOtherUniforms()
{
    int loc = glGetUniformLocation(program_light, "shadowTex");
    glUniform1i(loc, 1);
    
    Matrix4f light_VP_matrix = getLightProjection() * getLightView();
    loc = glGetUniformLocation(program_light, "light_VP");
    glUniformMatrix4fv(loc, 1, false, &light_VP_matrix(0,0)); // �ο�learnopengl�е�shader.cpp��setMat4������glm�е�type_ptrʵ��
}