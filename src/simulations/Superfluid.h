#pragma once
#include "Project.h"

SIM_BEG(Superfluid)

struct Superfluid_Scene : public Scene
{
/*  // --- Custom Launch Config Example ---
       
    struct Config
    {
        double speed = 10.0;
    };

    Superfluid_Scene(Config& info) :
        speed(info.speed)
    {}

    double speed;
*/

    /// --- Sim Variables ---
    // double speed;
    float t = 0;

    // --- Scene management ---
    void sceneAttributes(Input* input) override;
    void sceneStart() override;
    //void sceneStop() override;
    void sceneDestroy() override;
    void sceneMounted(Viewport* viewport) override;

    // --- Sim Logic Here ---
    // void processParticles();

    void sceneProcess() override;

    // --- Shaders ---
    std::unique_ptr<QOpenGLShaderProgram> shader = nullptr;

    void initGL() override;

    // --- Viewport handling ---
    void viewportProcess(Viewport* ctx) override;
    void viewportDraw(Viewport* ctx) override;

    // --- Input ---
    void mouseDown() override;
    void mouseUp() override;
    void mouseMove() override;
    void mouseWheel() override;
};

struct Superfluid_Project : public Project
{
    int panel_count = 1;

    void projectAttributes(Input* input) override;
    void projectPrepare() override;
    //void projectStart() override;
    //void projectStop() override;
    //void projectDestroy() override;

};

SIM_END(Superfluid)