#include "Mandelbrot.h"
SIM_DECLARE(Mandelbrot, "My Projects", "Mandelbrot")

/// Project ///

void Mandelbrot_Project::projectAttributes(Input* input)
{
}

void Mandelbrot_Project::projectPrepare()
{
    auto& layout = newLayout();
    create<Mandelbrot_Scene>()->mountTo(layout);
}

/// Scene ///

void Mandelbrot_Scene::sceneAttributes(Input* input)
{
    thread_count = (int)(1.5 * ((double)QThread::idealThreadCount()));

    input->realtime_slider("Thread Count", &thread_count, 1, 32);
    input->realtime_checkbox("GPU Compute", &gpu_compute);

    input->realtime_slider("Mandelbrot Quality", &quality, 1.0, 100.0);
    input->realtime_slider("Threshold", &threshold, 0, 1000);
    input->realtime_slider("Y Multiply", &y_mult, 0.1, 8);
    input->realtime_slider("Exponent", &exponent, 1, 8);
    input->realtime_slider("Version", &version, 0, 2);
    input->realtime_checkbox("Smoothing", &smoothing);

    input->realtime_slider("Real", &camera_x, -2.0, 1, 0.0000001); // updated in realtime
    input->realtime_slider("Imaginary", &camera_y, -1, 1, 0.0000001); // updated in realtime
    
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
    camera->focusWorldRect(-2, -1, 1, 1);
    
    context.setup();

    /*QString shader_src = R"(
        #version 430
        uniform float coeff = 1;  

        layout(std430, binding = 0) buffer Input_Buf  { float input_buf[]; };
        layout(std430, binding = 1) buffer Output_Buf { float output_buf[]; };
        layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

        void main(void)
        {
            uint id = gl_GlobalInvocationID.x;
            float value = input_buf[id];
            value += id*coeff;
            output_buf[id] = value;
        }
    )";*/

    QString shader_src = R"(
        #version 430

        uniform int MAX_ITER = 50; // Maximum Mandelbrot iterations
        uniform float BAILOUT = 4.0; // Escape threshold

        layout(std430, binding = 0) buffer Input_Buf  { float input_buf[]; }; // Interpreted as (real, imag) pairs
        layout(std430, binding = 1) buffer Output_Buf { float output_buf[]; }; // Stores normalized iteration count

        layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

        void main(void)
        {
            uint id = gl_GlobalInvocationID.x;
            uint idx = id * 2; // Each complex number is stored as two consecutive floats

            // Read complex coordinate (real, imag)
            float cr = input_buf[idx];
            float ci = input_buf[idx + 1];

            // Initialize (z_r, z_i) = (0, 0)
            float zr = 0.0;
            float zi = 0.0;

            int iter = 0;
            for (; iter < MAX_ITER; iter++) {
                float zr2 = zr * zr;
                float zi2 = zi * zi;
        
                if (zr2 + zi2 > BAILOUT) break; // Escape condition

                // Compute z = z^2 + c
                float temp = zr2 - zi2 + cr;
                zi = 2.0 * zr * zi + ci;
                zr = temp;
            }

            // Normalize iteration count to range [0, 1] for visualization
            output_buf[id] = float(iter) / float(MAX_ITER);
        }
    )";

    shader.setup_from_source(shader_src, &context);
}

inline double Mandelbrot_Scene::mandelbrot_basic(double x0, double y0, int max_iter)
{
    double x = 0.0, y = 0.0;
    int iter = 0;
    while (x * x + y * y <= 4.0 && iter < max_iter)
    {
        double xtemp = x * x - y * y + x0;
        y = 2.0 * x * y + y0;
        x = xtemp;
        iter++;
    }

    // Ensures black for deep-set points
    if (iter == max_iter)
        return max_iter;

    double log_zn = log2(x * x + y * y) / 2.0;
    return iter + (smoothing ? (1 - log2(log_zn)) : 0);
}

inline double Mandelbrot_Scene::mandelbrot_2(double x0, double y0, int max_iter)
{
    double x = 0.0, y = 0.0;
    int iter = 0;
    while (x * x + y * y <= 4.0 && iter < max_iter)
    {
        double xtemp = x * x - y * y + x0;
        y = 2.0 * x * y + y0;
        x = xtemp;
        iter++;
    }

    // Ensures black for deep-set points
    if (iter == max_iter)
        return max_iter;

    //double log_zn = log2(x * x + y * y) / 2.0;
    return (iter > threshold) ? max_iter : 50;
}

inline void Mandelbrot_Scene::mandelbrot_3(double x0, double y0, int max_iter, Vec2 &p, double &smooth_iter)
{
    double x = 0.0, y = 0.0;
    int iter = 0;
    while (x * x + y * y <= 4.0 && iter < max_iter)
    {
        double xtemp = x * x - y * y + x0;
        y = 2.0 * x * y + y0;
        x = xtemp;
        iter++;
    }

    double log_zn = log2(x * x + y * y) / 2.0;
    smooth_iter = iter + (smoothing ? (1 - log2(log_zn)) : 0);
    p.x = x;
    p.y = y;
}


inline std::tuple<uint8_t, uint8_t, uint8_t> Mandelbrot_Scene::smooth_color(double iter, int max_iter)
{
    double t = iter / max_iter; 

    // Smooth color transitions using HSV-like mapping
    uint8_t r = (uint8_t)(9 * (1 - t) * t * t * t * 255);
    uint8_t g = (uint8_t)(15 * (1 - t) * (1 - t) * t * t * 255);
    uint8_t b = (uint8_t)(8.5 * (1 - t) * (1 - t) * (1 - t) * t * 255);

    return { r, g, b };
}

inline std::tuple<uint8_t, uint8_t, uint8_t> Mandelbrot_Scene::ratio_color(double ratio)
{
    // Smooth color transitions using HSV-like mapping
    uint8_t r = (uint8_t)(9 * (1 - ratio) * ratio * ratio * ratio * 255);
    uint8_t g = (uint8_t)(15 * (1 - ratio) * (1 - ratio) * ratio * ratio * 255);
    uint8_t b = (uint8_t)(8.5 * (1 - ratio) * (1 - ratio) * (1 - ratio) * ratio * 255);

    return { r, g, b };
}

void Mandelbrot_Scene::sceneProcess()
{
    /// Process Scene update
}

void Mandelbrot_Scene::loadShaders()
{
    /*shader = std::make_unique<QOpenGLShaderProgram>();
    shader->addShaderFromSourceCode(QOpenGLShader::Vertex, R"(
        #version 330 core
        layout (location = 0) in vec2 pos;
        out vec2 fragCoord;
        void main() {
            fragCoord = pos;
            gl_Position = vec4(pos, 0.0, 1.0);
        }
    )");

    // Fragment Shader (Computes Mandelbrot Set)
    shader->addShaderFromSourceCode(QOpenGLShader::Fragment, R"(
        #version 330 core
        in vec2 fragCoord;
        out vec4 color;
        uniform float zoom;
        uniform float offsetX, offsetY;
        const int maxIter = 1000;

        void main() {
            vec2 c = vec2(fragCoord.x * zoom + offsetX, fragCoord.y * zoom + offsetY);
            vec2 z = vec2(0.0);
            int iter = 0;
            while (dot(z, z) < 4.0 && iter < maxIter) {
                z = vec2(z.x*z.x - z.y*z.y, 2.0 * z.x * z.y) + c;
                iter++;
            }
            float norm = float(iter) / maxIter;
            color = vec4(norm, norm * 0.5, norm * 0.2, 1.0);
        }
    )");

    if (!shader->link())
    {
        qDebug() << "Shader linking failed:" << shader->log();
        return;
    }*/
}

void Mandelbrot_Scene::viewportProcess(Viewport* ctx)
{
    camera->x = camera_x;
    camera->y = camera_y;

    /// Process Viewports running this Scene
    double fw = ctx->width;
    double fh = ctx->height;
    int iw = static_cast<int>(ctx->width);
    int ih = static_cast<int>(ctx->height);
    int  pixel_count = iw * ih;
    double zoom = camera->zoom_x;

    if (ctx->resized())
    {
        bmp.create(iw, ih, false);
        input.resize(pixel_count * 2);
        output.resize(pixel_count);
    }

    FRect stage_rect_on_world = ctx->camera.toWorldRect(FRect(0, 0, ctx->width, ctx->height));
    double wx1 = stage_rect_on_world.x1;
    double wx2 = stage_rect_on_world.x2;
    double wy1 = stage_rect_on_world.y1;
    double wy2 = stage_rect_on_world.y2;
    double ww = wx2 - wx1;
    double wh = wy2 - wy1;


    int max_iter = std::log2(zoom) * quality;
    double threshold_sq = threshold * threshold;

    if (!gpu_compute)
    {
        std::vector<QFuture<void>> futures(thread_count);

        auto pixel_ranges = splitRanges(pixel_count, thread_count);
        for (int ti = 0; ti < thread_count; ti++)
        {
            auto& pixel_range = pixel_ranges[ti];
            futures[ti] = QtConcurrent::run([this, ti, pixel_range, iw, ih, fw, fh, wx1, wy1, ww, wh, max_iter, threshold_sq]()
            {
                for (int i = pixel_range.first; i < pixel_range.second; i++)
                {
                    int x = (i % iw);
                    int y = (i / iw);
                    double fx = static_cast<double>(x);
                    double fy = static_cast<double>(y);
                    double rx = fx / fw;
                    double ry = fy / fh;
                    double wx = wx1 + (rx * ww);
                    double wy = wy1 + (ry * wh);
                    double smooth_iter;

                    switch (version)
                    {
                        case 0:
                        {
                            smooth_iter = mandelbrot_basic(wx, wy, max_iter);
                            auto [r, g, b] = smooth_color(smooth_iter, max_iter);
                            bmp.setPixel(x, y, r, g, b, 255);
                        }
                        break;

                        case 1:
                        {

                            smooth_iter = mandelbrot_2(wx, wy, max_iter);
                            auto [r, g, b] = smooth_color(smooth_iter, max_iter);
                            bmp.setPixel(x, y, r, g, b, 255);
                        }
                        break;
                    }
                }
            });
        }

        for (auto& future : futures)
            future.waitForFinished();
    }
    else // GPU
    {
        ///const int N = 23; // Number of coordinate pairs (not total floats!)
        ///float input[N * 2]; // Each complex number is represented by two floats
        ///float output[N];    // Each coordinate pair produces a single iteration count

        // Generate Mandelbrot coordinates in input buffer
        /*for (int i = 0; i < N; i++) 
        {
            float world_x = -2.0f + (i % 5) * 0.5f;  // Example X range: -2.0 to 2.0
            float world_y = -1.0f + (i / 5) * 0.5f;  // Example Y range: -1.0 to 1.0
            input[i * 2] = world_x;     // Real part
            input[i * 2 + 1] = world_y; // Imaginary part
            output[i] = 0.0f;     // Initialize output to zero
        }*/

        //int pixel_count = iw * ih;
        //float* input = new float[pixel_count * 2];
        //float* output = new float[pixel_count];
        ///float input[N * 2]; // Each complex number is represented by two floats
        ///float output[N];    // Each coordinate pair produces a single iteration count

        for (int i = 0; i < pixel_count; i++)
        {
            int x = (i % iw);
            int y = (i / iw);

            double fx = static_cast<double>(x);
            double fy = static_cast<double>(y);
            double rx = fx / fw;
            double ry = fy / fh;
            double wx = wx1 + (rx * ww);
            double wy = wy1 + (ry * wh);

            ///float world_x = -2.0f + (i % 5) * 0.5f;  // Example X range: -2.0 to 2.0
            ///float world_y = -1.0f + (i / 5) * 0.5f;  // Example Y range: -1.0 to 1.0
            input[i * 2] = wx;     // Real part
            input[i * 2 + 1] = wy; // Imaginary part
            output[i] = 0.0f;     // Initialize output to zero
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
        shader.program().setUniformValue("MAX_ITER", max_iter); // Set max iterations
        shader.program().setUniformValue("BAILOUT", 4.0f);  // Set bailout threshold
        shader.compute(pixel_count);  // N is now the number of coordinate pairs
        shader.end();

        // Clear CPU-side buffers before reading results
        ///memset(&output[0], 0, sizeof(float)* pixel_count);
        //for (int i = 0; i < pixel_count; i++)
        //{
        //    output[i] = 0.0f;
        //}

        // Read GPU output to CPU buffer
        output_buffer.read_to_cpu(&output[0], sizeof(float)* pixel_count);

        // Display results
        //qDebug() << "Mandelbrot Results: ";
        for (int i = 0; i < pixel_count; i++)
        {
            int x = (i % iw);
            int y = (i / iw);

            //double world_x = input[i * 2];
            //double world_y = input[i * 2 + 1];

            double ratio = output[i];

            //auto [r,g,b] = ratio_color(ratio);

            uint8_t r = (uint8_t)(9 * (1 - ratio) * ratio * ratio * ratio * 255);
            uint8_t g = (uint8_t)(15 * (1 - ratio) * (1 - ratio) * ratio * ratio * 255);
            uint8_t b = (uint8_t)(8.5 * (1 - ratio) * (1 - ratio) * (1 - ratio) * ratio * 255);

            bmp.setPixel(x, y, r, g, b, 255);

            //if (r > 10 || g > 10 || b > 10)
            //    qDebug() << "bmp(" << x << "," << y << ") -> Color: [" << r << ", " << g << ", " << b << "]";

            ///qDebug() << "  (" << world_x << "," << world_y << ") -> Color: [" << r << ", " << g << ", " << b << "]";
        }

        //delete[] input;
        //delete[] output;


        /*const int N = 23;
        float input[N];
        float output[N];
        for (int i = 0; i < N; i++) {
            input[i] = i;
            output[i] = -i;
        }

        ShaderBuffer input_buffer, output_buffer;

        input_buffer.setup(&context);
        output_buffer.setup(&context);

        input_buffer.allocate(input, sizeof(input));
        output_buffer.allocate(output, sizeof(output));

        //Bind buffers to shader
        input_buffer.bind_for_shader(0);
        output_buffer.bind_for_shader(1);


        //Compute
        shader.begin();
        shader.program().setUniformValue("coeff", 0.5f); //set uniforms
        shader.compute(N);
        shader.end();


        //Download input and resulted buffers to CPU
        //Clear to be sure we really read them
        for (int i = 0; i < N; i++) {
            input[i] = 0;
            output[i] = 0;
        }

        input_buffer.read_to_cpu(input, sizeof(input));
        output_buffer.read_to_cpu(output, sizeof(output));

        qDebug() << "Buffers: ";
        for (int i = 0; i < N; i++) {
            qDebug() << "  " << input[i] << " " << output[i];
        }*/









        /*QOpenGLExtraFunctions* glF = ctx->beginGL();

        shader->bind();
        shader->setUniformValue("zoom", static_cast<float>(camera->zoom_x));
        shader->setUniformValue("offsetX", 0);
        shader->setUniformValue("offsetY", 0);

        // Draw full-screen quad
        static const GLfloat verts[] = { -1, -1, 1, -1, -1, 1, 1, 1 };
        glF->glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, verts);
        glF->glEnableVertexAttribArray(0);
        glF->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        ctx->endGL();*/
    }
}

void Mandelbrot_Scene::viewportDraw(Viewport* ctx)
{
    /// Draw Scene to Viewport
    camera->stageTransform();
    ctx->drawBitmap(&bmp, 0, 0, ctx->width, ctx->height);
    
    ctx->drawWorldAxis(0.5, 0, 0.5);

    ctx->print() << "Frame delta time: " << this->project_dt(10) << " ms";
}

/// User Interaction

void Mandelbrot_Scene::mouseDown() {}
void Mandelbrot_Scene::mouseUp() {}
void Mandelbrot_Scene::mouseMove() {}
void Mandelbrot_Scene::mouseWheel() {}

SIM_END(Mandelbrot)