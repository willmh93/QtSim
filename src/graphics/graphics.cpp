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

#include "graphics.h"
#include "Project.h"

#include "qnanopainter.h"

#include <QOpenGLFramebufferObjectFormat>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>



OffscreenNanoPainter::OffscreenNanoPainter()
{}

OffscreenNanoPainter::~OffscreenNanoPainter()
{
    if (m_fbo)
    {
        m_fbo->release();
        delete m_fbo;
    }
    if (painter)
        delete painter;
}

QNanoPainter* OffscreenNanoPainter::begin(int w, int h, bool capture_pixels)
{
    QOpenGLFunctions glF(QOpenGLContext::currentContext());

    glF.glGetIntegerv(GL_FRAMEBUFFER_BINDING, &previousFBO);

    if (w != width || h != height)
    {
        width = w;
        height = h;

        if (m_fbo)
            delete m_fbo;

        QOpenGLFramebufferObjectFormat format;
        //format.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
        format.setInternalTextureFormat(GL_RGBA8);
        m_fbo = new QOpenGLFramebufferObject(width, height, format);
        painter = new QNanoPainter();
    }

    GLint viewport[4];
    glF.glGetIntegerv(GL_VIEWPORT, viewport);
    old_vw = viewport[2];   // Width of the viewport
    old_vh  = viewport[3];  // Height of the viewport

    // Bind the framebuffer for offscreen rendering
    m_fbo->bind();

    // Clear the framebuffer
    glF.glViewport(0, 0, width, height);
    glF.glClearColor(0.0f, 0.0f, 0.0f, 0.0f/*255.0f*/);
    glF.glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Create QNanoPainter and begin drawing
    painter->beginFrame(width, height);
    return painter;

}

void OffscreenNanoPainter::end()
{
    painter->endFrame();
    readPixels();

    QOpenGLFunctions glF(QOpenGLContext::currentContext());
    glF.glViewport(0, 0, old_vw, old_vh);

    //delete painter;
    //painter = nullptr;

    m_fbo->release();

    glF.glBindFramebuffer(GL_FRAMEBUFFER, previousFBO);
}

void OffscreenNanoPainter::drawToPainter(QNanoPainter* p, double x, double y)
{
    auto offscreenImage = QNanoImage::fromFrameBuffer(m_fbo);
    p->drawImage(offscreenImage, x, y, width, height);
}

void OffscreenNanoPainter::drawToPainter(QNanoPainter *p, double x, double y, double w, double h)
{
    auto offscreenImage = QNanoImage::fromFrameBuffer(m_fbo);
    double scale_x = w / width;
    double scale_y = h / height;

    // Draw the offscreen image onto the target painter
    p->save();
    p->scale(scale_x, scale_y);
    p->drawImage(offscreenImage, x / scale_x, y / scale_y, width, height);
    p->restore();
}

void OffscreenNanoPainter::readPixels()
{
    QOpenGLFunctions glF(QOpenGLContext::currentContext());

    int data_len = width * height * 4;
    if (data.size() != data_len)
        data.resize(data_len);

    glF.glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data.data());
}

const std::vector<GLubyte>& OffscreenNanoPainter::getPixels()
{
    return data;
}

QImage OffscreenNanoPainter::toImage() const
{
    return m_fbo->toImage();
}

// todo: Move to PaintContext
void Draw::arrow(PaintContext* ctx, Vec2& a, Vec2& b, QColor color, double arrow_size)
{
    QNanoPainter* p = ctx->painter;

    double dx = b.x - a.x;
    double dy = b.y - a.y;
    double len = sqrt(dx * dx + dy * dy);
    double angle = atan2(dy, dx);
    constexpr double tip_sharp_angle = 45.0 * M_PI / 180.0;

    if (arrow_size < 0)
        arrow_size = len * 0.05;

    //p->setLineCap(QNanoPainter::LineCap::CAP_ROUND);
    QNanoColor c = QNanoColor::fromQColor(color);
    p->setFillStyle(c);
    p->setStrokeStyle(c);
    p->beginPath();
    p->moveTo(a);
    p->lineTo(b);
    p->stroke();

    double rx1 = b.x + cos(angle + tip_sharp_angle) * arrow_size;
    double ry1 = b.y + sin(angle + tip_sharp_angle) * arrow_size;
    double rx2 = b.x + cos(angle - tip_sharp_angle) * arrow_size;
    double ry2 = b.y + sin(angle - tip_sharp_angle) * arrow_size;

    p->beginPath();
    p->moveTo(b);
    p->lineTo(rx1, ry1);
    p->lineTo(rx2, ry2);
    p->closePath();
    p->fill();
}

int Bitmap::bmp_index = 0;
void Bitmap::draw(PaintContext* ctx, double x, double y, double w, double h)
{
    img.loadFromData(data.data());

    ///nvgUpdateImage(ctx->painter->nvgCtx(), m_imageData->id, img->bits());

    //nano_img = QNanoImage::fromCache(img, "dummy");
    nano_img.updateImage(ctx->painter);

    //nano_img.updateImage(ctx->painter);
    ctx->painter->drawImage(nano_img, x, y, w, h);
}

void Bitmap::draw(PaintContext* ctx, const Vec2& pt, const Vec2& size)
{
    draw(ctx, pt.x, pt.y, size.x, size.y);
}
