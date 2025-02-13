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
    bool transform_coordinates = true;
    bool scale_lines_text = true;
    bool rotate_text = true;
    double seed = 0;

    vector<Particle> particles;

    Test_Instance()
    {
        // Args passed from Test.
        // Use initializer list for reference variables
    }

    void instanceAttributes(Options* options) override;

    void start() override;
    void mount(Panel *panel) override;
    void destroy() override;
    void processScene() override;
    void processPanel(Panel* ctx) override;
    void draw(Panel* ctx) override;

    //void mouseDown(MouseInfo mouse);
    //void mouseUp(MouseInfo mouse);
    //void mouseMove(MouseInfo mouse);
    //void mouseWheel(MouseInfo mouse);
};

struct Test : public Simulation
{
    int panel_count = 1;

    void projectAttributes(Options* options) override;
    void prepare() override;
};

SIM_END
#endif
