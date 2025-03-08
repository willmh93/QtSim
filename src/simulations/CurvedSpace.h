#pragma once
#include "Project.h"

SIM_BEG(CurvedSpace)

struct Particle : public Vec2
{
    Particle()
    {}

    Particle(double x, double y) : Vec2(x, y)
    {}
};

struct CurvedSpace_Scene : public Scene
{
    struct Config
    {
        double radius_mult;
        double radius;
        std::vector<double> big_arr;

        Config(double radius_mult = 1, double radius = 10) :
            radius_mult(radius_mult),
            radius(radius),
            big_arr(big_arr)
        {}
    };

    //static CurvedSpaceScene* instantiateScene(LaunchInfo& info)
    //{
    //    return new CurvedSpaceScene(info.radius_mult, info.radius, info.big_arr);
    //}

    vector<Particle> particles;
    double gravity = 0.5;
    double &radius_mult;
    double radius;
    bool custom_color = false;

    std::vector<double>& big_arr;

    CurvedSpace_Scene(Config& info) :
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
        for (auto& p : particles)
        {
            double dx = x - p.x;
            double dy = y - p.y;

            double r = sqrt(dx * dx + dy * dy);

            double force_magnitude = gravity * mass / (r * r); // No r^2 because F = G_2D * m1 * m2 / r
            double fx = force_magnitude * (dx / r);
            double fy = force_magnitude * (dy / r);

            // Compute accelerations
            double ax = fx / mass;
            double ay = fy / mass;

            // Update velocities of particle 1
            p.x += ax;
            p.y += ay;
        }
    }

    void sceneAttributes(Input* options) override;

    //void projectPrepare();
    void sceneStart() override;
    void sceneDestroy() override;
    void sceneProcess() override;
    void viewportDraw(Viewport* ctx) override;

    void mouseDown(MouseInfo mouse);
};

struct CurvedSpace_Project : public Project
{
    int viewport_count = 4;
    
    shared_ptr<CurvedSpace_Scene::Config> shared_config = make_shared<CurvedSpace_Scene::Config>();

    void projectPrepare() override;
    void projectAttributes(Input* options) override;
    
};

SIM_END(CurvedSpace)