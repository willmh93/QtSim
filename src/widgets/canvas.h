#pragma once
#include <QOpenGLFunctions>
#include "qnanowidget.h"
#include "qnanopainter.h"
#include "graphics.h"

class MainWindow;
class Project;

struct GLFunctions : public QOpenGLExtraFunctions
{
    QOpenGLFunctionsPrivate* getGLFunctions()
    {
        return this->d_ptr;
    }

    void setGLFunctions(QOpenGLFunctionsPrivate* p)
    {
        this->d_ptr = p;
    }
};



class CanvasRenderSource : public QObject
{
public:
    virtual void paint(QNanoPainter* p) {}
    virtual void onPainted(const std::vector<GLubyte>* frame) {}

    virtual void mouseDown(int x, int y, Qt::MouseButton btn) {}
    virtual void mouseUp(int x, int y, Qt::MouseButton btn) {}
    virtual void mouseMove(int x, int y) {}
    virtual void mouseWheel(int x, int y, int delta) {}
    virtual void keyPress(QKeyEvent* e) {}
    virtual void keyRelease(QKeyEvent* e) {}
};

class CanvasWidget : public QNanoWidget, public GLFunctions
{
    Q_OBJECT;

    bool render_to_offscreen = false;

protected:

    CanvasRenderSource* render_source = nullptr;

    explicit CanvasWidget(QWidget* parent = nullptr);

    void initializeGL() override;

    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;

    void paint(QNanoPainter* p) override;

public:

    void setRenderSource(CanvasRenderSource* src) {
        render_source = src;
    }

    void setBackground(int r, int g, int b) {
        setFillColor(QColor(r, g, b));
    }
};

class ProjectCanvasWidget : public CanvasWidget
{
    Q_OBJECT;

    // Offscreen surface options
    OffscreenNanoPainter offscreen_nano_painter;
    bool render_to_offscreen = false;
    int render_width = 0;
    int render_height = 0;

public:

    MainWindow* main_window = nullptr;

    ProjectCanvasWidget(QWidget *parent) : CanvasWidget(parent)
    {
        setBackground(10, 10, 15);
    }

    void useOffscreenSurface(bool b)
    {
        render_to_offscreen = b;
    }

    void setTargetResolution(int w, int h)
    {
        render_width = w;
        render_height = h;
    }

    void paint(QNanoPainter* p);
};