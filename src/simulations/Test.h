#ifndef TEST_H
#define TEST_H
#include "Simulation.h"


SIM_BEG(Test)

struct Particle : public Vec2
{
    double vx, vy;
    Particle(double x, double y, double vx, double vy)
        : Vec2(x,y), vx(vx), vy(vy)
    {}
};

struct Test_Instance : public SimulationInstance
{
    //using SimulationInstance::SimulationInstance;
    //struct LaunchConfig
    //{
    //};

    bool transform_coordinates = true;
    bool scale_lines_text = true;
    bool rotate_text = true;
    double seed = 0;

    double camera_x = 0;
    double camera_y = 0;
    double camera_rotation = 0;

    Vec2 ball_pos = { 0, 0 };

    vector<Particle> particles;

    /*Test_Instance(LaunchConfig &config)
    {
        // Args passed from Test.
        // Use initializer list for reference variables
    }*/

    void instanceAttributes(Options* options) override;

    void start() override;
    void mount(Panel *panel) override;
    void destroy() override;
    void processScene() override;
    void processPanel(Panel* ctx) override;
    void draw(Panel* ctx) override;

    void mouseDown() override;
    void mouseUp() override;
    void mouseMove() override;
    void mouseWheel() override;
};

struct Test : public Simulation<Test_Instance>
{
    int panel_count = 1;

    void projectAttributes(Options* options) override;
    void prepare() override;
};

SIM_END(Test)
#endif
