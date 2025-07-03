#pragma once

// Qt includes
#include <QObject>
#include <QString>
#include <QColor>
#include <QKeyEvent>
#include <QElapsedTimer>
#include <QFuture>
#include <QThreadPool>
#include <QReadWriteLock>
#include <QtConcurrent/QtConcurrent>

#include <QOpenGLShaderProgram>
#include <QMatrix4x4>

// Standard libraries for projects
#include <type_traits>
#include <random>
#include <cmath>
#include <vector>
#include <queue>
#include <limits>
#include <set>
#include <unordered_map>
#include <fstream>
#include <typeindex>
using namespace std;

// Custom includes
#include "camera.h"
#include "helpers.h"
#include "cache_stream.h"

// Graphics
#include "paint_context.h"
#include "graphics.h"
#include "canvas.h"
#include "ffmpeg_worker.h"

// UI
#include "options.h"
#include "imgui_custom.h"


// Provide macros for easy Project registration
template<typename... Ts>
std::vector<QString> VectorizeArgs(Ts&&... args) { return { QString::fromUtf8(std::forward<Ts>(args))... }; }

#define SIM_BEG(ns) namespace ns{
#define SIM_DECLARE(ns, ...) namespace ns { AutoRegisterProject<ns##_Project> register_##ns(VectorizeArgs(__VA_ARGS__));
#define SIM_END(ns) } using ns::ns##_Project;

//#define BASE_SIM(cls) using namespace cls;
// 
//#define CHILD_SORT(...) AutoRegisterProjectOrder register_order(VectorizeArgs(__VA_ARGS__));


#include <unordered_map>
#include <typeindex>
#include <typeinfo>
#include <mutex>
#include <functional>
#include <utility>

class VariableChangedTracker
{
    
    /// Holds the two maps for a particular type T
    template <typename T>
    struct StateMapPair {
        std::unordered_map<T*, T> current;
        std::unordered_map<T*, T> previous;
    };

    /// A simple record to store how we clear and commit for each type
    struct ClearCommit {
        std::function<void()> clearer;
        std::function<void()> committer;
    };

    mutable std::unordered_map<std::type_index, ClearCommit> registry_;
    std::mutex mutex_;


    /// The per-type map is allocated once per type T via a static local; we also
    /// register it (lazily) so that global clear/commit can iterate over it.
    template <typename T>
    StateMapPair<T>& getStateMap() const
    {
        static StateMapPair<T> maps;
        static bool registered = registerType<T>(maps);
        (void)registered; // silence unused warning
        return maps;
    }

    /// Register the given maps clear/commit methods in our registry
    template <typename T>
    bool registerType(StateMapPair<T>& maps) const
    {
        //std::lock_guard<std::mutex> lock(mutex_);

        registry_[std::type_index(typeid(T))] = {
            // Clearer: reset both 'current' and 'previous'
            [&]() {
                maps.current.clear();
                maps.previous.clear();
            },
            // Committer: copy 'current' => 'previous'
            [&]() {
                maps.previous = maps.current;
            }
        };
        return true;
    }

public:

    VariableChangedTracker() = default;

    /// Obtain a single global instance if you want to use it as a singleton
    static VariableChangedTracker& instance()
    {
        static VariableChangedTracker s_instance;
        return s_instance;
    }

    /// Returns true if 'var' differs from its last committed value, false otherwise.
    template <typename T>
    bool variableChanged(T& var) const
    {
        using NonConstT = typename std::remove_const<T>::type;
        auto& maps = getStateMap<NonConstT>();

        // Remove constness from the address
        auto key = const_cast<NonConstT*>(&var);

        auto it = maps.previous.find(key);
        if (it != maps.previous.end())
        {
            // Compare against the previously committed value
            bool changed = (var != it->second);
            // Always update 'current' so that commit will be correct
            maps.current[key] = var;
            return changed;
        }
        else
        {
            // First time we see this variable; store it but it doesn't "count" as changed yet
            maps.current[key] = var;
            return false;
        }
    }

    /// Returns true if any variable among args... has changed
    template <typename... Args>
    bool anyChanged(Args&&... args) const
    {
        return (variableChanged(std::forward<Args>(args)) || ...);
    }

    /// Explicitly update the 'current' value of a single variable
    template<typename T>
    void commitValue(T& var)
    {
        getStateMap<T>().current[&var] = var;
    }

    /// Explicitly update the 'current' values of all variables passed in
    template<typename... Args>
    void commitAll(Args&&... args)
    {
        (commitValue(std::forward<Args>(args)), ...);
    }

    /// Clears all tracked data (for every type) so we effectively start fresh
    void variableChangedClearMaps()
    {
        //std::lock_guard<std::mutex> lock(mutex_);
        for (auto& [_, cc] : registry_)
        {
            cc.clearer();
        }
    }

    /// Commits all tracked data (for every type), i.e. current => previous
    /// so that subsequent calls to variableChanged() compare against the newly committed data
    void variableChangedUpdateCurrent()
    {
        //std::lock_guard<std::mutex> lock(mutex_);
        for (auto& [_, cc] : registry_)
        {
            cc.committer();
        }
    }
};


// Declarations
class ProjectWorker;
class Options;
class ImOptions;

// Forward declare crossreferences
class QNanoPainter;
class CanvasWidget;

class Layout;
class Viewport;
class Project;

class Scene : public GLFunctions, public VariableChangedTracker
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

    int scene_index;
    std::vector<Viewport*> mounted_to_viewports;

    // Mounting to/from viewport
    void registerMount(Viewport* viewport);
    void registerUnmount(Viewport* viewport);


public:

    bool imgui_scene_attributes_visible = true;

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

    virtual std::string name()
    {
        return "Scene";
    }

    int sceneIndex()
    {
        return scene_index;
    }

    //SceneBase(Config& info) : gen(rd()) {}
    virtual ~Scene() = default;

   
    void mountTo(Viewport* viewport);
    void mountTo(Layout& viewports);
    void mountToAll(Layout& viewports);
    
    virtual void sceneAttributes() {}
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

    bool keyPressed(int key);

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

    int project_dt(int average_samples=1);

    int project_draw_dt(int average_samples);

    

    /*std::unordered_map<bool*, bool>     old_bools;
    std::unordered_map<int*, int>       old_ints;
    std::unordered_map<double*, double> old_doubles;


    template <typename... Args>
    bool anyChanged(Args&&... args)
    {
        return (variableChanged(std::forward<Args>(args)) || ...);
    }



    bool variableChanged(bool& var)
    {
        auto it = old_bools.find(&var);
        if (it != old_bools.end())
            return var != it->second;
        else
            old_bools[&var] = var;
        return false;
    }

    bool variableChanged(int& var)
    {
        auto it = old_ints.find(&var);
        if (it != old_ints.end())
            return var != it->second;
        else
            old_ints[&var] = var;
        return false;
    }

    bool variableChanged(double& var)
    {
        auto it = old_doubles.find(&var);
        if (it != old_doubles.end())
            return var != it->second;
        else
            old_doubles[&var] = var;
        return false;
    }*/

    /*template<typename T>
    std::unordered_map<T*, T>& getStateMap()
    {
        static std::unordered_map<T*, T> map;
        return map;
    }

    template<typename T>
    bool variableChanged(T& var)
    {
        auto& map = getStateMap<T>();
        auto it = map.find(&var);
        if (it != map.end())
            return var != it->second;
        else
            map[&var] = var;
        return false;
    }

    void variableChangedUpdateCurrent()
    {
        for (auto& item : old_bools)    item.second = *item.first;
        for (auto& item : old_ints)     item.second = *item.first;
        for (auto& item : old_doubles)  item.second = *item.first;
    }

    void variableChangedClearMaps()
    {
        old_bools.clear();
        old_ints.clear();
        old_doubles.clear();
    }*/

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

    const std::vector<Scene*>& scenes()
    {
        return all_scenes;
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

class Project : public CanvasRenderSource
{
    Q_OBJECT;

    int sim_uid = -1;

    Options* options;
    
    ProjectCanvasWidget* canvas = nullptr;
    Layout viewports;
    int scene_counter = 0;

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
        scene->scene_index = scene_counter++;
        scene->setGLFunctions(canvas->getGLFunctions());

        return scene;
    }

    template<typename SceneType> SceneType* create(typename SceneType::Config config)
    {
        auto config_ptr = make_shared<typename SceneType::Config>(config);

        SceneType* scene = new SceneType(*config_ptr);
        scene->temporary_environment = config_ptr;
        scene->scene_index = scene_counter++;
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
        scene->scene_index = scene_counter++;
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
    friend class ImOptions;
    friend class ProjectWorker;
    friend class CanvasWidget;
    friend class Scene;

    ProjectWorker* worker = nullptr;

    bool started = false;
    bool paused = false;

    double canvas_width = 0;
    double canvas_height = 0;

protected:

    void _projectPrepare();
    void _projectStart();
    void _projectStop();
    void _projectPause();
    void _projectDestroy();
    void _projectProcess();

public:

    MouseInfo mouse;

    Layout& newLayout();
    Layout& newLayout(int _viewports_x, int _viewports_y);
    Layout& currentLayout()
    {
        return viewports;
    }

    void configure(int sim_uid, ProjectCanvasWidget* canvas, Options *options);

    void onResize(); // Called on main GUI thread
    int canvasWidth();  // Screen dimensions of canvas
    int canvasHeight(); // Screen dimensions of canvas
    int getFrameTimeDelta() { return dt_projectProcess; }

    void updateViewportRects();
    Vec2 surfaceSize(); // Dimensions of FBO (depends on whether recording or not)

    virtual void projectAttributes() {}
    virtual void projectPrepare() = 0;
    virtual void projectStart() {}
    virtual void projectStop() {}
    virtual void projectDestroy() {}

    virtual void postProcess();

    void mouseDown(int x, int y, Qt::MouseButton btn);
    void mouseUp(int x, int y, Qt::MouseButton btn);
    void mouseMove(int x, int y);
    void mouseWheel(int x, int y, int delta);

    std::unordered_map<int, bool> key_pressed;
    bool keyPressed(int key) {
        if (key_pressed.count(key) == 0)
            key_pressed[key] = false;
        return key_pressed[key];
    }

    void keyPress(QKeyEvent* e);
    void keyRelease(QKeyEvent* e);

    void paint(QNanoPainter* p);
    void onPainted(const std::vector<GLubyte>* frame);

    ///

    void setRecordOnStart(bool b)
    {
        record_on_start = b;
    }

    bool startRecording();
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
