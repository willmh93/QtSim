#pragma once
#include "Project.h"

SIM_BEG(GLCube)

struct GLCube_Scene : public Scene
{
    // --- Variables ---
    GLSurface surface;
    Bitmap bmp;

    QOpenGLShaderProgram* shaderProgram = nullptr;
    GLuint VAO, VBO, EBO;
    float angle = 0.0f;

    float camera_rotation = 0;



    // --- Scene management ---

    void sceneAttributes() override;
    void sceneStart() override;
    ///void sceneStop() override;
    void sceneDestroy() override;
    void sceneMounted(Viewport* viewport) override;

    // --- Update methods ---

    void sceneProcess() override;

    // --- Shaders ---

    void initGL() override;

    // --- Viewport ---

    void viewportProcess(Viewport* ctx) override;
    void viewportDraw(Viewport* ctx) override;

    // --- Input ---
    void mouseDown() override;
    void mouseUp() override;
    void mouseMove() override;
    void mouseWheel() override;
};

struct GLCube_Project : public Project
{
    int panel_count = 2;

    void projectAttributes() override;
    void projectPrepare() override;
    //void projectStart() override;
    //void projectStop() override;
    //void projectDestroy() override;

};

SIM_END(GLCube)