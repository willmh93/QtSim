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
    vector<unique_ptr<Particle>> particles;
    double gravity = 0.5;
    double &radius_mult;
    double radius;
    bool custom_color = false;

    CurvedSpaceInstance(double &radius_mult, double radius) :
        radius_mult(radius_mult),
        radius(radius)
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

    void instanceAttributes(Options* options) override;

    //void prepare();
    void start() override;
    void destroy() override;
    void processScene() override;
    void draw(Panel* ctx) override;

    void mouseDown(MouseInfo mouse);
};

struct CurvedSpace : public Simulation
{
    int panel_count = 4;
    double radius_mult = 1;

    void prepare() override;
    void projectAttributes(Options* options) override;
    
};

SIM_END
#endif