#pragma once
#include "Project.h"

SIM_BEG(Mandelbrot)

struct Mandelbrot_Scene : public Scene
{
    /// --- Sim Variables ---
    GLBitmap bmp;

    QThreadPool pool;
    int thread_count = 1;
    bool gpu_compute = false;

    bool discrete_step = false;
    double quality = 20.0;
    double threshold = 50;
    bool thresholding = false;
    bool smoothing = true;

    int iter_lim;

    double camera_x = 0;
    double camera_y = 0;

    // --- Scene management ---
    void sceneAttributes(Input* input) override;
    void sceneStart() override;
    void sceneDestroy() override;
    void sceneMounted(Viewport* viewport) override;

    /// --- Sim Logic Here ---
    void iter_color(double ratio, uint8_t& r, uint8_t& g, uint8_t& b);

    double mandelbrot_basic(double x0, double y0);

    void cpu_mandelbrot(
        double fw, double fh,
        double wx, double wy,
        double ww, double wh,
        int pixel_count
    );

    void gpu_mandelbrot(
        double fw, double fh,
        double wx, double wy,
        double ww, double wh,
        int pixel_count
    );


    // --- Shaders ---
    GlContext context;
    ComputeShader shader;
    ShaderBuffer input_buffer, output_buffer;
    std::vector<float> input, output;

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
    void projectPrepare() override;
};

SIM_END(Mandelbrot)