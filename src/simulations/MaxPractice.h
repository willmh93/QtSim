#ifndef MAXPRACTICE_H
#define MAXPRACTICE_H
#include "Project.h"

SIM_BEG(MaxPractice)


struct Ball
{
    double rad;

    union
    {
        struct
        {
            double x;
            double y;
            double vx;
            double vy;
        };
        uint8_t buffer[sizeof(double) * 4];
    };
    
    int r, g, b;
};


struct MaxPractice_Scene : public Scene
{
    struct LaunchInfo
    {
    };

    //Ball ball;

    vector<Ball> balls;
    int particle_count = 20000;
    double max_speed = 5;
    double world_size = 500;

    void sceneAttributes(Input* options) override;

    void sceneStart() override;
    void sceneDestroy() override;
    void sceneProcess() override;
    void viewportDraw(Viewport* ctx) override;
};



struct MaxPractice_Project : public Project
{
    int viewport_count = 1;

    void projectPrepare();
    void projectStart();
};

SIM_END(MaxPractice)
#endif
