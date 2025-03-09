#pragma once
#include "Project.h"

SIM_BEG(AnnoyingRacing)

class Car : public Vec2
{
public:

    const double car_w = 20;
    const double car_h = 10;
    const double acceleration = 0.01;

    bool accelerating = false;
    bool turning_left = false;
    bool turning_right = false;
    
    double vx = 0;
    double vy = 0;
    double angle = 0;
    double speed = 0;

    void process(Viewport* ctx)
    {
        if (turning_left)
            angle -= 0.01;
        if (turning_right)
            angle += 0.01;

        if (accelerating)
        {
            speed += acceleration;
        }

        vx = cos(angle) * speed;
        vy = sin(angle) * speed;

        x += vx;
        y += vy;
    }

    void draw(Viewport* ctx)
    {
        ctx->setFillStyle(255, 0, 255);

        ctx->save();
        ctx->translate(x, y);
        ctx->rotate(angle);
        ctx->fillRect(-car_w / 2 , -car_h / 2, car_w, car_h);
        ctx->restore();
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

    void sceneAttributes(Input* input) override;

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

    void keyPressed(QKeyEvent* e) override;
    void keyReleased(QKeyEvent* e) override;
};

struct AnnoyingRacing_Project : public Project
{
    int panel_count = 1;

    void projectAttributes(Input* options) override;
    void projectPrepare() override;
    //void projectStart() override;
    //void projectStop() override;
    //void projectDestroy() override;

};

SIM_END(AnnoyingRacing)