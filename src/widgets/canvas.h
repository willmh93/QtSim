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
    //CanvasWidget* canvas;

    virtual void paint(QNanoPainter* p) {}
    virtual void onPainted(const std::vector<GLubyte>* frame) {}

    virtual void _mouseDown(int x, int y, Qt::MouseButton btn) {}
    virtual void _mouseUp(int x, int y, Qt::MouseButton btn) {}
    virtual void _mouseMove(int x, int y) {}
    virtual void _mouseWheel(int x, int y, int delta) {}
    virtual void _keyPress(QKeyEvent* e) {}
    virtual void _keyRelease(QKeyEvent* e) {}
};

class CanvasWidget : public QNanoWidget, public GLFunctions //GLEngineAbstract
{
    Q_OBJECT;

protected:
    CanvasRenderSource* render_source = nullptr;

public:

    /*// Todo: Completely abstract away Project from Canvas
    Project* project = nullptr;
    MainWindow* main_window = nullptr;

    int offscreen_w;
    int offscreen_h;

    OffscreenNanoPainter offscreen_nano_painter;*/

    explicit CanvasWidget(QWidget* parent = nullptr);
    ~CanvasWidget();

    void setRenderSource(CanvasRenderSource* src)
    {
        render_source = src;
    }

    void initializeGL() override;

    /*void setProject(Project* _sim)
    {
        project = _sim;
    }*/

    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;



    void paint(QNanoPainter* p) override;

    bool render_to_offscreen = false;
};




class RecordableCanvasWidget : public CanvasWidget
{
    Q_OBJECT;

    // Offscreen surface options
    OffscreenNanoPainter offscreen_nano_painter;
    bool render_to_offscreen = false;
    int render_width = 0;
    int render_height = 0;


public:

    MainWindow* main_window = nullptr;

    RecordableCanvasWidget(QWidget *parent) : CanvasWidget(parent)
    {}

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