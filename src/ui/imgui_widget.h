#ifndef IMGUIOPENGLWIDGET_H
#define IMGUIOPENGLWIDGET_H

#include <QOpenGLExtraFunctions>
#include <QObject>
#include <QPoint>
#include <memory>

class QMouseEvent;
class QWheelEvent;
class QKeyEvent;

#include "imgui_custom.h"

namespace QtImGui {

class WindowWrapper {
public:
    virtual ~WindowWrapper() {}
    virtual void installEventFilter(QObject* object) = 0;
    virtual QSize size() const = 0;
    virtual qreal devicePixelRatio() const = 0;
    virtual bool isActive() const = 0;
    virtual QPoint mapFromGlobal(const QPoint& p) const = 0;
    virtual QObject* object() = 0;

    virtual void setCursorShape(Qt::CursorShape shape) = 0;
    virtual void setCursorPos(const QPoint& local_pos) = 0;
};

class ImGuiRenderer : public QObject, QOpenGLExtraFunctions {
    Q_OBJECT
public:
    void initialize(WindowWrapper* window);
    void newFrame();
    void render();
    bool eventFilter(QObject* watched, QEvent* event);

    static ImGuiRenderer* instance();

public:
    ImGuiRenderer();
    ~ImGuiRenderer();

private:
    void onMousePressedChange(QMouseEvent* event);
    void onWheel(QWheelEvent* event);
    void onKeyPressRelease(QKeyEvent* event);

    void updateCursorShape(const ImGuiIO& io);
    void setCursorPos(const ImGuiIO& io);

    void renderDrawList(ImDrawData* draw_data);
    bool createFontsTexture();
    bool createDeviceObjects();

    std::unique_ptr<WindowWrapper> m_window;
    double       g_Time = 0.0f;
    bool         g_MousePressed[3] = { false, false, false };
    float        g_MouseWheel;
    float        g_MouseWheelH;
    GLuint       g_FontTexture = 0;
    int          g_ShaderHandle = 0, g_VertHandle = 0, g_FragHandle = 0;
    int          g_AttribLocationTex = 0, g_AttribLocationProjMtx = 0;
    int          g_AttribLocationPosition = 0, g_AttribLocationUV = 0, g_AttribLocationColor = 0;
    unsigned int g_VboHandle = 0, g_VaoHandle = 0, g_ElementsHandle = 0;

    ImGuiContext* g_ctx = nullptr;
};

typedef void* RenderRef;

#ifdef QT_WIDGETS_LIB
RenderRef initialize(QWidget* window, bool defaultRender = true);
#endif

RenderRef initialize(QWindow* window, bool defaultRender = true);
void newFrame(RenderRef ref = nullptr);
void render(RenderRef ref = nullptr);

}
#endif // IMGUIOPENGLWIDGET_H
