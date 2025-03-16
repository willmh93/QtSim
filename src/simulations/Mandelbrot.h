#pragma once
#include "Project.h"

SIM_BEG(Mandelbrot)

struct Mandelbrot_Scene : public Scene
{
/*  // --- Custom Launch Config Example ---
       
    struct Config
    {
        double speed = 10.0;
    };

    Mandelbrot_Scene(Config& info) :
        speed(info.speed)
    {}

    double speed;
*/

    /// --- Sim Variables ---
    Bitmap bmp;

    QThreadPool pool;
    int thread_count = 1;

    double quality = 10.0;
    double threshold = 2;
    double y_mult = 2;
    int exponent = 2;
    int version = 0;
    bool smoothing = 0;

    bool gpu_compute = false;

    double camera_x = 0;
    double camera_y = 0;

    // --- Scene management ---
    void sceneAttributes(Input* input) override;
    void sceneStart() override;
    //void sceneStop() override;
    void sceneDestroy() override;
    void sceneMounted(Viewport* viewport) override;

    /// --- Sim Logic Here ---
    std::tuple<uint8_t, uint8_t, uint8_t> smooth_color(double iter, int max_iter);
    std::tuple<uint8_t, uint8_t, uint8_t> ratio_color(double ratio);


    double mandelbrot_basic(double x0, double y0, int max_iter);
    double mandelbrot_2(double x0, double y0, int max_iter);
    void   mandelbrot_3(double x0, double y0, int max_iter, Vec2& p, double& smooth_iter);
   
    void sceneProcess() override;

    // --- Shaders ---
    //std::unique_ptr<QOpenGLShaderProgram> shader = nullptr;

    GlContext context;
    ComputeShader shader;
    std::vector<float> input;
    std::vector<float> output;
    ShaderBuffer input_buffer, output_buffer;

    void loadShaders() override;

    // --- Viewport handling ---
    void viewportProcess(Viewport* ctx) override;
    void viewportDraw(Viewport* ctx) override;

    // --- Input ---
    void mouseDown() override;
    void mouseUp() override;
    void mouseMove() override;
    void mouseWheel() override;
};

struct Mandelbrot_Project : public Project
{
    int panel_count = 1;

    void projectAttributes(Input* input) override;
    void projectPrepare() override;
    //void projectStart() override;
    //void projectStop() override;
    //void projectDestroy() override;

};

SIM_END(Mandelbrot)