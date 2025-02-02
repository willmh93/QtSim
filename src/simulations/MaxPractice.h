#ifndef MAXPRACTICE_H
#define MAXPRACTICE_H
#include "Simulation.h"

SIM_BEG(MaxPractice)

struct Ball
{
    double x;
    double y;
    double r;
    double vx;
    double vy;
};


struct MaxPracticeInstance : public SimulationInstance
{
    //Ball ball;

    vector<Ball> balls;

    void prepare();
    void destroy();
    void process(DrawingContext* ctx);
    void draw(DrawingContext* ctx);

};

struct MaxPractice : public Simulation
{
    int panel_count = 1;

    void prepare();
    void start();
};

SIM_END
#endif
