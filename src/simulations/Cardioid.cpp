#include "Cardioid.h"
SIM_DECLARE(Cardioid, "Mandelbrot", "Cardioid")

/// Project ///

void Cardioid_Project::projectAttributes(Input* input)
{}

void Cardioid_Project::projectPrepare()
{
    auto& layout = newLayout();

    /// Or create a single Scene instance and view on multiple Viewports
    auto* main_scene = create<Cardioid_Scene>();
    layout << main_scene;
    layout << create<Cardioid_Graph_Scene>({main_scene});
}

/// Scene ///

void Cardioid_Scene::sceneAttributes(Input* input)
{
    ImGui::SliderDouble("Angle", &interact_angle, 0, (2 * M_PI));
    ImGui::SliderDouble("Angle Step", &interact_angle_step, 0, (2 * M_PI) / 100.0);

    ImGui::Checkbox("show offset", &show_offset);
    //ImGui::Checkbox("show original", &show_original);
    //ImGui::Checkbox("show alternative", &show_alternative);
    ImGui::Checkbox("Interactive", &interactive);
}

void Cardioid_Scene::sceneMounted(Viewport* viewport)
{
    /// Initialize viewport (after sceneStart)
    camera->setOriginViewportAnchor(Anchor::CENTER);
    camera->focusWorldRect(-0.9, -1, 0.6, 1);
}

/*inline double getAngleFromXY_noComplex(double plot_x, double plot_y)
{
    double a = 1.0 - 4.0 * plot_x;  // real part
    double b = -4.0 * plot_y;      // imaginary part
    double mag = sqrt(a * a + b * b);

    double u = sqrt(0.5 * (mag + a));
    double v = (b >= 0.0) ? sqrt(0.5 * (mag - a)) : -sqrt(0.5 * (mag - a));

    double r1Re = 1.0 + u;
    double r1Im = v;
    double r2Re = 1.0 - u;
    double r2Im = -v;

    double mag1 = sqrt(r1Re * r1Re + r1Im * r1Im);// std::hypot(r1Re, r1Im); // sqrt(r1Re^2 + r1Im^2)
    double mag2 = sqrt(r2Re * r2Re + r2Im * r2Im);// std::hypot(r2Re, r2Im);

    double chosenRe, chosenIm;
    if (fabs(mag1 - 1.0) < fabs(mag2 - 1.0)) {
        chosenRe = r1Re;
        chosenIm = r1Im;
    }
    else {
        chosenRe = r2Re;
        chosenIm = r2Im;
    }
    return atan2(chosenIm, chosenRe);
}*/


inline double distance(double angle, double mx, double my)
{
    double px = 0.5 * cos(angle) - 0.25 * cos(2.0 * angle);
    double py = 0.5 * sin(angle) - 0.25 * sin(2.0 * angle);
    double dx = mx - px;
    double dy = my - py;
    return dx * dx + dy * dy; // Use squared distance (no sqrt needed)
};

inline double originalAngleBinarySearch(double mx, double my)
{
    double left = 0.0;
    double right = 2.0 * M_PI;
    const double eps = 1e-13;

    while (right - left > eps) 
    {
        double m1 = left + (right - left) / 3.0;
        double m2 = right - (right - left) / 3.0;

        if (distance(m1, mx, my) < distance(m2, mx, my))
            right = m2;
        else
            left = m1;
    }

    return (left + right) / 2.0;
}


static const double PI = 3.14159265358979323846;

// Cardioid functions
inline double xTheta(double theta) {
    return 0.5 * std::cos(theta) - 0.25 * std::cos(2.0 * theta);
}

inline double yTheta(double theta) {
    return 0.5 * std::sin(theta) - 0.25 * std::sin(2.0 * theta);
}

inline double dxTheta(double theta) {
    // Derivative of xTheta w.r.t. theta
    return -0.5 * std::sin(theta) + 0.5 * std::sin(2.0 * theta);
}

inline double dyTheta(double theta) {
    // Derivative of yTheta w.r.t. theta
    return  0.5 * std::cos(theta) - 0.5 * std::cos(2.0 * theta);
}

inline double ddxTheta(double theta) {
    // Second derivative of xTheta w.r.t. theta
    return -0.5 * std::cos(theta) + 1.0 * std::cos(2.0 * theta);
}

inline double ddyTheta(double theta) {
    // Second derivative of yTheta w.r.t. theta
    return -0.5 * std::sin(theta) + 1.0 * std::sin(2.0 * theta);
}

// Newton's Method to find theta that minimizes distance^2 from (px,py)
double originalAngleFromPoint(double px, double py, double tolerance, int maxIter)
{
    // Check if inside unstable region for Newton's method
    double y_bound = 0.001 + 4 * pow(px - 0.23, 2);
    if (px >= 0.25 && py < y_bound && py > -y_bound)
        return originalAngleBinarySearch(px, py);

    double initialGuess = atan2(py, px - 0.25);
    double theta = initialGuess;

    for (int i = 0; i < maxIter; ++i)
    {
        // Compute Delta x and Delta y
        double X = xTheta(theta) - px;
        double Y = yTheta(theta) - py;

        // First derivative f'(theta)
        double d1 = 2.0 * (X * dxTheta(theta) + Y * dyTheta(theta));

        // Second derivative f''(theta)
        double d2 = 2.0 * ((dxTheta(theta) * dxTheta(theta)) + X * ddxTheta(theta))
                  + 2.0 * ((dyTheta(theta) * dyTheta(theta)) + Y * ddyTheta(theta));

        // If denominator is near zero, break (avoid divide by zero)
        if (std::fabs(d2) < 1e-14) break;

        // Newton update
        double step = d1 / d2;
        theta -= step;

        // Wrap angle
        if (theta < 0)
            theta += 2.0 * PI;
        else if (theta >= 2.0 * PI) 
            theta -= 2.0 * PI;

        // If the step is sufficiently small, we consider we've converged
        if (std::fabs(step) < tolerance)
            break;
    }

    return theta;
}

inline void cardioidPolarCoord(
    double px, 
    double py, 
    double& tangent_x,
    double& tangent_y,
    double& tangent_angle,
    double& dist)
{
    double angle = originalAngleFromPoint(px, py);

    tangent_angle = 1.5 * angle;
    tangent_x = 0.5 * cos(angle) - 0.25 * cos(angle * 2.0);
    tangent_y = 0.5 * sin(angle) - 0.25 * sin(angle * 2.0);

    double dx = px - tangent_x;
    double dy = py - tangent_y;
    dist = sqrt(dx * dx + dy * dy);
}

double originalAngleFromPerpAngle(double perp_angle)
{
    return wrapRadians2PI((perp_angle + M_PI / 2.0) / 1.5);
}

Vec2 fromCardioidTransform(double perp_angle, double dist)
{
    double angle = (perp_angle + M_PI / 2.0) / 1.5;
    double tx = 0.5 * cos(angle) - 0.25 * cos(angle * 2.0);
    double ty = 0.5 * sin(angle) - 0.25 * sin(angle * 2.0);
    double px = tx + cos(perp_angle) * dist;
    double py = ty + sin(perp_angle) * dist;
    return { px, py };
}

void Cardioid_Scene::viewportProcess(Viewport* ctx)
{
    /// Process Viewports running this Scene
    interact_angle = originalAngleFromPoint(mouse->world_x, mouse->world_y);
}

void Cardioid_Scene::viewportDraw(Viewport* ctx)
{
    /// Draw Scene to Viewport
    ctx->drawWorldAxis(1, 0);
    camera->setTransformFilters(true, false, false);

    double ox = show_offset ? -0.25 : 0;

    fullPlot(ctx, 1, ox);

    ///if (show_alternative)
    ///    fullPlotAlternative(ctx, 1, ox);
    /// 

    if (interactive)
        animatePlot(ctx, 1, ox);
}

double originalAngle(double p2_x, double p2_y)
{
    double c2 = (1.0 - std::sqrt(3.0 - 8.0 * p2_x)) / 2.0;
    return std::atan2((2.0 * p2_y) / (1.0 - c2), c2);
}

void Cardioid_Scene::animatePlot(Viewport* ctx, double scale, double ox)
{
    ctx->setLineWidth(2);

    // Big blue circle
    double r1 = scale * 0.5;
    ctx->setStrokeStyle(80, 80, 255);
    ctx->strokeEllipse(ox * scale, 0, r1);

    // p1 dot
    double p1_x = r1 * cos(interact_angle) + ox * scale;
    double p1_y = r1 * sin(interact_angle);

    // Med green circle
    double r2 = r1 / 4.0;
    double angle2 = 2.0 * interact_angle;
    double c2_x = p1_x - r2 * cos(angle2);
    double c2_y = p1_y - r2 * sin(angle2);
    ctx->setStrokeStyle(0, 255, 0);
    ctx->strokeEllipse(c2_x, c2_y, r2);

    Vec2 off = Offset(5, 5);

    // draw p1 dot
    ctx->setFillStyle(0, 255, 255);
    ctx->fillEllipse(p1_x, p1_y, 0.015);
    ctx->fillText("p1", p1_x + off.x, p1_y + off.y);

    // draw p2 dot
    double p2_x = c2_x - r2 * cos(angle2);
    double p2_y = c2_y - r2 * sin(angle2);
    ctx->setFillStyle(0, 255, 255);
    ctx->fillEllipse(p2_x, p2_y, 0.015);
    ctx->fillText("p2", p2_x + off.x, p2_y + off.y);

    // draw tangent
    double perp_angle = 1.5 * interact_angle;// -M_PI / 2.0;
    double perp_x = p2_x + cos(perp_angle) * 0.2;
    double perp_y = p2_y + sin(perp_angle) * 0.2;
    ctx->beginPath();
    ctx->arrowMoveTo(p2_x, p2_y);
    ctx->arrowDrawTo(perp_x, perp_y);
    ctx->stroke();
    
    double tx1, ty1, tx2, ty2, ta, d;
    cardioidPolarCoord(mouse->world_x, mouse->world_y, tx1, ty1, ta, d);

    tx2 = tx1 + cos(ta - M_PI / 2.0) * d;
    ty2 = ty1 + sin(ta - M_PI / 2.0) * d;

    ctx->beginPath();
    ctx->arrowMoveTo(tx1, ty1);
    ctx->arrowDrawTo(tx2, ty2);
    ctx->stroke();

    double ta_perp = wrapRadians2PI(ta - M_PI / 2.0);
    //double ta_perp = originalAngleFromPerpAngle(ta);

    QString coord_txt = QString::asprintf("(%.1fd, %.2f)", ta_perp *180.0/M_PI, d);
    ctx->fillText(coord_txt, tx2 + off.x, ty2 + off.y);

    Vec2 pt = fromCardioidTransform(ta_perp, 1);
    ctx->setFillStyle(255, 255, 255);
    ctx->fillEllipse(pt.x, pt.y, 0.015);
}

void Cardioid_Scene::fullPlot(Viewport* ctx, double scale, double ox)
{
    // Full plot
    ctx->setStrokeStyle(255, 0, 0);
    ctx->beginPath();

    double r1 = scale * 0.5;
    double r2 = r1 / 4.0;

    double angle_step = (2 * M_PI) / 100.0;

    bool first = true;
    for (double angle = 0.0; angle < 2 * M_PI; angle += angle_step)
    {
        double angle2 = 2.0 * angle;

        double c1_x = r1 * cos(angle) + ox * scale;
        double c1_y = r1 * sin(angle);
        double c2_x = c1_x - r2 * cos(angle2);
        double c2_y = c1_y - r2 * sin(angle2);

        double plot_x = c2_x - r2 * cos(angle2);
        double plot_y = c2_y - r2 * sin(angle2);

        //double plot_x = 0.5 * cos(angle) - 0.25 * cos(angle * 2.0);
        //double plot_y = 0.5 * sin(angle) - 0.25 * sin(angle * 2.0);

        if (first)
        {
            ctx->moveTo(plot_x, plot_y);
            first = false;
        }
        else
        {
            ctx->lineTo(plot_x, plot_y);
        }
    }

    ctx->stroke();
}

void Cardioid_Scene::fullPlotAlternative(Viewport* ctx, double scale, double ox)
{
    // Full plot
    ctx->setStrokeStyle(255, 0, 255);
    ctx->beginPath();

    ctx->setLineWidth(4);
    ctx->setStrokeStyle(255, 0, 255);

    double r1 = scale * 0.5;
    double r2 = r1 / 4.0;

    double angle_step = (2 * M_PI) / 100.0;

    bool first = true;
    for (double angle = 0.0; angle < 2 * M_PI; angle += angle_step)
    {
        double a = sin(angle * 0.5);
        double plot_x = cos(angle) * pow(sin(angle * 0.5), 2) + (ox + 0.25) * scale;
        double plot_y = sin(angle) * pow(sin(angle * 0.5), 2);

        if (first)
        {
            ctx->moveTo(plot_x, plot_y);
            first = false;
        }
        else
        {
            ctx->lineTo(plot_x, plot_y);
        }
    }
    ctx->stroke();
}

void Cardioid_Graph_Scene::sceneAttributes(Input* input)
{}

void Cardioid_Graph_Scene::sceneStart()
{
    thread_count = (int)(1.5 * ((double)QThread::idealThreadCount()));
    pool.setMaxThreadCount(thread_count);
}

void Cardioid_Graph_Scene::sceneMounted(Viewport* viewport)
{
    camera->setOriginViewportAnchor(Anchor::CENTER);
    camera->focusWorldRect(-0.9, -1, 0.6, 1);
}

void Cardioid_Graph_Scene::viewportProcess(Viewport* ctx)
{
    int iw = ctx->width / 2;
    int ih = ctx->height / 2;
    bmp.setBitmapSize(iw, ih);

    if (bmp.reshadingStage(camera, 0, 0, ctx->width, ctx->height))
    {
        // Tangent angle heatmap
        std::vector<QFuture<void>> futures(thread_count);

        int pixel_count = iw * ih;
        auto pixel_ranges = splitRanges(pixel_count, thread_count);

        for (int ti = 0; ti < thread_count; ti++)
        {
            auto& pixel_range = pixel_ranges[ti];
            futures[ti] = QtConcurrent::run([&]()
            {
                for (int i = pixel_range.first; i < pixel_range.second; i++)
                {
                    int x = (i % iw), y = (i / iw);
                    double wx = bmp.wx(x);
                    double wy = bmp.wy(y);

                    double orig_angle = originalAngleFromPoint(wx, wy);
                    double perp_angle = wrapRadians2PI(1.5 * orig_angle - M_PI / 2.0) - M_PI;

                    double neg_angle = max(0.0, -perp_angle);
                    double pos_angle = max(0.0, perp_angle);

                    int pos_col = (int)((max(0.0, min(M_PI, neg_angle)) / M_PI) * 255.0);
                    int neg_col = (int)((max(0.0, min(M_PI, pos_angle)) / M_PI) * 255.0);

                    bmp.setPixel(x, y, neg_col, pos_col, 0, 255);
                }
            });
        }
 
        for (auto& future : futures)
            future.waitForFinished();
    }
}

void Cardioid_Graph_Scene::viewportDraw(Viewport* ctx)
{
    /// Bmp plotting
    ctx->drawSurface(bmp);
    ctx->drawWorldAxis();
    camera->setTransformFilters(true, false, false);
    
    double angle_step = (2.0 * M_PI) / 720.0;
    bool first;

    /// Show unstable region bound
    /*ctx->setLineWidth(2);
    ctx->setStrokeStyle(0, 0, 0);
    ctx->beginPath();
    ctx->moveTo(0.25, 0);
    for (double x = 0.25; x < 50.0; x += 0.01)
    {
        double y_bound = 0.001 + 4 * pow(x - 0.24, 2);
        ctx->lineTo(x, y_bound);
    }
    ctx->stroke();*/

    // Full plot
    ctx->setLineWidth(2);
    ctx->setStrokeStyle(255, 255, 255);
    ctx->beginPath();

    first = true;
    for (double angle = 0.0; angle < 2 * M_PI; angle += angle_step)
    {
        double plot_x = 0.5 * cos(angle) - 0.25 * cos(angle * 2.0);
        double plot_y = 0.5 * sin(angle) - 0.25 * sin(angle * 2.0);

        if (first)
        {
            ctx->moveTo(plot_x, plot_y);
            first = false;
        }
        else
        {
            ctx->lineTo(plot_x, plot_y);
        }
    }

    ctx->stroke();
}


/// User Interaction

void Cardioid_Scene::mouseDown() {}
void Cardioid_Scene::mouseUp() {}
void Cardioid_Scene::mouseMove() {}
void Cardioid_Scene::mouseWheel() {}

SIM_END(Cardioid)
