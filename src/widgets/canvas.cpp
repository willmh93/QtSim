#include "canvas.h"
#include "project.h"
#include "main_window.h"

CanvasWidget::CanvasWidget(QWidget* parent)
    : QNanoWidget(parent)
{
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
}

void CanvasWidget::mousePressEvent(QMouseEvent* event)
{
    if (!render_source) return;
    QPointF mousePos = event->position();
    render_source->mouseDown(mousePos.x(), mousePos.y(), event->button());
    update();
}

void CanvasWidget::mouseReleaseEvent(QMouseEvent* event)
{
    if (!render_source) return;
    QPointF mousePos = event->position();
    render_source->mouseUp(mousePos.x(), mousePos.y(), event->button());
    update();
}

void CanvasWidget::mouseMoveEvent(QMouseEvent* event)
{
    if (!render_source) return;
    QPointF mousePos = event->position();
    render_source->mouseMove(mousePos.x(), mousePos.y());
    update();
}

void CanvasWidget::wheelEvent(QWheelEvent* event)
{
    if (!render_source) return;
    QPointF mousePos = event->position();
    render_source->mouseWheel(mousePos.x(), mousePos.y(), event->angleDelta().y());
    update();
}

void CanvasWidget::keyPressEvent(QKeyEvent* event)
{
    if (!render_source) return;
    render_source->keyPress(event);
    update();
}

void CanvasWidget::keyReleaseEvent(QKeyEvent* event)
{
    if (!render_source) return;
    render_source->keyRelease(event);
    update();
}

void CanvasWidget::initializeGL()
{
    initializeOpenGLFunctions();
    QNanoWidget::initializeGL();
}

void CanvasWidget::paint(QNanoPainter* p)
{
    QScreen* screen = this->screen();
    qreal scaleFactor = screen->devicePixelRatio();

    int vw = width();
    int vh = height();

    p->beginFrame(vw*scaleFactor, vh*scaleFactor);
    p->scale(scaleFactor);

    if (render_source)
        render_source->paint(p);

    p->endFrame();
}

void ProjectCanvasWidget::paint(QNanoPainter* p)
{
    QScreen* screen = this->screen();
    qreal scaleFactor = screen->devicePixelRatio();

    int vw = width();
    int vh = height();

    p->beginFrame(vw * scaleFactor, vh * scaleFactor);
    p->scale(scaleFactor);

    if (render_source)
    {
        // Lock simulation processing while we paint simulation/imgui
        ///QMutexLocker locker(&main_window->sim_lock);
        QReadLocker readLock(&main_window->simLock);

        if (!render_to_offscreen)
        {
            render_source->paint(p);
            render_source->onPainted(nullptr);
        }
        else
        {
            double offscreen_aspect_ratio = ((double)render_width / (double)render_height);
            double viewport_aspect_ratio = ((double)vw / (double)vh);
            double off_x = 0, off_y = 0, scale_x = 1, scale_y = 1;

            if (offscreen_aspect_ratio > viewport_aspect_ratio)
            {
                // Offscreen is wider relative to viewport
                scale_x = static_cast<double>(vw) / static_cast<double>(render_width);
                scale_y = scale_x;

                // Center vertically
                off_y = (vh - (render_height * scale_y)) / 2.0;
            }
            else
            {
                // Offscreen is taller relative to viewport
                scale_y = static_cast<double>(vh) / static_cast<double>(render_height);
                scale_x = scale_y;

                // Center horizontally
                off_x = (vw - (render_width * scale_x)) / 2.0;
            }

            auto offscreen_painter = offscreen_nano_painter.begin(render_width, render_height, true);
            render_source->paint(offscreen_painter);
            offscreen_nano_painter.end();

            render_source->onPainted(&offscreen_nano_painter.getPixels());

            offscreen_nano_painter.drawToPainter(p,
                off_x,
                off_y,
                scale_x * render_width,
                scale_y * render_height);
        }
    }

    p->endFrame();
}
