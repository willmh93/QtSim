#ifndef CURVEDSPACE_H
#define CURVEDSPACE_H
#include "Project.h"

SIM_BEG(CurvedSpace)

struct Particle : public Vec2
{
    Particle()
    {}

    Particle(double x, double y) : Vec2(x, y)
    {}
};

struct CurvedSpaceScene : public Scene
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

    //static CurvedSpaceScene* instantiateScene(LaunchInfo& info)
    //{
    //    return new CurvedSpaceScene(info.radius_mult, info.radius, info.big_arr);
    //}

    vector<unique_ptr<Particle>> particles;
    double gravity = 0.5;
    double &radius_mult;
    double radius;
    bool custom_color = false;

    std::vector<double>& big_arr;

    CurvedSpaceScene(LaunchConfig& info) :
        radius_mult(info.radius_mult),
        radius(info.radius),
        big_arr(info.big_arr)
    {}

    /*CurvedSpaceScene(double &radius_mult, double radius,std::vector<double>& big_arr) :
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

    void sceneAttributes(Options* options) override;

    //void projectPrepare();
    void sceneStart() override;
    void sceneDestroy() override;
    void sceneProcess() override;
    void viewportDraw(Viewport* ctx) override;

    void mouseDown(MouseInfo mouse);
};

struct CurvedSpace : public Project<CurvedSpaceScene>
{
    int viewport_count = 4;
    
    shared_ptr<LaunchConfig> shared_config = make_shared<LaunchConfig>();

    

    void projectPrepare() override;
    void projectAttributes(Options* options) override;
    
};

SIM_END(CurvedSpace)
#endif