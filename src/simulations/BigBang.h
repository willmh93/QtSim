#pragma once
#include "SpaceEngine.h"

SIM_BEG(BigBang)

using namespace SpaceEngine;

struct BigBang_Scene : public SpaceEngine_Scene
{
    int particle_count = 100;
    double zoom = 2;
    double spin = 0;

    bool focus_rect_initialized = false;
    FRect focus_rect;

    std::unique_ptr<QOpenGLShaderProgram> shader = nullptr;
    //QMatrix4x4 projectionMatrix;

    
    //void initializeGL() override;

    void loadShaders();

    void sceneAttributes();
    void sceneStart() override;
    void sceneProcess() override;
    void sceneMounted(Viewport* ctx) override;
    void viewportProcess(Viewport* ctx) override;
    void viewportDraw(Viewport* ctx) override;
};

struct BigBang_Project : public SpaceEngine_Project
{
    void projectPrepare() override;
};

SIM_END(BigBang)