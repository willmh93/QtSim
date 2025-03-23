#include "Mandelbrot.h"
#include "Cardioid.h"

SIM_DECLARE(Mandelbrot, "Mandelbrot", "Mandelbrot Viewer")

/// Project ///

void Mandelbrot_Project::projectPrepare()
{
    auto& layout = newLayout();
    create<Mandelbrot_Scene>()->mountTo(layout);
}

/// Scene ///

void Mandelbrot_Scene::sceneAttributes(Input* input)
{
    thread_count = (int)(1.5 * ((double)QThread::idealThreadCount()));

    // CPU/GPU options
    ImGui::SliderInt("Thread Count", &thread_count, 1, 32);
    ImGui::Checkbox("GPU Compute", &gpu_compute);

    // Mandelbrot options
    ImGui::Checkbox("Discrete Step", &discrete_step);
    ImGui::Checkbox("Radial", &radial_mandelbrot);


    ImGui::SliderDouble("Mandelbrot Quality", &quality, 1.0, 150.0);
    ImGui::Checkbox("Smoothing", &smoothing);

    // CPU only
    ImGui::Checkbox("Thresholding", &thresholding);
    ImGui::SliderDouble("Threshold", &threshold, 0, 1000);

    //ImGui::SliderDouble("Real", &camera_x, -2.0, 1);
    //ImGui::SliderDouble("Imaginary", &camera_y, -1, 1);

    // CPU/GPU options
    /*input->realtime_slider("Thread Count", &thread_count, 1, 32);
    input->realtime_checkbox("GPU Compute", &gpu_compute);

    // Mandelbrot options
    input->realtime_checkbox("Discrete Step", &discrete_step);
    input->realtime_slider("Mandelbrot Quality", &quality, 1.0, 150.0);
    input->realtime_checkbox("Smoothing", &smoothing);

    // CPU only
    input->realtime_checkbox("Thresholding", &thresholding);
    input->realtime_slider("Threshold", &threshold, 0, 1000);

    input->realtime_float("Real", &camera_x, -2.0, 1, 0.001); // updated in realtime
    input->realtime_float("Imaginary", &camera_y, -1, 1, 0.001); // updated in realtime*/
    
}

void Mandelbrot_Scene::sceneStart()
{
    /// Initialize Scene
    pool.setMaxThreadCount(thread_count);

}

void Mandelbrot_Scene::sceneDestroy()
{
    /// Destroy Scene
}

void Mandelbrot_Scene::sceneMounted(Viewport* viewport)
{
    /// Initialize viewport (after sceneStart)
    camera->setOriginViewportAnchor(Anchor::CENTER);
    camera->focusWorldRect(-2, -1, 2, 1);
    
    context.setup();

    QString shader_src = R"(
        #version 430

        uniform int MAX_ITER = 50;
        uniform float BAILOUT = 4.0;
        uniform bool SMOOTH = false;

        layout(std430, binding = 0) buffer readonly Input_Buf  { float input_buf[]; }; // Interpreted as (real, imag) pairs
        layout(std430, binding = 1) buffer Output_Buf { float output_buf[]; }; // Stores normalized iteration count
        layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

        void main(void)
        {
            uint id = gl_GlobalInvocationID.x;
            uint idx = id * 2;

            float x0 = input_buf[idx];      // real
            float y0 = input_buf[idx + 1];  // imaginary
            float x = 0.0, y = 0.0;
            float xx = 0, yy = 0;

            int i = 0;
            while (xx + yy <= BAILOUT && i < MAX_ITER)
            {
                y = 2.0 * x * y + y0;
                x = xx - yy + x0;
                xx = x * x;
                yy = y * y;
                i++;
            }

            float result = float(i);
            float esc = (i < MAX_ITER) ? 1.0 : 0.0;
            output_buf[id] = esc * (SMOOTH ? (float(i) + (1.0-log2(log2(xx+yy)/2.0))) : float(i)) + (1.0-esc) * float(MAX_ITER);
        }
    )";

    shader.setup_from_source(shader_src, &context);
}

inline double Mandelbrot_Scene::mandelbrot_basic(double x0, double y0)
{
    double x = 0.0, y = 0.0, xx = 0.0, yy = 0.0;
    int iter = 0;
    while (xx + yy <= 4.0 && iter < iter_lim)
    {
        y = 2.0 * x * y + y0;
        x = xx - yy + x0;
        xx = x * x;
        yy = y * y;
        iter++;
    }

    // Ensures black for deep-set points
    if (iter == iter_lim)
        return iter_lim;

    if (smoothing)
        return iter + (1.0 - log2(log2(xx + yy) / 2.0));
    else
        return iter;
}

void Mandelbrot_Scene::iter_color(double ratio, uint8_t& r, uint8_t& g, uint8_t& b)
{
    r = (uint8_t)(9 * (1 - ratio) * ratio * ratio * ratio * 255);
    g = (uint8_t)(15 * (1 - ratio) * (1 - ratio) * ratio * ratio * 255);
    b = (uint8_t)(8.5 * (1 - ratio) * (1 - ratio) * (1 - ratio) * ratio * 255);
}

void Mandelbrot_Scene::cpu_mandelbrot(
    double fw, double fh, 
    double wx0, double wy0,
    double ww, double wh,
    int pixel_count)
{
    int iw = static_cast<int>(fw);
    double f_max_iter = static_cast<double>(iter_lim);

    std::vector<QFuture<void>> futures(thread_count);

    auto pixel_ranges = splitRanges(pixel_count, thread_count);
    if (!radial_mandelbrot)
    {
        for (int ti = 0; ti < thread_count; ti++)
        {
            auto& pixel_range = pixel_ranges[ti];
            futures[ti] = QtConcurrent::run([&]()
            {
                uint8_t r, g, b;
                for (int i = pixel_range.first; i < pixel_range.second; i++)
                {
                    int x = (i % iw), y = (i / iw);
                    double rx = static_cast<double>(x) / fw;
                    double ry = static_cast<double>(y) / fh;
                    double wx = wx0 + (rx * ww);
                    double wy = wy0 + (ry * wh);
                    double smooth_iter = mandelbrot_basic(wx, wy);
                    double ratio = smooth_iter / f_max_iter;

                    if (thresholding)
                        r = g = b = (ratio <= 0.9999) ? 127 : 0;
                    else
                        iter_color(ratio, r, g, b);

                    bmp->setPixel(x, y, r, g, b, 255);
                }
            });
        }
    }
    else
    {
        // Radial
        for (int ti = 0; ti < thread_count; ti++)
        {
            auto& pixel_range = pixel_ranges[ti];
            futures[ti] = QtConcurrent::run([&]()
            {
                uint8_t r, g, b;
                for (int i = pixel_range.first; i < pixel_range.second; i++)
                {
                    int x = (i % iw), y = (i / iw);
                    double rx = static_cast<double>(x) / fw;
                    double ry = static_cast<double>(y) / fh;
                    double wx = wx0 + (rx * ww);
                    double wy = wy0 + (ry * wh);

                    double perp_angle = wx;// rx* (2.0 * M_PI);
                    double point_dist = wy;// -0.75 + (1.25 * ry);
                    //double 
                    //double mandelbrot_x = 0.5 * cos(tangent_angle) - 0.25 * cos(tangent_angle * 2.0);
                    //double mandelbrot_y = 0.5 * sin(tangent_angle) - 0.25 * sin(tangent_angle * 2.0);

                    Vec2 mandel_pt = Cardioid::fromCardioidTransform(perp_angle, -point_dist);

                    //if (mandel_pt.x > -0.75)
                    {
                        double smooth_iter = mandelbrot_basic(mandel_pt.x, mandel_pt.y);
                        double ratio = smooth_iter / f_max_iter;

                        if (thresholding)
                            r = g = b = (ratio <= 0.9999) ? 127 : 0;
                        else
                            iter_color(ratio, r, g, b);


                        bmp->setPixel(x, y, r, g, b, 255);
                    }

                    //double wx = wx0 + (rx * ww);
                    //double wy = wy0 + (ry * wh);
                    //
                    //double tangent_angle;
                    //double dist;
                    //Cardioid::cardioidPolarCoord(wx, wy, tangent_x, tangent_y, tangent_angle, dist);


                    /*double smooth_iter = mandelbrot_basic(angle, dist);
                    double ratio = smooth_iter / f_max_iter;

                    if (thresholding)
                        r = g = b = (ratio <= 0.9999) ? 127 : 0;
                    else
                        iter_color(ratio, r, g, b);

                    bmp->setPixel(x, y, r, g, b, 255);*/
                }
            });
        }
    }

    for (auto& future : futures)
        future.waitForFinished();
}

void Mandelbrot_Scene::gpu_mandelbrot(
    double fw, double fh, 
    double wx0, double wy0, 
    double ww, double wh, 
    int pixel_count)
{
    int iw = static_cast<int>(fw);
    double f_max_iter = static_cast<double>(iter_lim);

    for (int i = 0; i < pixel_count; i++)
    {
        int x = (i % iw);
        int y = (i / iw);

        double fx = static_cast<double>(x);
        double fy = static_cast<double>(y);
        double rx = fx / fw;
        double ry = fy / fh;
        double wx = wx0 + (rx * ww);
        double wy = wy0 + (ry * wh);

        input[i * 2] = wx;
        input[i * 2 + 1] = wy;
    }

    input_buffer.setup(&context);
    output_buffer.setup(&context);
    input_buffer.allocate(&input[0], sizeof(float) * pixel_count * 2);
    output_buffer.allocate(&output[0], sizeof(float) * pixel_count);

    // Bind buffers to shader
    input_buffer.bind_for_shader(0);
    output_buffer.bind_for_shader(1);

    // Compute
    shader.begin();
    shader.program().setUniformValue("MAX_ITER", iter_lim);
    shader.program().setUniformValue("BAILOUT", 4.0f);
    shader.program().setUniformValue("SMOOTH", smoothing);
    shader.compute(pixel_count);
    shader.end();

    // Read GPU output to CPU buffer
    output_buffer.read_to_cpu(&output[0], sizeof(float) * pixel_count);

    // Display results
    uint8_t r, g, b;
    for (int i = 0; i < pixel_count; i++)
    {
        int x = (i % iw);
        int y = (i / iw);
        double ratio = output[i] / f_max_iter;

        if (thresholding)
            r = g = b = (ratio <= 0.9999) ? 127 : 0;
        else
            iter_color(ratio, r, g, b);

        bmp->setPixel(x, y, r, g, b, 255);
    }
}

void Mandelbrot_Scene::viewportProcess(Viewport* ctx)
{
    //camera->targ_pan_x = -camera_x + camera->x;
    //camera->targ_pan_y = -camera_y + camera->y;

    /// Process Viewports running this Scene
    double fw = ctx->width;
    double fh = ctx->height;
    int iw = static_cast<int>(fw);
    int ih = static_cast<int>(fh);
    int pixel_count = iw * ih;
    double zoom = camera->zoom_x;

    if (ctx->resized())
    {
        if (bmp)
            delete bmp;

        bmp = new Bitmap();
        bmp->create(iw, ih);

        // Allocate input/output buffers for GPU
        input.resize(pixel_count * 2);
        output.resize(pixel_count);
    }

    FRect stage_rect_on_world = ctx->camera.toWorldRect(0, 0, ctx->width, ctx->height);
    double wx1 = stage_rect_on_world.x1;
    double wx2 = stage_rect_on_world.x2;
    double wy1 = stage_rect_on_world.y1;
    double wy2 = stage_rect_on_world.y2;
    double ww = wx2 - wx1;
    double wh = wy2 - wy1;

    if (discrete_step)
        iter_lim = static_cast<int>(quality);
    else
        iter_lim = std::log2(zoom) * quality;

    double f_max_iter = static_cast<double>(iter_lim);
    double threshold_sq = threshold * threshold;

    if (gpu_compute)
        gpu_mandelbrot(fw, fh, wx1, wy1, ww, wh, pixel_count);
    else
        cpu_mandelbrot(fw, fh, wx1, wy1, ww, wh, pixel_count);
}

void Mandelbrot_Scene::viewportDraw(Viewport* ctx)
{
    /// Draw Scene to Viewport
    camera->stageTransform();
    ctx->drawSurface(bmp, 0, 0, ctx->width, ctx->height);
    ctx->drawWorldAxis(0.5, 0, 0.5);

    ctx->print() << "Frame delta time: " << this->project_dt(10) << " ms\n";
    ctx->print() << "Max Iterations: " << iter_lim;
}

/// User Interaction

void Mandelbrot_Scene::mouseDown() {}
void Mandelbrot_Scene::mouseUp() {}
void Mandelbrot_Scene::mouseMove() {}
void Mandelbrot_Scene::mouseWheel() {}

SIM_END(Mandelbrot)