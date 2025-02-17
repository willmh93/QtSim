#pragma once
#include "Project.h"

SIM_BEG(AnnoyingRacing_Project)

class Car : public Vec2
{
public:

    double max_speed;

    void draw(Viewport* ctx)
    {
        ctx->setFillStyle(255, 0, 255);
        ctx->fillRect(x, y, 20, 10);
    }
};

struct AnnoyingRacing_Scene : public Scene
{
/*
    // Custom Launch Config Example

    struct LaunchConfig
    {
        double particle_speed = 10.0;
    };
    
    AnnoyingRacing_Scene(LaunchConfig& info) : 
        particle_speed(info.particle_speed)
    {}
    
    double particle_speed;
*/

    vector<Car> cars;

    void sceneAttributes(Options* options) override;

    void sceneStart() override;
    //void sceneStop() override;
    void sceneDestroy() override;
    void sceneMounted(Viewport* viewport) override;
    void sceneProcess() override;

    void viewportProcess(Viewport* ctx) override;
    void viewportDraw(Viewport* ctx) override;

    void mouseDown() override;
    void mouseUp() override;
    void mouseMove() override;
    void mouseWheel() override;
};

struct AnnoyingRacing_Project : public Project<AnnoyingRacing_Scene>
{
    int panel_count = 1;

    void projectAttributes(Options* options) override;
    void projectPrepare() override;
    //void projectStart() override;
    //void projectStop() override;
    //void projectDestroy() override;

};

SIM_END(AnnoyingRacing_Project)