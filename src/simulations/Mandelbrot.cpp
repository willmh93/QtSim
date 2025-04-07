#include "Mandelbrot.h"


SIM_DECLARE(Mandelbrot, "Fractal", "Mandelbrot Viewer")

/// Project ///

void Mandelbrot_Project::projectPrepare()
{
    auto& layout = newLayout();
    create<Mandelbrot_Scene>()->mountTo(layout);
}

/// Scene ///

void Mandelbrot_Scene::sceneAttributes()
{
    thread_count = (int)(1.5 * ((double)QThread::idealThreadCount()));

    //ImGui::Spline("Test Spline", x_spline);

    // CPU/GPU options
    ImGui::SeparatorText("Compute");
    ImGui::SliderInt("Thread Count", &thread_count, 1, 32);
    ImGui::Checkbox("GPU Compute (less options)", &gpu_compute);
    ImGui::Checkbox("Discrete Step", &discrete_step);
    
    ImGui::Checkbox("log (x) modifier", &log_x);
    ImGui::Checkbox("log (y) modifier", &log_y);

    //if (!log_modifier)
    //    ImGui::Checkbox("Second Pass", &second_pass);

    if (discrete_step)
        ImGui::DragDouble("Mandelbrot Steps", &quality, 1000.0, 1.0, 1000000.0, "%.0f", ImGuiSliderFlags_Logarithmic);
    else
        ImGui::SliderDouble("Mandelbrot Quality", &quality, 1.0, 150.0);

    ImGui::SeparatorText("Main Cardioid");
    if (!gpu_compute)
    {
        ImGui::Checkbox("Flatten", &flatten_main_cardioid);
        if (flatten_main_cardioid)
        {

            if (ImGui::SliderDouble("Flatness", &flatten_amount, 0.0, 1.0))
                cardioid_lerp_amount = 1.0 - flatten_amount;
        }

        ImGui::Checkbox("Show Inside ", &show_inside_main_cardioid);
        ImGui::Checkbox("Show Left", &show_left_of_main_cardioid);
    }

    ImGui::Checkbox("Interactive", &interactive_cardioid);


    ImGui::SeparatorText("Visual Options");
    ImGui::Checkbox("Smoothing", &smoothing);

    ImGui::SliderDouble("Rotation", &cam_rotation, 0.0, M_PI * 2.0);

    // 1e16 = double limit before preicions loss
    ImGui::DragDouble("Zoom", &cam_zoom, cam_zoom/100, 300.0, 1e16); 

    // CPU only
    ImGui::Checkbox("Thresholding", &thresholding);

    if (ImGui::InputTextMultiline("Config", config_buf, 1024))
    {
        loadConfigBuffer();
    }

    //static float v[5] = { 0.390f, 0.575f, 0.565f, 1.000f };
    //ImGui::Bezier("X-Spline", x_spline);       // draw
    //ImGui::Bezier("Y-Spline", y_spline);       // draw

    
    /*static float points[] = {
        0.0f, 0.0f,
        0.1f, 0.1f,
        0.2f, 0.2f,
        0.3f, 0.3f,
        0.4f, 0.4f,
        0.5f, 0.5f,
        0.6f, 0.6f,
        0.7f, 0.7f,
        0.8f, 0.8f,
    };
    int new_count = 3;
    ImGui::CurveEditor("Test Spline", points, 3, 
        {300, 300},
        0,
        &new_count);*/

    //float min = -3.0f;
    //float max = 3.0f;
    //float a = ImGui::BezierValueNormalize(0.0f, min, max, x_spline);
    //float b = ImGui::BezierValueNormalize(0.5f, min, max, x_spline);
    //float c = ImGui::BezierValueNormalize(1.0f, min, max, x_spline);
    //float d = ImGui::BezierValueNormalize(2.0f, min, max, x_spline);
    //float e = ImGui::BezierValueNormalize(-1.0f, min, max, x_spline);
    //float f = ImGui::BezierValueNormalize(-2.0f, min, max, x_spline);
    //int j = 5;
}

std::string Mandelbrot_Scene::serializeConfig()
{
    QJsonObject info;
    info["x"]             = camera->x;
    info["y"]             = camera->y;
    info["quality"]       = quality;
    info["discrete_step"] = discrete_step;
    info["zoom"]          = cam_zoom;
    info["rotation"]      = cam_rotation;
    info["log_x"]         = log_x;
    info["log_y"]         = log_y;
    info["smoothing"]     = smoothing;

    QJsonDocument jsonDoc(info);
    return jsonDoc.toJson().toStdString();
}

void Mandelbrot_Scene::deserializeConfig(std::string json)
{
    QJsonDocument jsonDoc = QJsonDocument::fromJson(json.c_str());
    if (jsonDoc.isNull() || !jsonDoc.isObject()) return;
    QJsonObject info = jsonDoc.object();

    camera->x     = info["x"].toDouble(), 
    camera->y     = info["y"].toDouble();
    quality       = info["quality"].toDouble(quality);
    discrete_step = info["discrete_step"].toBool(discrete_step);
    cam_zoom      = info["zoom"].toDouble();
    cam_rotation  = info["rotation"].toDouble();
    log_x         = info["log_x"].toBool(log_x);
    log_y         = info["log_y"].toBool(log_y);
    smoothing     = info["smoothing"].toBool(smoothing);
}

void Mandelbrot_Scene::updateConfigBuffer()
{
    strcpy_s(config_buf, serializeConfig().c_str());
}

void Mandelbrot_Scene::loadConfigBuffer()
{
    deserializeConfig(config_buf);
}

void Mandelbrot_Scene::sceneStart()
{
    /// Initialize Scene
    pool.setMaxThreadCount(thread_count);
    ///camera->setRelativeZoomRange(0.1, 1e300);

    cardioid_lerper.create((2.0 * M_PI) / 5760, 0.005);
}

void Mandelbrot_Scene::sceneDestroy()
{
    /// Destroy Scene
}

void Mandelbrot_Scene::sceneMounted(Viewport* viewport)
{
    /// Initialize viewport (after sceneStart)
    camera->setOriginViewportAnchor(Anchor::CENTER);
    camera->setPanningUsesOffset(false);
    camera->focusWorldRect(-2, -1, 2, 1);
    cam_zoom = camera->zoom_x;
    //camera->zoom_y = -camera->zoom_y;
    //camera->targ_zoom_y = -camera->targ_zoom_y;
    
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

void Mandelbrot_Scene::step_color(double step, uint8_t& r, uint8_t& g, uint8_t& b)
{
    double ratio;
    if (smoothing)
        ratio = fmod(step, 1.0);
    else
        ratio = fmod(step/10.0, 1.0);

    r = (uint8_t)(9 * (1 - ratio) * ratio * ratio * ratio * 255);
    g = (uint8_t)(15 * (1 - ratio) * (1 - ratio) * ratio * ratio * 255);
    b = (uint8_t)(8.5 * (1 - ratio) * (1 - ratio) * (1 - ratio) * ratio * 255);
}

void Mandelbrot_Scene::iter_ratio_color(double ratio, uint8_t& r, uint8_t& g, uint8_t& b)
{
    if (thresholding)
    {
        r = g = b = (ratio <= 0.9999) ? 0 : 127;
        return;
    }
    r = (uint8_t)(9 * (1 - ratio) * ratio * ratio * ratio * 255);
    g = (uint8_t)(15 * (1 - ratio) * (1 - ratio) * ratio * ratio * 255);
    b = (uint8_t)(8.5 * (1 - ratio) * (1 - ratio) * (1 - ratio) * ratio * 255);
}



void Mandelbrot_Scene::cpu_mandelbrot(
    Viewport *ctx,
    double fw, double fh, 
    double wx0, double wy0,
    double ww, double wh,
    int pixel_count)
{
    int iw = static_cast<int>(fw);
    int ih = static_cast<int>(fw);
    double f_max_iter = static_cast<double>(iter_lim);

    
    if (!flatten_main_cardioid)
    {
        // Standard
        dispatchBooleans(
            boolsTemplate(regularMandelbrot, [&], ctx),
            smoothing,
            log_x, log_y,
            show_inside_main_cardioid,
            show_left_of_main_cardioid
        );

        /*dispatchBooleans(
            boolsTemplate(splineMandelbrot, [&], ctx),
            smoothing,
            log_x, log_y,
            show_inside_main_cardioid,
            show_left_of_main_cardioid
        );*/
    }
    else
    {
        // Flat lerp
        dispatchBooleans(
            boolsTemplate(radialMandelbrot, [&], ctx),
            smoothing,
            log_x, log_y,
            show_inside_main_cardioid,
            show_left_of_main_cardioid
        );
    }
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
        {
            iter_ratio_color(ratio, r, g, b);
        }

        bmp.setPixel(x, y, r, g, b, 255);
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

    if (anyChanged(
            quality, 
            smoothing,
            thresholding, 
            discrete_step,
            flatten_main_cardioid, 
            show_inside_main_cardioid,
            show_left_of_main_cardioid,
            cardioid_lerp_amount,
            log_x, log_y
            //second_pass
            )
        )
    {
        bmp.setNeedsReshading();
    }

    camera->rotation = cam_rotation;
    camera->setZoom(cam_zoom);

    bmp.setStageRect(0, 0, ctx->width, ctx->height);
    bmp.setBitmapSize(iw, ih);

    if (bmp.needsReshading(ctx))
    {
        updateConfigBuffer();

        if (gpu_compute)
            gpu_mandelbrot(fw, fh, wx1, wy1, ww, wh, pixel_count);
        else
            cpu_mandelbrot(ctx, fw, fh, wx1, wy1, ww, wh, pixel_count);
    }
}

void Mandelbrot_Scene::viewportDraw(Viewport* ctx)
{
    /// Draw Scene to Viewport
    camera->stageTransform();
    ctx->drawSurface(bmp);
    ctx->drawWorldAxis(0.5, 0, 0.5);

    if (interactive_cardioid)
    {
        if (!flatten_main_cardioid)
        {
            // Regular interactive Mandelbrot
            if (!log_x && !log_y)
                Cardioid::plot(this, ctx, true);
        }
        else
        {
            // Lerped/Flattened Mandelbrot
            camera->scalingLines(false);
            ctx->setLineWidth(1);
            ctx->beginPath();
            ctx->drawPath(cardioid_lerper.lerped(cardioid_lerp_amount));
            ctx->stroke();
        }
    }

    ctx->print() << "Frame delta time: " << this->project_dt(10) << " ms";
    ctx->print() << "\nMax Iterations: " << iter_lim;

    double zoom_factor = camera->getRelativeZoomFactor().average();
    ctx->print() << "\nZoom: " << QString::asprintf("%.1f", zoom_factor) << "x";
}

/// User Interaction

void Mandelbrot_Scene::mouseDown() {}
void Mandelbrot_Scene::mouseUp() {}
void Mandelbrot_Scene::mouseMove() {}
void Mandelbrot_Scene::mouseWheel() {}

SIM_END(Mandelbrot)