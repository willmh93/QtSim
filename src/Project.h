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
#include "World.h"
#include "helpers.h"
#include "CacheContext.h"

// Graphics
//#include "FFmpegWorker.h"
#include "DrawingContext.h"
#include "graphics.h"

// UI
#include "Options.h"


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
class Canvas2D;

class Layout;
class Viewport;
class Project;

//template<typename T>
//class SceneBase
//{
//};

class Scene
{
    std::random_device rd;
    std::mt19937 gen;

    Options* options = nullptr;

    QElapsedTimer timer_sceneProcess;
    int dt_sceneProcess;
    size_t dt_call_index;
    std::vector<MovingAverage::MA> dt_ma_list;

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

    Scene() : gen(rd()) {}
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
    virtual void sceneProcess() = 0;

    virtual void loadShaders() {}

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

   
    int scene_dt(int average_samples=1)
    {
        if (dt_call_index >= dt_ma_list.size())
            dt_ma_list.push_back(MovingAverage::MA(average_samples));

        auto& ma = dt_ma_list[dt_call_index++];
        return ma.push(dt_sceneProcess);
    }
};

class Viewport : public DrawingContext
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

    OffscreenGLSurface offscreen_surface;
    
public:

    QMatrix4x4 projectionMatrix;
    QMatrix4x4 transformMatrix;
    QMatrix4x4 modelViewMatrix()
    {
        return projectionMatrix * transformMatrix;
    }

    QOpenGLExtraFunctions* beginGL()
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
    }

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

class Project : public QObject
{
    Q_OBJECT

    int sim_uid = -1;

    Options* options;
    Input* input_proxy;

    QThread* ffmpeg_thread = nullptr;
    FFmpegWorker* ffmpeg_worker = nullptr;

    // Recording states
    bool allow_start_recording = true;
    bool record_on_start = false;
    bool recording = false;
    bool window_capture = false;
    bool encoder_busy = false;
    bool encode_next_paint = false;
    
    QImage window_rgba_image;
    std::vector<GLubyte> frame_buffer; // Not changed until next process
    
    Canvas2D* canvas = nullptr;
    QElapsedTimer timer_projectProcess;
    int dt_projectProcess;

    Layout viewports;

    bool shaders_loaded = false;

    void _loadShaders()
    {
        for (Scene* scene : viewports.all_scenes)
            scene->loadShaders();
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
    static SceneType* create()
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

        return scene;
    }

    template<typename SceneType> static SceneType* create(typename SceneType::Config config)
    {
        auto config_ptr = make_shared<typename SceneType::Config>(config);

        SceneType* scene = new SceneType(*config_ptr);
        scene->temporary_environment = config_ptr;
        return scene;
    }

    template<typename SceneType>
    static SceneType* create(shared_ptr<typename SceneType::Config> config)
    {
        if (!config)
            throw "Launch Config wasn't created";

        SceneType* scene = new SceneType(*config);
        scene->temporary_environment = config;
        return scene;
    }

    template<typename SceneType>
    static shared_ptr<SimSceneList> create(int count)
    {
        auto ret = make_shared<SimSceneList>();
        for (int i = 0; i < count; i++)
            ret->push_back(create<SceneType>());
        return ret;
    }

    template<typename SceneType>
    static shared_ptr<SimSceneList> create(int count, typename SceneType::Config config)
    {
        auto ret = make_shared<SimSceneList>();
        for (int i = 0; i < count; i++)
            ret->push_back(create<SceneType>(config));
        return ret;
    }

protected:

    CacheContext cache; // todo: Make dynamic and remove include

    friend class QtSim;
    friend class ProjectWorker;
    friend class Canvas2D;

    ProjectWorker* worker = nullptr;

    bool started = false;
    bool paused = false;

    double canvas_width = 0;
    double canvas_height = 0;

public:

    MouseInfo mouse;

    Layout& newLayout();
    Layout& newLayout(int _viewports_x, int _viewports_y);


    void configure(int sim_uid, Canvas2D *canvas, Options *options);

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

    void _keyPress(QKeyEvent* e)
    {
        for (Scene* scene : viewports.all_scenes)
            scene->keyPressed(e);
    }
    void _keyRelease(QKeyEvent* e)
    {
        for (Scene* scene : viewports.all_scenes)
            scene->keyReleased(e);
    }

    void _draw(QNanoPainter* p);
    void onPainted(const std::vector<GLubyte>* frame);

    bool startRecording();
    void setRecordOnStart(bool b)
    {
        record_on_start = b;
    }

    bool encodeFrame(uint8_t* data);
    void finalizeRecording();

signals:

    void workerReady();
    void frameReady(uint8_t* data);
    void endRecording();
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
