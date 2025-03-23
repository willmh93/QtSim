#pragma once
#include "Project.h"
#include <complex>

SIM_BEG(Cardioid)

// Expose
double originalAngleFromPoint(
    double px, 
    double py, 
    double tolerance = 1e-10, 
    int maxIter = 50);

void cardioidPolarCoord(
    double px,
    double py,
    double& tangent_x,
    double& tangent_y,
    double& tangent_angle,
    double& dist);

double originalAngleFromPerpAngle(
    double perp_angle);

Vec2 fromCardioidTransform(
    double perp_angle,
    double dist);

struct Cardioid_Scene : public Scene
{
    // --- Variables ---
    double interact_angle = 0.0;
    double interact_angle_step = (2 * M_PI) / 720.0;
    
    bool show_offset = false;
    bool interactive = true;
    

    // --- Scene management ---
    void sceneAttributes(Input* input) override;
    //void sceneStart() override;
    //void sceneStop() override;
    //void sceneDestroy() override;
    void sceneMounted(Viewport* viewport) override;

    // --- Update methods ---
    
    //void sceneProcess() override;

    // --- Shaders ---
    //void loadShaders() override;

    // --- Viewport ---
    void fullPlot(Viewport* ctx, double scale, double ox);
    void fullPlotAlternative(Viewport* ctx, double scale, double ox);
    void fullPlotAlternative2(Viewport* ctx, double scale, double ox);
    void animatePlot(Viewport* ctx, double scale, double ox);

    void viewportProcess(Viewport* ctx) override;
    void viewportDraw(Viewport* ctx) override;

    // --- Input ---
    void mouseDown() override;
    void mouseUp() override;
    void mouseMove() override;
    void mouseWheel() override;
};

struct Cardioid_Project : public Project
{
    void projectAttributes(Input* input) override;
    void projectPrepare() override;

};

struct Cardioid_Graph_Scene : public Scene
{
    struct Config {
        Cardioid_Scene* main_scene;
    };

    Cardioid_Graph_Scene(Config& info) :
        main_scene(info.main_scene)
    {}

    Cardioid_Scene* main_scene;
    WorldBitmap bmp;

    int thread_count = 1;
    QThreadPool pool;

    void sceneAttributes(Input* input) override;
    void sceneStart();

    void sceneMounted(Viewport* viewport) override;
    void viewportProcess(Viewport* ctx) override;
    void viewportDraw(Viewport* ctx) override;
};

SIM_END(Cardioid)