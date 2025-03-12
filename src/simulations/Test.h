#ifndef TEST_H
#define TEST_H
#include "Project.h"



SIM_BEG(Test)

struct Particle : public Vec2
{
    double fx = 0, fy = 0;
    double vx, vy;
    Particle(double x, double y, double vx, double vy)
        : Vec2(x,y), vx(vx), vy(vy)
    {}
};


struct Test_Scene : public Scene
{
    // --- Custom Launch Config Example ---
       
    struct Config
    {
        double speed = 10.0;
    };

    Test_Scene(Config& info) :
        speed(info.speed)
    {}

    double speed;


    bool transform_coordinates = true;
    bool scale_lines_text = true;
    bool rotate_text = true;
    double seed = 0;

    double camera_x = 0;
    double camera_y = 0;
    double camera_rotation = 0;

    Vec2 ball_pos = { 0, 0 };

    vector<Particle> particles;

    

    //

    // Scene management
    void sceneAttributes(Input* options) override;
    void sceneStart() override;
    void sceneMounted(Viewport *viewport) override;
    void sceneDestroy() override;

    // --- Simulation processing ---
    void sceneProcess() override;

    // Viewport handling
    void viewportProcess(Viewport* ctx) override;
    void viewportDraw(Viewport* ctx) override;

    // Input
    void mouseDown() override;
    void mouseUp() override;
    void mouseMove() override;
    void mouseWheel() override;
};

struct Test_Project : public Project
{
    int viewport_count = 1;

    void projectAttributes(Input* options) override;
    void projectPrepare() override;
};

SIM_END(Test)
#endif
