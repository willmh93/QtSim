#pragma once

// Qt includes
#include <QObject>
#include <QString>
#include <QColor>
#include <QElapsedTimer>
#include <QKeyEvent>

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
#include "FFmpegWorker.h"
#include "DrawingContext.h"
#include "graphics.h"

// UI
#include "Options.h"


// Provide macros for easy Project registration
template<typename... Ts>
std::vector<QString> VectorizeArgs(Ts&&... args) { return { QString::fromUtf8(std::forward<Ts>(args))... }; }

#define SIM_BEG(cls) namespace NS_##cls {
#define SIM_DECLARE(cls, ...) namespace NS_##cls { AutoRegisterProject<cls> register_##cls(VectorizeArgs(__VA_ARGS__));
#define BASE_SIM(cls) using namespace NS_##cls; //typedef NS_##cls::Sim cls;
//#define BASE_SIM(id) namespace NS_##id {
#define SIM_END(cls) } using NS_##cls::cls;

//#define CHILD_SORT(...) AutoRegisterProjectOrder register_order(VectorizeArgs(__VA_ARGS__));

class QNanoPainter;
class Canvas2D;

class Layout;
class Viewport;
class ProjectBase;


class Scene
{
    std::random_device rd;
    std::mt19937 gen;

    Options* options = nullptr;

protected:

    friend class Viewport;

    std::vector<Viewport*> mounted_to_viewports;

    // Mounting to/from viewport
    void registerMount(Viewport* viewport);
    void registerUnmount(Viewport* viewport);


public:

    std::shared_ptr<void> temporary_environment;

    struct LaunchConfig {};

    //static_assert(has_launch_config<Scene>::value, "Derived class must define InternalClass");

    ProjectBase* main = nullptr;
    Camera* camera = nullptr;
    CacheContext* cache = nullptr;

    MouseInfo *mouse;

    Scene() : gen(rd()) {}
    Scene(LaunchConfig& info) : gen(rd()) {}
    virtual ~Scene() = default;

    void mountTo(Viewport* viewport);
    void mountTo(Layout& viewports);
    void mountToAll(Layout& viewports);
    
    virtual void sceneAttributes(Options* options) {}
    virtual void sceneStart() {}
    virtual void sceneMounted(Viewport *ctx) {}
    virtual void sceneStop() {}
    virtual void sceneDestroy() {}
    virtual void sceneProcess() = 0;

    virtual void viewportProcess(Viewport* ctx) {}
    virtual void viewportDraw(Viewport* ctx) = 0;

    virtual void mouseDown() {}
    virtual void mouseUp() {}
    virtual void mouseMove() {}
    virtual void mouseWheel() {}

    virtual void keyPressed(QKeyEvent *e) {}
    virtual void keyReleased(QKeyEvent* e) {}

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
};

class Viewport : public DrawingContext
{
protected:

    friend class ProjectBase;
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
    
public:

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
};

class Layout
{
    std::vector<Viewport*> viewports;

protected:

    friend class ProjectBase;
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

class ProjectBase : public QObject
{
    Q_OBJECT

    int sim_uid = -1;

    Options* options;

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
    QElapsedTimer dt_timer;
    int frame_dt;

    Layout viewports;

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

    void setProjectInfoState(ProjectInfo::State state)
    {
        getProjectInfo()->state = state;
        options->refreshTreeUI();
    }

protected:

    CacheContext cache;

    friend class QtSim;
    friend class Canvas2D;

    bool started;
    bool paused;

public:

    MouseInfo mouse;

    Layout& newLayout();
    Layout& newLayout(int _viewports_x, int _viewports_y);


    void configure(int sim_uid, Canvas2D *canvas, Options *options);

    int canvasWidth();  // Screen dimensions of canvas
    int canvasHeight(); // Screen dimensions of canvas
    int getFrameTimeDelta() { return frame_dt; }

    void updateViewportRects();
    Vec2 surfaceSize(); // Dimensions of FBO (depends on whether recording or not)

    void _projectPrepare();
    void _projectStart();
    void _projectStop();
    void _projectPause();
    void _projectDestroy();
    void _projectProcess();

    virtual void projectAttributes(Options* options) {}
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

struct SimSceneList : public std::vector<Scene*>
{
    void mountTo(Layout& viewports)
    {
        for (size_t i = 0; i < size(); i++)
            at(i)->mountTo(viewports[i]);
    }
};

// Subclass ProjectBase so Project contains scene type information
template <typename T>
class Project : public ProjectBase
{
public:
    typedef T SceneType;
    typedef typename T::LaunchConfig LaunchConfig;

    // Methods for spawning new scenes

    static SceneType* createScene()
    {
        shared_ptr<LaunchConfig> config = make_shared<LaunchConfig>();
        SceneType* scene;

        if constexpr (std::is_constructible_v<SceneType, LaunchConfig&>)
        {
            scene = new SceneType(*config);
            scene->temporary_environment = config;
        }
        else
            scene = new SceneType();

        return scene;
    }

    static SceneType* createScene(LaunchConfig config)
    {
        shared_ptr<LaunchConfig> config_ptr = make_shared<LaunchConfig>(config);

        SceneType* scene = new SceneType(*config_ptr);
        scene->temporary_environment = config_ptr;
        return scene;
    }

    static SceneType* createScene(shared_ptr<LaunchConfig> config)
    {
        if (!config)
            throw "Launch Config wasn't created";

        SceneType* scene = new SceneType(*config);
        scene->temporary_environment = config;
        return scene;
    }

    static shared_ptr<SimSceneList> makeScenes(int count)
    {
        auto ret = make_shared<SimSceneList>();
        for (int i = 0; i < count; i++)
            ret->push_back(createScene());
        return ret;
    }
};

template <typename T>
struct AutoRegisterProject
{
    AutoRegisterProject(const std::vector<QString> &tree_path)
    {
        ProjectBase::addProjectInfo(tree_path, []() -> ProjectBase* {
            return (ProjectBase*)(new T());
        });
    }
};
