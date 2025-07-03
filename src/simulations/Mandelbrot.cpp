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

    static ImRect vr = { 0.0f, 0.8f, 0.8f, 0.0f };

    //x_spline.set(x_spline_points, 6, 1000);
    //if (!x_spline.initialized())
    //{
    //    x_spline = ImGui::Spline::fromEquation(-3, 3, 12, x_spline_points, [](float x)
    //    {
    //        return sin(x);
    //    });
    //}

    //memset(x_spline_points, 0, sizeof(x_spline_points));

    x_spline.set(x_spline_points, ImSpline::PointsArrSize(x_spline_points), 1000);
    
    //x_spline.fromEquation(0.01f, 5.0f, [](float x) {
    //    return logf(x);
    //    //return sin(x*3.0f)/2.0f;
    //}, 0.01f, 6, 100, 10);

    y_spline.set(y_spline_points, ImSpline::PointsArrSize(y_spline_points), 1000);
    
    //ImGui::SplineEditor("X Spline", &x_spline, &vr, 200);
    //ImGui::SameLine();
    //ImGui::SplineEditor("Y Spline", &y_spline, &vr, 200);

    ImSpline::SplineEditorPair("X/Y Spline", &x_spline, &y_spline, &vr, 900.0f);

    // CPU/GPU options
    ImGui::SeparatorText("Compute");
    //ImGui::SliderInt("Threads", &thread_count, 1, 32);
    //ImGui::Checkbox("GPU Compute (less options)", &gpu_compute);

    //ImGui::Checkbox("log (x) modifier", &log_x);
    //ImGui::Checkbox("log (y) modifier", &log_y);

    //if (!log_modifier)
    //    ImGui::Checkbox("Second Pass", &second_pass);

    ImGui::Checkbox("Discrete Step", &discrete_step);
    if (discrete_step)
        ImGui::DragDouble("Steps", &quality, 1000.0, 1.0, 1000000.0, "%.0f", ImGuiSliderFlags_Logarithmic);
    else
        ImGui::SliderDouble("Quality", &quality, 1.0, 150.0);

    ImGui::SeparatorText("Main Cardioid");
    if (!gpu_compute)
    {
        ImGui::Checkbox("Flatten", &flatten_main_cardioid);
        if (flatten_main_cardioid)
        {
            if (ImGui::SliderDouble("Flatness", &flatten_amount, 0.0, 1.0))
                cardioid_lerp_amount = 1.0 - flatten_amount;
        }

        //ImGui::Checkbox("Show Inside ", &show_inside_main_cardioid);
        ImGui::Checkbox("Show period-2 bulb", &show_period2_bulb);
    }

    ImGui::Checkbox("Interactive", &interactive_cardioid);


    ImGui::SeparatorText("Visual Options");
    ImGui::Checkbox("Thresholding", &thresholding);
    if (!thresholding)
        ImGui::Checkbox("Smoothing", &smoothing);

    ImGui::SeparatorText("View");
    ImGui::SliderDouble("Rotation", &cam_rotation, 0.0, M_PI * 2.0);

    // 1e16 = double limit before preicions loss
    //ImGui::DragDouble("Zoom X", &cam_zoom_x, cam_zoom_x / 100.0, 1.0, 1e16);
    //ImGui::DragDouble("Zoom Y", &cam_zoom_y, cam_zoom_y / 100.0, 1.0, 1e16);
    ImGui::DragDouble("Zoom Mult", &cam_zoom, cam_zoom / 100, 300.0, 1e16);
    ImGui::SliderDouble2("Zoom X/Y", cam_zoom_xy, 0.1, 10.0);

    // CPU only
    if (ImGui::InputTextMultiline("Config", config_buf, 1024))
    {
        loadConfigBuffer();
    }
}

std::string Mandelbrot_Scene::serializeConfig()
{
    QJsonObject info;
    info["x"] = camera->x;
    info["y"] = camera->y;
    info["quality"] = quality;
    info["discrete_step"] = discrete_step;
    info["zoom"] = cam_zoom;
    info["zoom_x"] = cam_zoom_xy[0];
    info["zoom_y"] = cam_zoom_xy[1];
    info["rotation"] = cam_rotation;
    info["smoothing"] = smoothing;
    info["spline_x"] = x_spline.serialize().c_str();
    info["spline_y"] = y_spline.serialize().c_str();

    QJsonDocument jsonDoc(info);
    return jsonDoc.toJson().toStdString();
}

void Mandelbrot_Scene::deserializeConfig(std::string json)
{
    QJsonDocument jsonDoc = QJsonDocument::fromJson(json.c_str());
    if (jsonDoc.isNull() || !jsonDoc.isObject()) return;
    QJsonObject info = jsonDoc.object();

    camera->x = info["x"].toDouble(0),
    camera->y = info["y"].toDouble(0);
    quality = info["quality"].toDouble(quality);
    discrete_step = info["discrete_step"].toBool(discrete_step);
    cam_zoom = info["zoom"].toDouble(1);
    cam_zoom_xy[0] = info["zoom_x"].toDouble(1);
    cam_zoom_xy[1] = info["zoom_y"].toDouble(1);
    cam_rotation = info["rotation"].toDouble(0);
    smoothing = info["smoothing"].toBool(smoothing);

    if (info.contains("spline_x")) 
        x_spline.deserialize(info["spline_x"].toString().toStdString());
    if (info.contains("spline_y")) 
        y_spline.deserialize(info["spline_y"].toString().toStdString());
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
        ratio = fmod(step / 10.0, 1.0);

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
    Viewport* ctx,
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
        /*dispatchBooleans(
            boolsTemplate(regularMandelbrot, [&], ctx),
            smoothing,
            log_x, log_y,
            show_inside_main_cardioid,
            show_period2_bulb
        );*/

        bool linear = false;// x_spline.isSimpleLinear() && y_spline.isSimpleLinear();

        dispatchBooleans(
            boolsTemplate(regularMandelbrot, [&], ctx),
            smoothing,
            linear,
            show_period2_bulb
        );
    }
    else
    {
        // Flat lerp
        dispatchBooleans(
            boolsTemplate(radialMandelbrot, [&], ctx),
            smoothing,
            show_period2_bulb
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
        //show_inside_main_cardioid,
        show_period2_bulb,
        cardioid_lerp_amount,
        //log_x, log_y,
        x_spline.hash(),
        y_spline.hash()
        //second_pass
        )
        )
    {
        bmp.setNeedsReshading();
    }

    camera->rotation = cam_rotation;
    camera->setZoomX(cam_zoom * cam_zoom_xy[0]);
    camera->setZoomY(cam_zoom * cam_zoom_xy[1]);

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
            //if (!log_x && !log_y)
            //    Cardioid::plot(this, ctx, true);
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
    //ctx->print() << "\nLinear: " << x_spline.isLinear();

    double zoom_factor = camera->getRelativeZoomFactor().average();
    ctx->print() << "\nZoom: " << QString::asprintf("%.1f", zoom_factor) << "x";
}

/// User Interaction

void Mandelbrot_Scene::mouseDown() {}
void Mandelbrot_Scene::mouseUp() {}
void Mandelbrot_Scene::mouseMove() {}
void Mandelbrot_Scene::mouseWheel() {}

SIM_END(Mandelbrot)