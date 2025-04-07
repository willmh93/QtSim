#include "Cardioid.h"
#include "imgui_spline.h"

SIM_DECLARE(Cardioid, "Fractal", "Cardioid (Mandelbrot)")

// Exported
void plot(Scene *scene, Viewport* ctx, bool interactive, int segments, double ox)
{
    ctx->camera.saveCameraTransform();
    ctx->camera.setTransformFilters(true, false, false, false);

    // Full plot
    ctx->setLineWidth(2);
    ctx->setStrokeStyle(255, 0, 0);
    ctx->beginPath();

    double r1 = 0.5;
    double r2 = r1 / 4.0;
    double angle_step = (2 * M_PI) / static_cast<double>(segments);
    bool first = true;

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
            ctx->lineTo(plot_x, plot_y);
    }

    ctx->stroke();

    if (interactive)
    {

        double tx1, ty1, tx2, ty2, ta, oa, d;
        cardioidPolarCoord(scene->mouse->world_x, scene->mouse->world_y, tx1, ty1, ta, d, oa);

        ctx->camera.scalingSizes(true);

        // Big blue circle
        double r1 = 0.5;
        ctx->setStrokeStyle(255, 0, 255);
        ctx->strokeEllipse(ox, 0, r1);


        // p1 dot
        double p1_x = r1 * cos(oa) + ox;
        double p1_y = r1 * sin(oa);

        // Med green circle
        double r2 = r1 / 4.0;
        double angle2 = 2.0 * oa;
        double c2_x = p1_x - r2 * cos(angle2);
        double c2_y = p1_y - r2 * sin(angle2);
        ctx->setStrokeStyle(255, 0, 255);
        ctx->strokeEllipse(c2_x, c2_y, r2);

        ctx->camera.scalingSizes(false);

        Vec2 off = scene->Offset(5, 5);

        // draw p1 dot
        ctx->setFillStyle(255, 0, 255);
        ctx->fillEllipse(p1_x, p1_y, 8);
        ctx->fillText("p1", p1_x + off.x, p1_y + off.y);

        // draw p2 dot
        double p2_x = c2_x - r2 * cos(angle2);
        double p2_y = c2_y - r2 * sin(angle2);
        ctx->setFillStyle(255, 0, 255);
        ctx->fillEllipse(p2_x, p2_y, 8);
        ctx->fillText("p2", p2_x + off.x, p2_y + off.y);

        // draw tangent arrow
        double tangent_head_x = p2_x + cos(ta) * 0.2;
        double tangent_head_y = p2_y + sin(ta) * 0.2;
        ctx->beginPath();
        ctx->arrowMoveTo(p2_x, p2_y);
        ctx->arrowDrawTo(tangent_head_x, tangent_head_y);
        ctx->stroke();

        // Draw perpendicular arrow (to mouse)
        double perp_angle = wrapRadians2PI(ta - M_PI / 2.0);
        tx2 = tx1 + cos(perp_angle) * d;
        ty2 = ty1 + sin(perp_angle) * d;

        ctx->beginPath();
        ctx->arrowMoveTo(tx1, ty1);
        ctx->arrowDrawTo(tx2, ty2);
        ctx->stroke();
    }
    ctx->camera.restoreCameraTransform();
}

/// Project ///

void Cardioid_Project::projectAttributes()
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

void Cardioid_Scene::sceneAttributes()
{
    //ImGui::SliderDouble("Angle", &interact_angle, 0, (2 * M_PI));
    ImGui::SliderDouble("Angle Step", &interact_angle_step, 0, (2 * M_PI) / 100.0);

    ImGui::SliderDouble("Spin multiplier", &interact_spin_mult, 0.0, 1.0);
    ImGui::SliderDouble("Angle", &interact_angle, 0.0, (2.0 * M_PI));
    ImGui::SliderDouble("Distance", &interact_dist, 0.0, 1.0);

    //ImGui::Checkbox("show offset", &show_offset);
    //ImGui::Checkbox("show original", &show_original);
    //ImGui::Checkbox("show alternative", &show_alternative);
    ImGui::Checkbox("Interactive", &interactive);

    ///static float v[5] = { 0.390f, 0.575f, 0.565f, 1.000f };
    ///ImGui::Bezier( "easeOutSine", v );       // draw
    ///float y = ImGui::BezierValue( 0.5f, v ); // x delta in [0..1] range
}

void Cardioid_Scene::sceneStart()
{
    //cumulative_cardioid = computeCumulativeCardioid((2 * M_PI) / 72000.0);
    cumulative_cardioid_lookup.create((2.0 * M_PI) / 3600.0, 0.01);

}

void Cardioid_Scene::sceneMounted(Viewport* viewport)
{
    /// Initialize viewport (after sceneStart)
    camera->setOriginViewportAnchor(Anchor::CENTER);
    camera->focusWorldRect(-0.9, -1, 0.6, 1, true);
    camera->setRelativeZoomRange(0.001, 100);
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



/*inline double distance(double angle, double mx, double my)
{
    double px = 0.5 * cos(angle) - 0.25 * cos(2.0 * angle);
    double py = 0.5 * sin(angle) - 0.25 * sin(2.0 * angle);
    double dx = mx - px;
    double dy = my - py;
    return dx * dx + dy * dy; // Use squared distance (no sqrt needed)
};*/


/*#include <cmath>
#include <cstddef>
#include <immintrin.h>

void distance_fast(const double* angle, const double* mx, const double* my, double* out, std::size_t count)
{
    //#ifdef USE_AVX2
    std::size_t i = 0;
    for (; i + 3 < count; i += 4) {
        __m256d a = _mm256_loadu_pd(&angle[i]);

        alignas(32) double a_vals[4], sin_vals[4], cos_vals[4];
        _mm256_store_pd(a_vals, a);

        for (int j = 0; j < 4; ++j) {
            sin_vals[j] = std::sin(a_vals[j]);
            cos_vals[j] = std::cos(a_vals[j]);
        }

        __m256d sin_a = _mm256_load_pd(sin_vals);
        __m256d cos_a = _mm256_load_pd(cos_vals);

        __m256d sin_2a = _mm256_mul_pd(_mm256_set1_pd(2.0), _mm256_mul_pd(sin_a, cos_a));
        __m256d cos_2a = _mm256_sub_pd(_mm256_mul_pd(_mm256_set1_pd(2.0), _mm256_mul_pd(cos_a, cos_a)), _mm256_set1_pd(1.0));

        __m256d px = _mm256_sub_pd(_mm256_mul_pd(_mm256_set1_pd(0.5), cos_a), _mm256_mul_pd(_mm256_set1_pd(0.25), cos_2a));
        __m256d py = _mm256_sub_pd(_mm256_mul_pd(_mm256_set1_pd(0.5), sin_a), _mm256_mul_pd(_mm256_set1_pd(0.25), sin_2a));

        __m256d mx_v = _mm256_loadu_pd(&mx[i]);
        __m256d my_v = _mm256_loadu_pd(&my[i]);

        __m256d dx = _mm256_sub_pd(mx_v, px);
        __m256d dy = _mm256_sub_pd(my_v, py);

        __m256d dist2 = _mm256_add_pd(_mm256_mul_pd(dx, dx), _mm256_mul_pd(dy, dy));
        _mm256_storeu_pd(&out[i], dist2);
    }

    // Scalar fallback for leftovers
    for (; i < count; ++i) {
        double a = angle[i];
        double px = 0.5 * std::cos(a) - 0.25 * std::cos(2.0 * a);
        double py = 0.5 * std::sin(a) - 0.25 * std::sin(2.0 * a);
        double dx = mx[i] - px;
        double dy = my[i] - py;
        out[i] = dx * dx + dy * dy;
    }
    ///#else
    ///// Entire scalar fallback if AVX2 not available
    ///for (std::size_t i = 0; i < count; ++i) {
    ///    double a = angle[i];
    ///    double px = 0.5 * std::cos(a) - 0.25 * std::cos(2.0 * a);
    ///    double py = 0.5 * std::sin(a) - 0.25 * std::sin(2.0 * a);
    ///    double dx = mx[i] - px;
    ///    double dy = my[i] - py;
    ///    out[i] = dx * dx + dy * dy;
    ///}
    ///#endif
}


// Computes the squared distance for an angle using AVX2,
// via our compute_squared_distances function that processes 4 doubles.
inline double originalAngleBinarySearch(double mx, double my)
{
    double left = 0.0;
    double right = 2.0 * M_PI;
    const double eps = 1e-13;

    while (right - left > eps)
    {
        double diff = right - left;
        double m1 = left + diff / 3.0;
        double m2 = right - diff / 3.0;

        alignas(32) double angles[4] = { m1, m2, m1, m2 };
        alignas(32) double mx_array[4] = { mx, mx, mx, mx };
        alignas(32) double my_array[4] = { my, my, my, my };
        alignas(32) double out[4];

        distance_fast(angles, mx_array, my_array, out, 4);

        double d1 = out[0];
        double d2 = out[1];

        if (d1 < d2)
            right = m2;
        else
            left = m1;
    }

    return (left + right) / 2.0;
}
*/





double originalAngleFromPerpAngle(double perp_angle)
{
    return wrapRadians2PI((perp_angle + M_PI / 2.0) / 1.5);
}


void Cardioid_Scene::viewportProcess(Viewport* ctx)
{
    /// Process Viewports running this Scene
    //interact_angle = originalAngleFromPoint(mouse->world_x, mouse->world_y);
}

void Cardioid_Scene::viewportDraw(Viewport* ctx)
{
    /// Draw Scene to Viewport
    ctx->drawWorldAxis(1, 0);
    camera->setTransformFilters(true, false, false, false);

    double ox = show_offset ? -0.25 : 0;

    
    //plotCumulativeCardioid(ctx, cumulative_cardioid, interact_spin_mult);

    ctx->beginPath();
    ctx->drawPath(cumulative_cardioid_lookup.lerped(interact_spin_mult));
    ctx->stroke();

    camera->scalingSizes(false);



    Vec2 p = cumulative_cardioid_lookup.project(interact_angle, interact_dist, interact_spin_mult);
    ctx->setFillStyle(255, 0, 0);
    ctx->fillEllipse(p.x, p.y, 5);

    Vec2 p2 = cumulative_cardioid_lookup.originalPolarCoordinate(mouse->world_x, mouse->world_y, interact_spin_mult);
    Vec2 p3 = cumulative_cardioid_lookup.project(p2.x, p2.y, interact_spin_mult);

    ctx->setFillStyle(255, 0, 0);
    ctx->fillEllipse(p3.x, p3.y, 5);
    
    ///if (show_alternative)
    ///    fullPlotAlternative(ctx, 1, ox);
    /// 

    ///fullPlot(ctx, 1, ox, interact_spin_mult);
    ///if (interactive)
    ///    animatePlot(ctx, 1, ox);
}

double originalAngle(double p2_x, double p2_y)
{
    double c2 = (1.0 - std::sqrt(3.0 - 8.0 * p2_x)) / 2.0;
    return std::atan2((2.0 * p2_y) / (1.0 - c2), c2);
}

void Cardioid_Scene::animatePlot(Viewport* ctx, double scale, double ox)
{
    ctx->setLineWidth(2);

    double tx1, ty1, tx2, ty2, ta, oa, d;
    cardioidPolarCoord(mouse->world_x, mouse->world_y, tx1, ty1, ta, d, oa);

    camera->scalingSizes(true);
    
    // Big blue circle
    double r1 = scale * 0.5;
    ctx->setStrokeStyle(80, 80, 255);
    ctx->strokeEllipse(ox * scale, 0, r1);

    // p1 dot
    double p1_x = r1 * cos(oa) + ox * scale;
    double p1_y = r1 * sin(oa);

    // Med green circle
    double r2 = r1 / 4.0;
    double angle2 = 2.0 * oa;
    double c2_x = p1_x - r2 * cos(angle2);
    double c2_y = p1_y - r2 * sin(angle2);
    ctx->setStrokeStyle(0, 255, 0);
    ctx->strokeEllipse(c2_x, c2_y, r2);

    Vec2 off = Offset(5, 5);

    camera->scalingSizes(false);

    // draw p1 dot
    ctx->setFillStyle(0, 255, 255);
    ctx->fillEllipse(p1_x, p1_y, 5);
    ctx->fillText("p1", p1_x + off.x, p1_y + off.y);

    // draw p2 dot
    double p2_x = c2_x - r2 * cos(angle2);
    double p2_y = c2_y - r2 * sin(angle2);
    ctx->setFillStyle(0, 255, 255);
    ctx->fillEllipse(p2_x, p2_y, 5);
    ctx->fillText("p2", p2_x + off.x, p2_y + off.y);

    // draw tangent arrow
    double tangent_head_x = p2_x + cos(ta) * 0.2;
    double tangent_head_y = p2_y + sin(ta) * 0.2;
    ctx->beginPath();
    ctx->arrowMoveTo(p2_x, p2_y);
    ctx->arrowDrawTo(tangent_head_x, tangent_head_y);
    ctx->stroke();
    
    
    // Draw perpendicular arrow (to mouse)
    double perp_angle = wrapRadians2PI(ta - M_PI / 2.0);
    ///tx2 = tx1 + cos(perp_angle) * d;
    ///ty2 = ty1 + sin(perp_angle) * d;
    Vec2 tp = fromPolarCoordinate(oa, d);
    tx2 = tp.x;
    ty2 = tp.y;

    ctx->beginPath();
    ctx->arrowMoveTo(tx1, ty1);
    ctx->arrowDrawTo(tx2, ty2);
    ctx->stroke();

    ctx->print() << "p1 angle: " << QString::asprintf("%.0f deg", oa * 180.0 / M_PI);
    ctx->print() << "\np2 tangent angle: " << QString::asprintf("%.0f deg", ta * 180.0 / M_PI);
    ctx->print() << "\np2 perp angle: " << QString::asprintf("%.0f deg", perp_angle * 180.0 / M_PI);


    //double ta_perp = wrapRadians2PI(ta - M_PI / 2.0);
    //double ta_perp = originalAngleFromPerpAngle(ta);

    /*QString coord_txt = QString::asprintf("(%.1fd, %.2f)", perp_angle *180.0/M_PI, d);
    ctx->fillText(coord_txt, tx2 + off.x, ty2 + off.y);

    Vec2 pt = fromPolarCoordinate(perp_angle, 1);
    ctx->setFillStyle(255, 255, 255);
    ctx->fillEllipse(pt.x, pt.y, 0.015);*/
}

void Cardioid_Scene::plotCumulativeCardioid(
    Viewport* ctx, 
    const CumulativeCardioid& cardioid,
    double angle_mult)
{
    double plot_x = 0.25;
    double plot_y = 0.0;
    double plot_direction = 0.0;

    ctx->beginPath();
    ctx->moveTo(plot_x, plot_y);

    for (int i = 0; i < cardioid.size(); i++)
    {
        double step_angle = cardioid[i].step_angle;
        double step_dist = cardioid[i].step_dist;

        double scaled_step_angle = step_angle * angle_mult;
        plot_direction += scaled_step_angle;

        plot_x += cos(plot_direction) * step_dist;
        plot_y += sin(plot_direction) * step_dist;

        ctx->lineTo(plot_x, plot_y);
    }

    ctx->stroke();
}

void Cardioid_Scene::fullPlot(Viewport* ctx, double scale, double ox, double spin_mult)
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
        //double straight_len = (angle / (2 * M_PI)) * 3.837;

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

void Cardioid_Graph_Scene::sceneAttributes()
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
    bmp.setStageRect(0, 0, ctx->width, ctx->height);

    if (bmp.needsReshading(ctx))
    {
        // Tangent angle heatmap
        bmp.forEachWorldPixel(ctx, [this](int x, int y, double wx, double wy)
        {
            double tx1, ty1, tx2, ty2, ta, oa, d;
            cardioidPolarCoord(wx, wy, tx1, ty1, ta, d, oa);

            if (d >= 0)
            {
                double perp_angle = wrapRadians(ta - M_PI / 2.0);

                double neg_angle = max(0.0, -perp_angle);
                double pos_angle = max(0.0, perp_angle);

                int neg_col = (int)((max(0.0, min(M_PI, neg_angle)) / M_PI) * 255.0);
                int pos_col = (int)((max(0.0, min(M_PI, pos_angle)) / M_PI) * 255.0);

                bmp.setPixel(x, y, neg_col, pos_col, 0, 255);
            }
            else
            {
                bmp.setPixel(x, y, 0, 0, 0, 255);
            }
        });

        // Tangent angle heatmap
        /*std::vector<QFuture<void>> futures(thread_count);

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

                    double tx1, ty1, tx2, ty2, ta, oa, d;
                    cardioidPolarCoord(wx, wy, tx1, ty1, ta, d, oa);

                    if (d >= 0)
                    {
                        //double orig_angle = originalAngleFromPoint(wx, wy);
                        //double perp_angle = wrapRadians2PI(1.5 * orig_angle - M_PI / 2.0) - M_PI;
                        //double perp_angle = ta - M_PI;// wrapRadians2PI(ta - M_PI / 2.0) - M_PI;
                        double perp_angle = wrapRadians(ta - M_PI/2.0);// wrapRadians2PI(ta - M_PI / 2.0) - M_PI;

                        double neg_angle = max(0.0, -perp_angle);
                        double pos_angle = max(0.0, perp_angle);

                        int neg_col = (int)((max(0.0, min(M_PI, neg_angle)) / M_PI) * 255.0);
                        int pos_col = (int)((max(0.0, min(M_PI, pos_angle)) / M_PI) * 255.0);

                        bmp.setPixel(x, y, neg_col, pos_col, 0, 255);
                    }
                    else
                    {
                        bmp.setPixel(x, y, 0, 0, 0, 255);
                    }
                }
            });
        }
 
        for (auto& future : futures)
            future.waitForFinished();*/
    }
}

void Cardioid_Graph_Scene::viewportDraw(Viewport* ctx)
{
    /// Bmp plotting
    ctx->drawSurface(bmp);
    ctx->drawWorldAxis();
    camera->setTransformFilters(true, false, false, false);
    
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
