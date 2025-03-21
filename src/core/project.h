#pragma once

// Qt includes
#include <QObject>
#include <QString>
#include <QColor>
#include <QKeyEvent>
#include <QElapsedTimer>
#include <QFuture>
#include <QThreadPool>
#include <QtConcurrent/QtConcurrent>

#include <QOpenGLShaderProgram>
#include <QMatrix4x4>

// Standard libraries for projects
#include <random>
#include <cmath>
#include <vector>
#include <queue>
#include <limits>
#include <set>
#include <unordered_map>
#include <fstream>
using namespace std;

// Custom includes
#include "camera.h"
#include "helpers.h"
#include "cache_stream.h"

// Graphics
#include "paint_context.h"
#include "graphics.h"
//#include "gl_engine_abstract.h"
#include "canvas.h"

// UI
#include "options.h"


// Provide macros for easy Project registration
template<typename... Ts>
std::vector<QString> VectorizeArgs(Ts&&... args) { return { QString::fromUtf8(std::forward<Ts>(args))... }; }

#define SIM_BEG(ns) namespace ns{
#define SIM_DECLARE(ns, ...) namespace ns { AutoRegisterProject<ns##_Project> register_##ns(VectorizeArgs(__VA_ARGS__));
#define SIM_END(ns) } using ns::ns##_Project;

//#define BASE_SIM(cls) using namespace cls;
// 
//#define CHILD_SORT(...) AutoRegisterProjectOrder register_order(VectorizeArgs(__VA_ARGS__));


// Declarations
class ProjectWorker;
class Options;
class FFmpegWorker;

// Forward declare crossreferences
class QNanoPainter;
class CanvasWidget;

class Layout;
class Viewport;
class Project;

class Scene : public GLFunctions
{
    //Q_OBJECT;

    std::random_device rd;
    std::mt19937 gen;

    Options* options = nullptr;
    Project* project = nullptr; // Project Scene was launched from

    QElapsedTimer timer_sceneProcess;
    int dt_sceneProcess = 0;

    std::vector<MovingAverage::MA> dt_scene_ma_list;
    std::vector<MovingAverage::MA> dt_project_ma_list;
    std::vector<MovingAverage::MA> dt_project_draw_ma_list;

    size_t dt_call_index = 0;
    size_t dt_process_call_index = 0;
    size_t dt_draw_call_index = 0;

    ///OffscreenGLSurface offscreen_surface;

protected:

    friend class Viewport;
    friend class Project;

    std::vector<Viewport*> mounted_to_viewports;

    // Mounting to/from viewport
    void registerMount(Viewport* viewport);
    void registerUnmount(Viewport* viewport);


public:

    std::shared_ptr<void> temporary_environment;

    struct Config {};

    //static_assert(has_launch_config<SceneBase>::value, "Derived class must define InternalClass");

    Project* main = nullptr;
    Camera* camera = nullptr;
    CacheContext* cache = nullptr;

    MouseInfo *mouse;

    Scene() : gen(rd())
    {
    }

    //SceneBase(Config& info) : gen(rd()) {}
    virtual ~Scene() = default;

   
    void mountTo(Viewport* viewport);
    void mountTo(Layout& viewports);
    void mountToAll(Layout& viewports);
    
    virtual void sceneAttributes(Input* options) {}
    virtual void sceneStart() {}
    virtual void sceneMounted(Viewport *ctx) {}
    virtual void sceneStop() {}
    virtual void sceneDestroy() {}
    virtual void sceneProcess() {}

    virtual void initGL() {}

    virtual void viewportProcess(Viewport* ctx) {}
    virtual void viewportDraw(Viewport* ctx) = 0;

    virtual void mouseDown() {}
    virtual void mouseUp() {}
    virtual void mouseMove() {}
    virtual void mouseWheel() {}

    virtual void keyPressed(QKeyEvent* e) {};
    virtual void keyReleased(QKeyEvent* e) {};

    void _destroy()
    {
        sceneDestroy();
    }

    double random(double min = 0, double max = 1)
    {
        std::uniform_real_distribution<> dist(min, max);
        return dist(gen);
    }

    Vec2 Offset(double stage_offX, double stage_offY)
    {
        return camera->toWorldOffset({ stage_offX, stage_offY });
    }

    GLSurface createSurface(int w, int h)
    {
        ///GLSurface surface = std::make_shared<GLSurface>();
        GLSurface surface;

        // Cache currently active FBO for restoring in end()
        //glGetIntegerv(GL_FRAMEBUFFER_BINDING, &surface->prev_fbo);

        //surface->prepare(w, h);
        surface.prepare(w, h);

        //surface->fbo->bind();

        return surface;
    }

    /*void setSurfaceViewport(GLSurface surface, int w, int h)
    {
        // Cache currently active FBO for restoring in end()
        glGetIntegerv(GL_FRAMEBUFFER_BINDING, &surface->prev_fbo);

        // Cache old viewport size for restoring in end()
        GLint viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);
        surface->old_width = viewport[2];  // Width of the viewport
        surface->old_height = viewport[3];  // Height of the viewport

        surface->bind();

        // Clear the framebuffer
        glViewport(0, 0, surface->width, surface->height);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    void releaseSurface(GLSurface surface)
    {
        surface->fbo->release();

        // Restore previously active FBO / Viewport Size
        glBindFramebuffer(GL_FRAMEBUFFER, surface->prev_fbo);
        glViewport(0, 0, surface->old_width, surface->old_height);
    }*/

    int scene_dt(int average_samples=1);

    int project_dt(int average_samples);

    int project_draw_dt(int average_samples);

};

class Viewport : public PaintContext
{
protected:

    friend class Project;
    friend class Scene;
    friend class Layout;

    int viewport_index;
    int viewport_grid_x;
    int viewport_grid_y;

    Layout* layout;
    Scene* scene;
    Options* options;

    double x;
    double y;

    QString print_text;
    QTextStream print_stream;

    ///OffscreenGLSurface offscreen_surface;
    
public:

    QMatrix4x4 projectionMatrix;
    QMatrix4x4 transformMatrix;
    QMatrix4x4 modelViewMatrix()
    {
        return projectionMatrix * transformMatrix;
    }

    ///GLSurface* newSurface()
    ///{
    ///    return offscreen_surface.newSurface();
    ///}

    /*QOpenGLExtraFunctions* beginGL()
    {
        return offscreen_surface.begin(width, height);
    }

    void endGL()
    {
        // Draw FBO to QNanoPainter
        QTransform cur_transform = painter->currentTransform();
        painter->resetTransform();
        painter->transform(default_viewport_transform);

        offscreen_surface.drawToPainter(painter);
        offscreen_surface.end();

        painter->resetTransform();
        painter->transform(cur_transform);
    }*/

    /*void paintSurface(GLSurface surface, double _x, double _y, double _w, double _h)
    {
        // Draw FBO to QNanoPainter
        QTransform cur_transform = painter->currentTransform();
        painter->resetTransform();
        painter->transform(default_viewport_transform);

        auto offscreenImage = QNanoImage::fromFrameBuffer(surface->fbo);

        // Note: This does NOT immediately draw the image to painter fbo.
        // Active FBO must be retained for render pipeline.
        painter->drawImage(offscreenImage, _x, _y, _w, _h);

        painter->resetTransform();
        painter->transform(cur_transform);
    }*/

    Viewport(
        Layout *layout,
        Options *options,
        int viewport_index, 
        int grid_x,
        int grid_y
    );

    ~Viewport();

    void draw(QNanoPainter* p);

    int viewportIndex() { return viewport_index; }
    int viewportGridX() { return viewport_grid_x; }
    int viewportGridY() { return viewport_grid_y; }

    template<typename T, typename... Args>
    T* construct(Args&&... args)
    {
        qDebug() << "Scene constructed. Mounting to Viewport: " << viewport_index;

        scene = new T(std::forward<Args>(args)...);
        scene->registerMount(this);
        return dynamic_cast<T*>(scene);
    }

    template<typename T>
    T* mountScene(T *_sim)
    {
        qDebug() << "Mounting existing scene to Viewport: " << viewport_index;

        scene = _sim;
        scene->registerMount(this);
        return _sim;
    }

    QTextStream& print()
    {
        return print_stream;
    }
};

class Layout
{
    std::vector<Viewport*> viewports;

protected:

    friend class Project;
    friend class Scene;

    Options* options;

    // If 0, viewports expand in that direction. If both 0, expand whole grid.
    int targ_viewports_x = 0;
    int targ_viewports_y = 0;
    int cols = 0;
    int rows = 0;

    std::vector<Scene*> all_scenes;

public:

    using iterator = typename std::vector<Viewport*>::iterator;
    using const_iterator = typename std::vector<Viewport*>::const_iterator;

    ~Layout()
    {
        // Only invoked when you switch project
        clear();
    }

    void clear()
    {
        // layout freed each time you call setLayout
        for (Viewport* p : viewports)
            delete p;

        viewports.clear();
    }

    void setSize(int targ_viewports_x, int targ_viewports_y)
    {
        this->targ_viewports_x = targ_viewports_x;
        this->targ_viewports_y = targ_viewports_y;
    }

    void add(
        int _viewport_index,
        int _grid_x,
        int _grid_y)
    {
        Viewport* viewport = new Viewport(this, options, _viewport_index, _grid_x, _grid_y);
        viewports.push_back(viewport);
    }

    template<typename T, typename... Args>
    Layout *constructAll(Args&&... args)
    {
        for (Viewport* viewport : viewports)
            viewport->construct<T>(std::forward<Args>(args)...);
        return this;
    }

    Viewport* operator[](int i) 
    {
        expandCheck(i+1);
        return viewports[i]; 
    }

    Layout& operator <<(Scene* scene)
    {
        scene->mountTo(*this);
        return *this;
    }

    void resize(int viewport_count);
    void expandCheck(size_t count);

    iterator begin() { return viewports.begin(); }
    iterator end() { return viewports.end(); }

    const_iterator begin() const { return viewports.begin(); }
    const_iterator end() const { return viewports.end(); }

    int count() const {
        return static_cast<int>(viewports.size());
    }
};

struct SimSceneList : public std::vector<Scene*>
{
    void mountTo(Layout& viewports)
    {
        for (size_t i = 0; i < size(); i++)
            at(i)->mountTo(viewports[i]);
    }
};

class RecordManager : public QObject
{
    Q_OBJECT;

    QThread* ffmpeg_thread = nullptr;
    FFmpegWorker* ffmpeg_worker = nullptr;

    bool recording = false;
    bool encoder_busy = false;

public:

    ~RecordManager();

    bool isRecording()
    {
        return recording;
    }

    bool isInitialized();

    bool encoderBusy()
    {
        return encoder_busy;
    }

    bool attachingEncoder()
    {
        return (isRecording() && !isInitialized());
    }

    bool startRecording(
        QString filename,
        Size record_resolution,
        int record_fps,
        bool flip);

    void finalizeRecording()
    {
        emit endRecording();
    }

    bool encodeFrame(uint8_t* data)
    {
        encoder_busy = true;
        emit frameReady(data);
        return true;
    }

public: signals:

    void onFinalized();

private: signals:

    void frameReady(uint8_t* data);
    void workerReady();
    void endRecording();
};

class Project : public CanvasRenderSource
{
    Q_OBJECT;

    int sim_uid = -1;

    Options* options;
    Input* input_proxy;

    
    RecordableCanvasWidget* canvas = nullptr;
    Layout viewports;

    QElapsedTimer timer_projectProcess;
    QElapsedTimer timer_projectDraw;
    int dt_projectProcess = 0;
    int dt_projectDraw = 0;


    // Recording states
    RecordManager record_manager;

    // Keep in project
    bool allow_start_recording = true;
    bool record_on_start = false;
    bool window_capture = false;
    bool encode_next_paint = false; 
    
    QImage window_rgba_image;
    std::vector<GLubyte> frame_buffer; // Not changed until next process

    // Shaders
    bool done_single_process = false;
    bool shaders_loaded = false;

    void _loadShaders()
    {
        for (Scene* scene : viewports.all_scenes)
            scene->initGL();
    }

public:

    // Factory methods
    static std::vector<shared_ptr<ProjectInfo>> &projectInfoList()
    {
        static std::vector<shared_ptr<ProjectInfo>> info_list;
        return info_list;
    }

    static shared_ptr<ProjectInfo> findProjectInfo(int sim_uid)
    {
        for (auto& info : projectInfoList())
        {
            if (info->sim_uid == sim_uid)
                return info;
        }
        return nullptr;
    }

    static void addProjectInfo(const std::vector<QString> &tree_path, const CreatorFunc& func)
    {
        static int factory_sim_index = 0;
        projectInfoList().push_back(std::make_shared<ProjectInfo>(ProjectInfo(
            tree_path, 
            func, 
            factory_sim_index++,
            ProjectInfo::INACTIVE 
        )));
    }
    
    std::shared_ptr<ProjectInfo> getProjectInfo()
    {
        return findProjectInfo(sim_uid);
    }

    void setProjectInfoState(ProjectInfo::State state);

    // Shared Scene creators
    template<typename SceneType>
    SceneType* create()
    {
        auto config = make_shared<typename SceneType::Config>();
        SceneType* scene;

        if constexpr (std::is_constructible_v<SceneType, typename SceneType::Config&>)
        {
            scene = new SceneType(*config);
            scene->temporary_environment = config;

        }
        else
            scene = new SceneType();

        scene->project = this;
        scene->setGLFunctions(canvas->getGLFunctions());

        return scene;
    }

    template<typename SceneType> SceneType* create(typename SceneType::Config config)
    {
        auto config_ptr = make_shared<typename SceneType::Config>(config);

        SceneType* scene = new SceneType(*config_ptr);
        scene->temporary_environment = config_ptr;
        scene->project = this;
        scene->setGLFunctions(canvas->getGLFunctions());

        return scene;
    }

    template<typename SceneType>
    SceneType* create(shared_ptr<typename SceneType::Config> config)
    {
        if (!config)
            throw "Launch Config wasn't created";

        SceneType* scene = new SceneType(*config);
        scene->temporary_environment = config;
        scene->project = this;
        scene->setGLFunctions(canvas->getGLFunctions());

        return scene;
    }

    template<typename SceneType>
    shared_ptr<SimSceneList> create(int count)
    {
        auto ret = make_shared<SimSceneList>();
        for (int i = 0; i < count; i++)
        {
            SceneType* scene = create<SceneType>();
            scene->project = this;
            scene->setGLFunctions(canvas->getGLFunctions());

            ret->push_back(scene);
        }
        return ret;
    }

    template<typename SceneType>
    shared_ptr<SimSceneList> create(int count, typename SceneType::Config config)
    {
        auto ret = make_shared<SimSceneList>();
        for (int i = 0; i < count; i++)
        {
            SceneType* scene = create<SceneType>(config);
            scene->project = this;
            scene->setGLFunctions(canvas->getGLFunctions());

            ret->push_back(scene);
        }
        return ret;
    }

protected:

    CacheContext cache; // todo: Make dynamic and remove include

    friend class MainWindow;
    friend class ProjectWorker;
    friend class CanvasWidget;
    friend class Scene;

    ProjectWorker* worker = nullptr;

    bool started = false;
    bool paused = false;

    double canvas_width = 0;
    double canvas_height = 0;

public:

    MouseInfo mouse;

    Layout& newLayout();
    Layout& newLayout(int _viewports_x, int _viewports_y);


    void configure(int sim_uid, RecordableCanvasWidget* canvas, Options *options);

    void onResize(); // Called on main GUI thread

    int canvasWidth();  // Screen dimensions of canvas
    int canvasHeight(); // Screen dimensions of canvas
    int getFrameTimeDelta() { return dt_projectProcess; }

    void updateViewportRects();
    Vec2 surfaceSize(); // Dimensions of FBO (depends on whether recording or not)

    // Make protected
    void _projectPrepare();
    void _projectStart();
    void _projectStop();
    void _projectPause();
    void _projectDestroy();
    void _projectProcess();

    virtual void projectAttributes(Input* options) {}
    virtual void projectPrepare() = 0;
    virtual void projectStart() {}
    virtual void projectStop() {}
    virtual void projectDestroy() {}


    virtual void postProcess();

    void _mouseDown(int x, int y, Qt::MouseButton btn);
    void _mouseUp(int x, int y, Qt::MouseButton btn);
    void _mouseMove(int x, int y);
    void _mouseWheel(int x, int y, int delta);

    void _keyPress(QKeyEvent* e);
    void _keyRelease(QKeyEvent* e);

    int project_dt(int average_samples = 1);
    int project_draw_dt(int average_samples = 1);

    void paint(QNanoPainter* p);
    void onPainted(const std::vector<GLubyte>* frame);


    ///


    bool startRecording();
    void setRecordOnStart(bool b)
    {
        record_on_start = b;
    }


    void finalizeRecording();

};

template <typename T>
struct AutoRegisterProject
{
    AutoRegisterProject(const std::vector<QString> &tree_path)
    {
        Project::addProjectInfo(tree_path, []() -> Project* {
            return (Project*)(new T());
        });
    }
};
