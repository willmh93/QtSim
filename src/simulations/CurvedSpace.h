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
    struct LaunchConfig
    {
        double radius_mult;
        double radius;
        std::vector<double> big_arr;

        LaunchConfig(double radius_mult = 1, double radius = 10) :
            radius_mult(radius_mult),
            radius(radius),
            big_arr(big_arr)
        {}
    };

    //static CurvedSpaceInstance* instantiateInstance(LaunchInfo& info)
    //{
    //    return new CurvedSpaceInstance(info.radius_mult, info.radius, info.big_arr);
    //}

    vector<unique_ptr<Particle>> particles;
    double gravity = 0.5;
    double &radius_mult;
    double radius;
    bool custom_color = false;

    std::vector<double>& big_arr;

    CurvedSpaceInstance(LaunchConfig& info) :
        radius_mult(info.radius_mult),
        radius(info.radius),
        big_arr(info.big_arr)
    {}

    /*CurvedSpaceInstance(double &radius_mult, double radius,std::vector<double>& big_arr) :
        radius_mult(radius_mult),
        radius(radius),
        big_arr(big_arr)
    {}*/

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

struct CurvedSpace : public Simulation<CurvedSpaceInstance>
{
    int panel_count = 4;
    
    shared_ptr<LaunchConfig> shared_config = make_shared<LaunchConfig>();

    

    void prepare() override;
    void projectAttributes(Options* options) override;
    
};

SIM_END(CurvedSpace)
#endif