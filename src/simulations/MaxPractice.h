#ifndef MAXPRACTICE_H
#define MAXPRACTICE_H
#include "Simulation.h"

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


struct MaxPracticeInstance : public SimulationInstance
{
    struct LaunchInfo
    {
    };

    //Ball ball;

    vector<Ball> balls;
    int particle_count = 20000;
    double max_speed = 5;
    double world_size = 500;

    void instanceAttributes(Options* options) override;

    void start() override;
    void destroy() override;
    void processScene() override;
    void draw(Panel* ctx) override;
};



struct MaxPractice : public Simulation<MaxPracticeInstance>
{
    int panel_count = 1;

    void prepare();
    void start();
};

SIM_END(MaxPractice)
#endif
