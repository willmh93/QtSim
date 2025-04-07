#pragma once
#include "Project.h"

SIM_BEG(Multithreading)

struct Particle : public Vec2
{
    double fx = 0, fy = 0;
    double vx, vy;
    Particle(double x, double y, double vx, double vy)
        : Vec2(x, y), vx(vx), vy(vy)
    {}
};

struct Multithreading_Scene : public Scene
{
    /// --- Sim Variables ---
    QThreadPool pool;
    int thread_count = 4;

    vector<Particle> particles;
    double acceleration = 0.0000002;

    /// --- Scene management ---
    void sceneAttributes() override;
    void sceneStart() override;
    //void sceneStop() override;
    void sceneDestroy() override;
    void sceneMounted(Viewport* viewport) override;

    /// --- Sim Logic Here ---
    void computeForces();
    void applyForces();
    void updatePositions();


    void sceneProcess() override;

    /// --- Viewport handling ---
    void viewportProcess(Viewport* ctx) override;
    void viewportDraw(Viewport* ctx) override;

    /// --- Input ---
    void mouseDown() override;
    void mouseUp() override;
    void mouseMove() override;
    void mouseWheel() override;
};

struct Multithreading_Project : public Project
{
    int panel_count = 1;

    void projectAttributes() override;
    void projectPrepare() override;
    //void projectStart() override;
    //void projectStop() override;
    //void projectDestroy() override;

};

SIM_END(Multithreading)