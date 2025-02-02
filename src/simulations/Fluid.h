#ifndef FLUID_H
#define FLUID_H
#include "Simulation.h"

SIM_BEG(Fluid)

struct Particle : public Vec2
{
    double vx = 0;
    double vy = 0;

    Particle(){}
    Particle(double x, double y) : Vec2(x,y) {}
};

struct FluidInstance : public SimulationInstance, public Options
{
    double world_w = 250.0;
    double world_h = 250.0;

    vector<Particle*> particles;
    vector<Link<Particle>> links;

    DelaunayTriangulation<Particle> delaunay;
    std::vector<Triangle<Particle>> triangles;

    void prepare();
    
    void destroy();
    void process(DrawingContext* ctx);
    void draw(DrawingContext* ctx);

    void applyLinkViscosity(Particle* a, Particle* b, double r, double strength, double dt);
    void applyViscosityAll(double r, double strength, double dt);

    void spring(Particle* a, Particle* b, double restLength, double k, double damping, double deltaTime);
};

struct Fluid : public Simulation
{
    int panel_count = 4;

    void prepare();
    void start();
};


SIM_END
#endif