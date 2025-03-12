#ifndef FLUID_H
#define FLUID_H
#include "Project.h"

SIM_BEG(Fluid)

struct Particle : public Vec2
{
    double vx = 0;
    double vy = 0;

    Particle(){}
    Particle(double x, double y) : Vec2(x,y) {}
};

struct Fluid_Scene : public Scene
{
    double world_w = 250.0;
    double world_h = 250.0;

    vector<Particle*> particles;
    vector<Link<Particle>> links;

    DelaunayTriangulation<Particle> delaunay;
    std::vector<Triangle<Particle>> triangles;

    double timestep = 0.1;
    int particle_count = 100;
    double spring_dist = 20.0;
    double spring_stiffness = 1;
    double spring_damping = 0.01;
    double viscosity_strength = 1;
    double viscosity_spring_dist_ratio = 10.0;
    
    void sceneAttributes(Input* options) override;

    void sceneStart() override;
    void sceneDestroy() override;
    void sceneProcess() override;
    void viewportDraw(Viewport* ctx) override;

    void applyLinkViscosity(Particle* a, Particle* b, double r, double strength, double dt);
    void applyViscosityAll(double r, double strength, double dt);

    void spring(Particle* a, Particle* b, double restLength, double k, double damping, double deltaTime);
};

struct Fluid_Project : public Project
{
    int viewport_count = 4;

    void projectPrepare() override;
    //void projectStart();
    void projectAttributes(Input* options) override;
    
};


SIM_END(Fluid)
#endif