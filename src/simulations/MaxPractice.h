#ifndef MAXPRACTICE_H
#define MAXPRACTICE_H
#include "Simulation.h"

SIM_BEG(MaxPractice)


struct Ball
{
    double x;
    double y;
    double rad;
    double vx;
    double vy;
    
    int r, g, b;
};


struct MaxPracticeInstance : public SimulationInstance
{
    //Ball ball;

    vector<Ball> balls;
    int particle_count = 100;
    double max_speed = 5;
    double world_size = 500;

    void instanceAttributes() override;

    void start();
    void destroy();
    void process(DrawingContext* ctx);
    void draw(DrawingContext* ctx);

};



struct MaxPractice : public Simulation<MaxPracticeInstance>
{
    int panel_count = 1;

    void prepare();
    void start();
};

SIM_END
#endif
