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

#include "Canvas2D.h"
#include "Simulation.h"

Canvas2D::Canvas2D(QWidget* parent)
    : QNanoWidget(parent)
{
    //setFillColor("#000000");
    setMouseTracking(true);
}

Canvas2D::~Canvas2D()
{}

void Canvas2D::mousePressEvent(QMouseEvent* event)
{
    if (!sim) return;
    QPointF mousePos = event->position();
    sim->_mouseDown(mousePos.x(), mousePos.y(), event->button());
    update();
}

void Canvas2D::mouseReleaseEvent(QMouseEvent* event)
{
    if (!sim) return;
    QPointF mousePos = event->position();
    sim->_mouseUp(mousePos.x(), mousePos.y(), event->button());
    update();
}

void Canvas2D::mouseMoveEvent(QMouseEvent* event)
{
    if (!sim) return;
    QPointF mousePos = event->position();
    sim->_mouseMove(mousePos.x(), mousePos.y());
    update();
}

void Canvas2D::wheelEvent(QWheelEvent* event)
{
    if (!sim) return;
    QPointF mousePos = event->position();
    sim->_mouseWheel(mousePos.x(), mousePos.y(), event->angleDelta().y());
    update();
}

void Canvas2D::initializeGL()
{
    initializeOpenGLFunctions();
    QNanoWidget::initializeGL();
}

void Canvas2D::paint(QNanoPainter* p)
{
    QScreen* screen = this->screen();
    qreal scaleFactor = screen->devicePixelRatio();

    int vw = width();
    int vh = height();

    p->beginFrame(vw*scaleFactor, vh*scaleFactor);
    p->scale(scaleFactor);

    if (!sim || !sim->started)
    {
        p->setFillStyle({ 10,10,15 });
        p->fillRect(0, 0, vw, vh);
    }
    
    if (sim && sim->started)
    {
        if (!render_to_offscreen)
        {
            sim->_draw(p);
            sim->onPainted(nullptr);
        }
        else
        {
            double offscreen_aspect_ratio = ((double)offscreen_w / (double)offscreen_h);
            double viewport_aspect_ratio = ((double)vw / (double)vh);
            double off_x = 0, off_y = 0, scale_x = 1, scale_y = 1;

            if (offscreen_aspect_ratio > viewport_aspect_ratio)
            {
                // Offscreen is wider relative to viewport
                scale_x = static_cast<double>(vw) / static_cast<double>(offscreen_w);
                scale_y = scale_x;

                // Center vertically
                off_y = (vh - (offscreen_h * scale_y)) / 2.0;
            }
            else
            {
                // Offscreen is taller relative to viewport
                scale_y = static_cast<double>(vh) / static_cast<double>(offscreen_h);
                scale_x = scale_y;

                // Center horizontally
                off_x = (vw - (offscreen_w * scale_x)) / 2.0;
            }

            // Draw simulation to offscreen painter
            auto offscreen_painter = offscreen_nano_painter.begin(offscreen_w, offscreen_h, true);
            sim->_draw(offscreen_painter);
            offscreen_nano_painter.end();

            // Provide simulation with frame pixels
            sim->onPainted(&offscreen_nano_painter.getPixels());

            // Draw offscreen painter to main painter
            offscreen_nano_painter.drawToPainter(p,
                off_x,
                off_y,
                scale_x * offscreen_w,
                scale_y * offscreen_h);
        }
    }

    p->endFrame();
}