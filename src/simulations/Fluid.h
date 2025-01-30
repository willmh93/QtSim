#ifndef FLUID_H
#define FLUID_H
#include "Simulation.h"

SIM_BEG(Fluid)

struct Particle : public Vec2
{
    double vx = 0;
    double vy = 0;

    Particle()
    {}

    Particle(double x, double y) : Vec2(x,y)
    {}
};

struct Fluid : public Simulation
{
    //Camera cam; // Optional camera for custom transforms

    int particle_count = 300;
    double world_w = 250.0;
    double world_h = 250.0;

    vector<Particle*> particles;
    vector<Link<Particle>> links;

    DelaunayTriangulation<Particle> delaunay;
    std::vector<Triangle<Particle>> triangles;

    double timestep = 0.1;

    double spring_dist = 20.0;
    double spring_stiffness = 0.1;
    double spring_damping = 0.001;

    double viscosity_strength = 0.5;
    double viscosity_dist = 200.0;

    void prepare();
    void start();
    void destroy();
    void process();
    void draw(QNanoPainter* p);

    //void extractTriangleLinks(std::set<Link>& edgeSet);

    void applyViscosity(double r, double strength, double dt);
    void spring(Particle* a, Particle* b, double restLength, double k, double damping, double deltaTime);

    void mouseDown(int x, int y, Qt::MouseButton btn);
    void mouseUp(int x, int y, Qt::MouseButton btn);
    void mouseMove(int x, int y);
    void mouseWheel(int delta);
};

SIM_END
#endif