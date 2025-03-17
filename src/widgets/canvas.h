#pragma once
#include <QOpenGLFunctions>
#include "qnanowidget.h"
#include "qnanopainter.h"
#include "graphics.h"

class MainWindow;
class Project;

class Canvas2D : public QNanoWidget, protected QOpenGLFunctions
{
    Q_OBJECT;

public:

    // Todo: Completely abstract away Project from Canvas
    Project* project = nullptr;
    MainWindow* main_window = nullptr;

    int offscreen_w;
    int offscreen_h;

    OffscreenNanoPainter offscreen_nano_painter;
    QOpenGLFramebufferObject* m_fbo1 = nullptr;

    explicit Canvas2D(QWidget* parent = nullptr);
    ~Canvas2D();

    void initializeGL() override;

    void setProject(Project* _sim)
    {
        project = _sim;
    }

    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;

    void paint(QNanoPainter* p) override;

    bool render_to_offscreen = false;
};
