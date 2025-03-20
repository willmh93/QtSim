#pragma once
#include <QOpenGLFunctions>
#include <QOpenGLWidget>

#include "qnanopainter.h"

class MainWindow;
class Project;

/*struct GLFunctions : public QOpenGLExtraFunctions
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

class GLEngineAbstract : public QOpenGLWidget, public GLFunctions
{
    Q_OBJECT;

    friend class MainWindow;
    friend class CanvasWidget;

    Project* project = nullptr;
    MainWindow* main_window = nullptr;

    QNanoPainter* m_painter = nullptr;
    bool m_setupDone = false;

public:

    explicit GLEngineAbstract(QWidget* parent = nullptr) : QOpenGLWidget(parent)
    {
    }
    virtual ~GLEngineAbstract() override 
    {
    }

    void setProject(Project* _sim)
    {
        project = _sim;
    }

    static QNanoPainter* getInstance()
    {

    }

protected:

    void initializeGL() override
    {
        m_painter = QNanoPainter::getInstance();
        initializeOpenGLFunctions();
    }

    virtual void paint(QNanoPainter* painter)
    {

    }

    void prepaint()
    {
        glClearColor(0, 0, 0, 255);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        nvgBeginFrame(m_painter->nvgCtx(), width(), height(), devicePixelRatio());
    }

    void postpaint()
    {
        nvgEndFrame(m_painter->nvgCtx());
    }

    void paintGL() override
    {
        if (!m_painter)
            return;

        if ((width() > 0 && height() > 0) || m_setupDone) 
        {
            m_setupDone = true;
            //prepaint();
            paint(m_painter);
            //postpaint();
        }
    }

};*/

/*class GLEngine : public GLEngineAbstract
{
protected:

    void initializeGL() override
    {
    }

    void paintGL() override
    {
        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    }
};

class GLEngineNanoVG : public GLEngineAbstract, public QNanoWidget
{
protected:

    void initializeGL() override
    {
        QNanoWidget::initializeGL();
    }

    void paintGL() override
    {
        QNanoWidget::paintGL();
    }
};*/