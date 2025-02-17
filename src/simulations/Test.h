#ifndef TEST_H
#define TEST_H
#include "Project.h"


SIM_BEG(Test)

struct Particle : public Vec2
{
    double vx, vy;
    Particle(double x, double y, double vx, double vy)
        : Vec2(x,y), vx(vx), vy(vy)
    {}
};

struct Test_Scene : public Scene
{
    //using Scene::Scene;
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

    /*Test_Scene(LaunchConfig &config)
    {
        // Args passed from Test.
        // Use initializer list for reference variables
    }*/

    void sceneAttributes(Options* options) override;
    void sceneStart() override;
    void sceneMounted(Viewport *viewport) override;
    void sceneDestroy() override;
    void sceneProcess() override;

    void viewportProcess(Viewport* ctx) override;
    void viewportDraw(Viewport* ctx) override;

    void mouseDown() override;
    void mouseUp() override;
    void mouseMove() override;
    void mouseWheel() override;
};

struct Test : public Project<Test_Scene>
{
    int viewport_count = 1;

    void projectAttributes(Options* options) override;
    void projectPrepare() override;
};

SIM_END(Test)
#endif
