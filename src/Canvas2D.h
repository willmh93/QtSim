#ifndef CANVAS2D_H
#define CANVAS2D_H

#include <QOpenGLFunctions>
#include "qnanowidget.h"
#include "qnanopainter.h"
#include "graphics.h"



class ProjectBase;

class Canvas2D : public QNanoWidget, protected QOpenGLFunctions
{
    Q_OBJECT;


public:

    // Todo: Completely remove Project from Canvas
    ProjectBase* scene = nullptr;
    int offscreen_w;
    int offscreen_h;

    OffscreenNanoPainter offscreen_nano_painter;// (offscreen_w, offscreen_h);

    QOpenGLFramebufferObject* m_fbo1 = nullptr;

    explicit Canvas2D(QWidget* parent = nullptr);
    ~Canvas2D();

    void initializeGL() override;

    void setProject(ProjectBase* _sim)
    {
        scene = _sim;
    }

    //virtual void process() {}

    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

    //void (*onPainted)() = nullptr;
    void paint(QNanoPainter* p) override;
    //void paint2(QNanoPainter* p);

    bool render_to_offscreen = false;
    //std::vector<GLubyte> frame_data;

};

#endif // CANVAS2D_H
