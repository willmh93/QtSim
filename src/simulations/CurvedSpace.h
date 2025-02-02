#ifndef CURVEDSPACE_H
#define CURVEDSPACE_H
#include "Simulation.h"

SIM_BEG(CurvedSpace)

struct Particle : public Vec2
{
    Particle()
    {}

    Particle(double x, double y) : Vec2(x, y)
    {}
};

struct CurvedSpaceInstance : public SimulationInstance
{
    /*int r;
    int g;
    int b;

    void prepare(int _r, int _g, int _b)
    {
        r = _r;
        g = _g;
        b = _b;
    }

    void process(DrawingContext& ctx)
    {

    }
    void draw(DrawingContext& ctx)
    {
        ctx.setFillStyle(r, g, b, 100);
        ctx.beginPath();
        ctx.circle(100, 100, 100);
        ctx.fill();
    }*/

    vector<unique_ptr<Particle>> particles;
    double &gravity;
    
    CurvedSpaceInstance(double& _gravity) : gravity(_gravity)
    {}

    void gravitateSpace(double x, double y, double mass)
    {
        for (const auto& p : particles)
        {
            double dx = x - p->x;
            double dy = y - p->y;

            double r = sqrt(dx * dx + dy * dy);

            double force_magnitude = gravity * mass / (r * r); // No r^2 because F = G_2D * m1 * m2 / r
            double fx = force_magnitude * (dx / r);
            double fy = force_magnitude * (dy / r);

            // Compute accelerations
            double ax = fx / mass;
            double ay = fy / mass;

            // Update velocities of particle 1
            p->x += ax;
            p->y += ay;
        }
    }

    void prepare();
    //void start();
    void destroy();
    void process(DrawingContext& ctx);
    void draw(DrawingContext& ctx);

    void mouseDown(MouseInfo mouse);
};

struct CurvedSpace : public Simulation
{
    double gravity = 1;

    void prepare();
    void start();
};

SIM_END
#endif