#pragma once
#include "Simulation.h"

SIM_BEG(Particles)

struct Particle
{
    double x, y;
    double vx, vy;
};

struct Sim : public Simulation
{
    int particle_count;
    double gravity;

    std::vector<Particle> particles;

    void prepare();
    void start();
    void destroy();

    void process();
    void draw(QNanoPainter* p);

    void mouseWheel(int delta);
};

SIM_END