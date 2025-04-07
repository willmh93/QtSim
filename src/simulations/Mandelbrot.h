#pragma once
#include "Project.h"
#include "Cardioid.h"

SIM_BEG(Mandelbrot)

/*double min_x = 10000;
double max_x = -10000;

if (xx - yy + x0 < min_x)
{
    min_x = xx - yy + x0;
}

if (xx - yy + x0 > max_x)
{
    max_x = xx - yy + x0;
}*/

template <bool smoothing, bool log_x, bool log_y>
inline double mandelbrot(double x0, double y0, int iter_lim)
{
    double x = 0.0, y = 0.0, xx = 0.0, yy = 0.0;
    int iter = 0;
    while (xx + yy <= 4.0 && iter < iter_lim)
    {
        if constexpr (log_y)
            y = log(2.0 * x * y + y0);
        else 
            y = (2.0 * x * y + y0);

        if constexpr (log_x)
            x = log(xx - yy + x0);
        else 
            x = (xx - yy + x0);


        xx = x * x;
        yy = y * y;
        iter++;
    }

    // Ensures black for deep-set points
    if (iter == iter_lim)
        return iter_lim;

    if constexpr (smoothing)
        return iter + (1.0 - log2(log2(xx + yy) / 2.0));
    else
        return iter;
}

template <bool smoothing, bool log_x, bool log_y>
inline double spline_mandelbrot(double x0, double y0, int iter_lim, float *x_spline, float* y_spline)
{
    double x = 0.0, y = 0.0, xx = 0.0, yy = 0.0;
    int iter = 0;
    while (xx + yy <= 4.0 && iter < iter_lim)
    {
        //y = 2.0 * x * y + y0;
        //x = xx - yy + x0;
        y = ImGui::BezierValueNormalize(2.0 * x * y + y0, -100.0f, 100.0f, x_spline);
        x = ImGui::BezierValueNormalize(xx - yy + x0, -100.0f, 100.0f, y_spline);

        xx = x * x;
        yy = y * y;
        iter++;
    }

    // Ensures black for deep-set points
    if (iter == iter_lim)
        return iter_lim;

    if constexpr (smoothing)
        return iter + (1.0 - log2(log2(xx + yy) / 2.0));
    else
        return iter;
}

struct Mandelbrot_Scene : public Scene
{
    /// --- Sim Variables ---
    //Bitmap *bmp = nullptr;
    CanvasBitmapObject bmp;

    QThreadPool pool;
    int thread_count = 1;
    bool gpu_compute = false;

    bool discrete_step = false;
    bool flatten_main_cardioid = false;

    double quality = 20.0;
    bool thresholding = false;
    bool smoothing = true;
    bool log_x = false;
    bool log_y = false;
    double cam_rotation = 0.0;
    double cam_zoom = 1;

    bool show_inside_main_cardioid = true;
    bool show_left_of_main_cardioid = true;
    bool interactive_cardioid = false;

    int iter_lim;

    Cardioid::CardioidLerper cardioid_lerper;
    double flatten_amount = 0.0;
    double cardioid_lerp_amount = 1.0; // 1 - flatten

    float x_spline[5] = { 0.0f, 0.0f, 1.0f, 1.0f, 0.0f };
    float y_spline[5] = { 0.0f, 0.0f, 1.0f, 1.0f, 0.0f };

    char config_buf[1024];

    // --- Scene management ---
    void sceneAttributes() override;
    void sceneStart() override;
    void sceneDestroy() override;
    void sceneMounted(Viewport* viewport) override;

    /// --- Sim Logic Here ---
    void step_color(double step, uint8_t& r, uint8_t& g, uint8_t& b);
    void iter_ratio_color(double ratio, uint8_t& r, uint8_t& g, uint8_t& b);

    std::string serializeConfig();
    void deserializeConfig(std::string json);

    void updateConfigBuffer();
    void loadConfigBuffer();

    //double mandelbrot_basic(double x0, double y0);
    
    
    /*template<typename Callback>
    void forEachWorldPixel(Viewport* ctx, Callback&& callback)
    {
        
        //FQuad bmp_quad_on_world

        //ctx->camera.toStageQuad(
        //FQuad quad = 
        //{
        //    { 0, 0 },
        //    { ctx->width, 0 },
        //    { ctx->width, ctx->height },
        //    { 0, ctx->height }
        //);
        //
        //const std::vector<ScanLineSegment> &lines = FillConvexQuadSegmentsMerged(quad.a, quad.b, quad.c, quad.d);
        //int a = 5;

        double fw = ctx->width;
        double fh = ctx->height;
        int iw = static_cast<int>(fw);
        int ih = static_cast<int>(fh);

        FRect stage_rect_on_world = ctx->camera.toWorldRect(0, 0, ctx->width, ctx->height);
        double wx0 = stage_rect_on_world.x1;
        double wy0 = stage_rect_on_world.y1;
        double wx1 = stage_rect_on_world.x2;
        double wy1 = stage_rect_on_world.y2;
        double ww = wx1 - wx0;
        double wh = wy1 - wy0;

        forEachPixelPos(iw, ih, thread_count, [&](int x, int y)
        {
            double rx = static_cast<double>(x) / fw;
            double ry = static_cast<double>(y) / fh;
            double wx = wx0 + (rx * ww);
            double wy = wy0 + (ry * wh);
            std::forward<Callback>(callback)(x, y, wx, wy);
        });
    }*/

    /*template<typename Callback>
    void forEachWorldPixel(Viewport* ctx, Bitmap *bmp, Callback&& callback)
    {
        // Goal:          For EVERY pixel in the bitmap, efficiently convert to world coordinate
        // Assumptions:   
        // 
        // Info needed: Assuming you loop over BITMAP pixels from start to last:
        // > World position of (0,0) to (w,0)  ->  Lerp between these world positions for source color of (x,y) pixel
        

            
    };*/

    /*template<
        bool a,
        int b,
        double c
    >
    void test()
    {
        if constexpr (a && b > 5 && c < 10.0)
        {

        }
    }*/

    template<
        bool smoothed,
        bool log_x,
        bool log_y,
        bool vis_main_cardioid,
        bool vis_left_of_main_cardioid
    >
    void regularMandelbrot(Viewport* ctx)
    {
        bool a = false;
        int b = 7;
        double c = 8.0;


        double f_max_iter = static_cast<double>(iter_lim);
        bmp.forEachWorldPixel(ctx, [&](int x, int y, double wx, double wy)
        {
            if constexpr (!vis_main_cardioid)
            {
                double tx, ty, ta, oa, d;
                Cardioid::cardioidPolarCoord(wx, wy, tx, ty, ta, d, oa);

                if (d < 0)
                {
                    bmp.setPixel(x, y, 127, 0, 0, 255);
                    return;
                }
            }

            if constexpr (!vis_left_of_main_cardioid)
            {
                //Vec2 mandel_pt = Cardioid::fromPolarCoordinate(perp_angle, -point_dist);
                if (wx < -0.75)
                {
                    bmp.setPixel(x, y, 127, 0, 0, 255);
                    return;
                }
            }

            double smooth_iter = mandelbrot<smoothed, log_x, log_y>(wx, wy, iter_lim);

            uint8_t r, g, b;
            if (discrete_step)
                step_color(smooth_iter /*+ 0.5*/, r, g, b);
            else
            {
                double ratio = smooth_iter / f_max_iter;
                iter_ratio_color(ratio, r, g, b);
            }

            bmp.setPixel(x, y, r, g, b, 255);
        });
    };

    template<
        bool smoothed,
        bool log_x,
        bool log_y,
        bool vis_main_cardioid,
        bool vis_left_of_main_cardioid
    >
    void splineMandelbrot(Viewport* ctx)
    {
        bool a = false;
        int b = 7;
        double c = 8.0;

        double f_max_iter = static_cast<double>(iter_lim);
        bmp.forEachWorldPixel(ctx, [&](int x, int y, double wx, double wy)
        {
            if constexpr (!vis_main_cardioid)
            {
                double tx, ty, ta, oa, d;
                Cardioid::cardioidPolarCoord(wx, wy, tx, ty, ta, d, oa);

                if (d < 0)
                {
                    bmp.setPixel(x, y, 127, 0, 0, 255);
                    return;
                }
            }

            if constexpr (!vis_left_of_main_cardioid)
            {
                //Vec2 mandel_pt = Cardioid::fromPolarCoordinate(perp_angle, -point_dist);
                if (wx < -0.75)
                {
                    bmp.setPixel(x, y, 127, 0, 0, 255);
                    return;
                }
            }

            double smooth_iter = spline_mandelbrot<smoothed, log_x, log_y>(wx, wy, iter_lim, x_spline, y_spline);

            uint8_t r, g, b;
            if (discrete_step)
                step_color(smooth_iter /*+ 0.5*/, r, g, b);
            else
            {
                double ratio = smooth_iter / f_max_iter;
                iter_ratio_color(ratio, r, g, b);
            }

            bmp.setPixel(x, y, r, g, b, 255);
        });
    };

    template<
        bool smoothed,
        bool log_x,
        bool log_y,
        bool vis_main_cardioid, 
        bool vis_left_of_main_cardioid
    >
    void radialMandelbrot(Viewport* ctx)
    {
        double f_max_iter = static_cast<double>(iter_lim);
        bmp.forEachWorldPixel(ctx, [&](int x, int y, double angle, double point_dist)
        {
            Vec2 polard_coord = cardioid_lerper.originalPolarCoordinate(angle, point_dist, cardioid_lerp_amount);

            if (polard_coord.y < 0)
            {
                bmp.setPixel(x, y, 0, 0, 0, 255);
                return;
            }

            Vec2 mandel_pt = Cardioid::fromPolarCoordinate(polard_coord.x, polard_coord.y);
            double recalculated_orig_angle = cardioid_lerper.originalPolarCoordinate(mandel_pt.x, mandel_pt.y, 1.0).x;

            bool hide =
                (polard_coord.x < M_PI && recalculated_orig_angle > M_PI*1.1) ||
                (polard_coord.x > M_PI && recalculated_orig_angle < M_PI*0.9);

            if (hide)
            {
                bmp.setPixel(x, y, 0, 0, 0, 255);
                return;
            }

            if constexpr (!vis_left_of_main_cardioid)
            {
                if (mandel_pt.x < -0.75)
                {
                    bmp.setPixel(x, y, 0, 0, 0, 255);
                    return;
                }
            }

            double smooth_iter = mandelbrot<smoothed, log_x, log_y>(mandel_pt.x, mandel_pt.y, iter_lim);
            double ratio = smooth_iter / f_max_iter;

            uint8_t r, g, b;
            iter_ratio_color(ratio, r, g, b);

            bmp.setPixel(x, y, r, g, b, 255);
        });
    };

    void cpu_mandelbrot(
        Viewport *ctx,
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