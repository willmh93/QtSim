/*
 * This file is part of QtSim
 *
 * Copyright (C) 2025 William Hemsworth
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "paint_context.h"
#include "helpers.h"
#include "graphics.h"

double roundAxisTickStep(double ideal_step)
{
    if (ideal_step <= 0)
        return 0; // or handle error

    // Determine the order of magnitude of 'ideal_step'
    double exponent = floor(log10(ideal_step));
    double factor = pow(10.0, exponent);

    // Normalize ideal_step to the range [1, 10)
    double base = ideal_step / factor;
    double niceMultiplier;

    // 0.5, 1

    // Choose the largest candidate from {1, 2, 2.5, 5, 10} that is <= base.
    if (base >= 5)
        niceMultiplier = 5;
    else
        niceMultiplier = 1;

    /*if (base >= 10.0)
        niceMultiplier = 10.0;
    else if (base >= 5.0)
        niceMultiplier = 5.0;
    else if (base >= 2.5)
        niceMultiplier = 2.5;
    else if (base >= 2.0)
        niceMultiplier = 2.0;
    else
        niceMultiplier = 1.0;*/

    return niceMultiplier * factor;
}

/*double upperTickStep(double ideal_step, double rounded_step)
{
    double base = step / factor;
    double currentMultiplier = 

    // Choose the largest candidate from {1, 2, 2.5, 5, 10} that is <= base.
    if (base >= 5)
        niceMultiplier = 5;
    else if (base >= 2.5)
        niceMultiplier = 2.5;
    else
        niceMultiplier = 1;


    return niceMultiplier * factor;
}
*/
double roundAxisValue(double v, double step)
{
    return floor(v / step) * step;
}

double ceilAxisValue(double v, double step)
{
    return ceil(v / step) * step;
}

double getAngle(Vec2 a, Vec2 b)
{
    return (b - a).angle();
}

//double wrap180(double angle)
//{
//
//}

void PaintContext::drawBitmap(Bitmap* bmp, double x, double y, double w, double h)
{
    if (camera.transform_coordinates)
    {
        painter->strokeRect(x, y, w, h);
    }
    else
    {
        QTransform cur_transform = painter->currentTransform();
        painter->resetTransform();
        painter->transform(default_viewport_transform);

        bmp->draw(this, x, y, w, h);

        painter->resetTransform();
        painter->transform(cur_transform);
    }
}

void PaintContext::drawWorldAxis(
    double axis_opacity,
    double grid_opacity, 
    double text_opacity)
{
    painter->save();

    // Fist, draw axis
    Vec2 stage_origin = camera.toStage(0, 0);
    FRect stage_rect = { 0, 0, width, height };

    // World quad
    Vec2 world_tl = camera.toWorld(0, 0);
    Vec2 world_tr = camera.toWorld(width, 0);
    Vec2 world_br = camera.toWorld(width, height);
    Vec2 world_bl = camera.toWorld(0, height);

    double world_w = world_br.x - world_tl.x;
    double world_h = world_br.y - world_tl.y;

    double stage_size = sqrt(width * width + height * height);
    double world_size = sqrt(world_w * world_w + world_h * world_h);
    double world_zoom = (world_size / stage_size);
    double angle = camera.rotation;

    // Get +positive Axis rays
    Ray axis_rayX = { stage_origin, angle + 0 };
    Ray axis_rayY = { stage_origin, angle + M_PI_2 };

    // Get axis intersection with viewport rect
    Vec2 negX_intersect, posX_intersect, negY_intersect, posY_intersect;
    bool x_axis_visible = getRayRectIntersection(&negX_intersect, &posX_intersect, stage_rect, axis_rayX);
    bool y_axis_visible = getRayRectIntersection(&negY_intersect, &posY_intersect, stage_rect, axis_rayY);

    // Convert to world coordinates
    Vec2 negX_intersect_world = camera.toWorld(negX_intersect);
    Vec2 posX_intersect_world = camera.toWorld(posX_intersect);
    Vec2 negY_intersect_world = camera.toWorld(negY_intersect);
    Vec2 posY_intersect_world = camera.toWorld(posY_intersect);

    // Draw with world coordinates, without line scaling
    camera.setTransformFilters(true, false, false);

    // Draw main axis lines
    setStrokeStyle(255, 255, 255, static_cast<int>(255.0 * axis_opacity));
    setLineWidth(1);

    beginPath();
    moveToSharp(negX_intersect_world.x, negX_intersect_world.y);
    lineToSharp(posX_intersect_world.x, posX_intersect_world.y);
    moveToSharp(negY_intersect_world.x, negY_intersect_world.y);
    lineToSharp(posY_intersect_world.x, posY_intersect_world.y);
    stroke();

    // Draw axis ticks
    camera.setTransformFilters(false, false, false);
    double ideal_step_wx = (width / 7.0) / camera.zoom_x;
    double ideal_step_wy = (height / 7.0) / camera.zoom_y;
    double step_wx = roundAxisTickStep(ideal_step_wx);
    double step_wy = roundAxisTickStep(ideal_step_wy);
    double step, ideal_step;
    if (step_wx < step_wy)
    {
        ideal_step = ideal_step_wx;
        step = step_wy = step_wx;
    }
    else
    {
        ideal_step = ideal_step_wy;
        step = step_wx = step_wy;
    }

    //double upperTickStep = 

    double prev_step = fmod(step, ideal_step *2.0);
    double next_step = roundAxisTickStep(prev_step * 5.0);
    double step_stretch = (ideal_step - prev_step) / (next_step - prev_step);
    double big_step_wx = step_wx * 5.0;
    double big_step_wy = step_wy * 5.0;

    


    /*fillText("ideal_step: " + QString::number(ideal_step), 0, 0);
    fillText("prev_step: " + QString::number(prev_step), 0, 20);
    fillText("next_step: " + QString::number(next_step), 0, 40);
    fillText("step_stretch: " + QString::number(step_stretch), 0, 60);*/

    double big_angle = step_stretch * M_PI * 2;
    double small_angle = (big_angle) + M_PI/2;

    double big_visibility = 1;// sin(big_angle) / 2 + 0.5;
    double small_visibility = 0;// sin(small_angle) / 2 + 0.5;

    /*fillText("big_angle: " + QString::number((int)(big_angle*180.0/M_PI)), 0, 100);
    fillText("big_visibility: " + QString::number(big_visibility), 0, 120);

    fillText("small_angle: " + QString::number((int)(small_angle * 180.0 / M_PI)), 0, 160);
    fillText("small_visibility: " + QString::number(small_visibility), 0, 180);*/

    Vec2 x_perp_off = camera.toStageOffset(0, 6 * world_zoom);
    Vec2 x_perp_norm = camera.toStageOffset(0, 1).normalized();

    Vec2 y_perp_off = camera.toStageOffset(6 * world_zoom, 0);
    Vec2 y_perp_norm = camera.toStageOffset(1, 0).normalized();

    setTextAlign(TextAlign::ALIGN_CENTER);
    setTextBaseline(TextBaseline::BASELINE_MIDDLE);

    QNanoFont font(QNanoFont::FontId::DEFAULT_FONT_NORMAL);
    font.setPixelSize(12);
    setFont(font);
    //painter->setPixelAlignText(QNanoPainter::PixelAlign::PIXEL_ALIGN_HALF);
    //painter->setPixelAlign(QNanoPainter::PixelAlign::PIXEL_ALIGN_HALF);

    double spacing = 8;

    // Get world bounds, regardless of rotation
    double world_minX = std::min({ world_tl.x, world_tr.x, world_br.x, world_bl.x });
    double world_maxX = std::max({ world_tl.x, world_tr.x, world_br.x, world_bl.x });
    double world_minY = std::min({ world_tl.y, world_tr.y, world_br.y, world_bl.y });
    double world_maxY = std::max({ world_tl.y, world_tr.y, world_br.y, world_bl.y });

    // Draw gridlines (big step)
    if (grid_opacity > 0)
    {
        //setStrokeStyle(255, 255, 255, static_cast<int>(255.0 * grid_opacity));
        //beginPath();
        


        setFillStyle(255, 255, 255, big_visibility * 5.0);

        bool offset = false;
        Vec2 p1, p2;

        for (double wy = ceilAxisValue(world_minY, big_step_wy); wy < world_maxY; wy += big_step_wy)
        {
            offset = !offset;

            int64_t iy = wy / big_step_wy;

            Ray line_ray_y(camera.toStage(0, wy), angle);
            if (!getRayRectIntersection(&p1, &p2, stage_rect, line_ray_y)) break;

            for (double wx = ceilAxisValue(world_minX, big_step_wx); wx < world_maxX; wx += big_step_wx)
            {
                int64_t ix = wx / big_step_wx;
                //int64_t i = ix + (iy % 2 ? 1 : 0);

                Ray line_ray_x(camera.toStage(wx, 0), angle + M_PI_2);
                if (!getRayRectIntersection(&p1, &p2, stage_rect, line_ray_x)) break;

                if ((ix + iy) % 2 == 0)
                    fillRect(wx, wy, big_step_wx, big_step_wy);
            }

        }
    }

    // Small step
    {
        //setStrokeStyle(255, 255, 255, static_cast<int>(255.0 * grid_opacity));
        //beginPath();
        

        setFillStyle(255, 255, 255, 5.0 * small_visibility);

        bool offset = false;
        Vec2 p1, p2;

        for (double wy = ceilAxisValue(world_minY, step_wy); wy < world_maxY; wy += step_wy)
        {
            offset = !offset;

            int64_t iy = wy / step_wy;

            Ray line_ray_y(camera.toStage(0, wy), angle);
            if (!getRayRectIntersection(&p1, &p2, stage_rect, line_ray_y)) break;

            for (double wx = ceilAxisValue(world_minX, step_wx); wx < world_maxX; wx += step_wx)
            {
                int64_t ix = wx / step_wx;
                //int64_t i = ix + (iy % 2 ? 1 : 0);

                Ray line_ray_x(camera.toStage(wx, 0), angle + M_PI_2);
                if (!getRayRectIntersection(&p1, &p2, stage_rect, line_ray_x)) break;

                if ((ix+iy) % 2 == 0)
                    fillRect(wx, wy, step_wx, step_wy);
            }
            
        }

        // X
        /*Vec2 p1, p2;
        for (double wx = ceilAxisValue(world_minX, step_wx); wx < world_maxX; wx += step_wx)
        {
            if (abs(wx) < 1e-9) continue;
            Ray line_ray(camera.toStage(wx, 0), angle + M_PI_2);
            if (!getRayRectIntersection(&p1, &p2, stage_rect, line_ray)) break;

            //moveToSharp(p1);
            //lineToSharp(p2);
        }

        // Y
        for (double wy = ceilAxisValue(world_minY, step_wy); wy < world_maxY; wy += step_wy)
        {
            if (abs(wy) < 1e-9) continue;
            Ray line_ray(camera.toStage(0, wy), angle);
            if (!getRayRectIntersection(&p1, &p2, stage_rect, line_ray)) break;

            //moveToSharp(p1);
            //lineToSharp(p2);
        }*/

        //stroke();
    }

    setStrokeStyle(255, 255, 255, static_cast<int>(255.0 * axis_opacity));
    setFillStyle(255, 255, 255, static_cast<int>(255.0 * text_opacity));
    beginPath();

    bool draw_text = (text_opacity > 0.01);

    // Draw x-axis labels
    for (double wx = ceilAxisValue(world_minX, step_wx); wx < world_maxX; wx += step_wx)
    {
        if (abs(wx) < 1e-9) continue;

        Vec2 stage_pos = camera.toStage(wx, 0);
        QString txt = QString("%1").arg(wx);
        Vec2 txt_size = boundingBoxNumberScientific(wx).size();

        double txt_dist = (abs(cos(angle)) * txt_size.y + abs(sin(angle)) * txt_size.x) * 0.5 + spacing;

        Vec2 tick_anchor = stage_pos + x_perp_off + (x_perp_norm * txt_dist);

        moveToSharp(stage_pos - x_perp_off);
        lineToSharp(stage_pos + x_perp_off);
        if (draw_text) fillNumberScientific(wx, tick_anchor);
    }

    // Draw y-axis labels
    for (double wy = ceilAxisValue(world_minY, step_wy); wy < world_maxY; wy += step_wy)
    {
        if (abs(wy) < 1e-9) continue;

        Vec2 stage_pos = camera.toStage(0, wy);
        QString txt = QString("%1").arg(wy);
        //Vec2 txt_size = measureText(txt);
        Vec2 txt_size = boundingBoxNumberScientific(wy).size();

        double txt_dist = (abs(cos(angle)) * txt_size.y + abs(sin(angle)) * txt_size.x) * 0.5 + spacing;

        Vec2 tick_anchor = stage_pos + y_perp_off + (y_perp_norm * txt_dist);

        moveToSharp(stage_pos - y_perp_off);
        lineToSharp(stage_pos + y_perp_off);
        if (draw_text) fillNumberScientific(wy, tick_anchor);
    }

    stroke();
    painter->restore();
    camera.setTransformFilters(true, true, true);
}
