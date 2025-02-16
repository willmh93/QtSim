#pragma once

// Qt includes
#include <QObject>
#include <QString>
#include <QColor>
#include <QElapsedTimer>

// Standard libraries for simulations
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


// Provide macros for easy Simulation registration
template<typename... Ts>
std::vector<QString> VectorizeArgs(Ts&&... args) { return { QString::fromUtf8(std::forward<Ts>(args))... }; }

#define SIM_BEG(cls) namespace NS_##cls {
#define SIM_DECLARE(cls, ...) namespace NS_##cls { AutoRegisterSimulation<cls> register_##cls(VectorizeArgs(__VA_ARGS__));
#define BASE_SIM(cls) using namespace NS_##cls; //typedef NS_##cls::Sim cls;
//#define BASE_SIM(id) namespace NS_##id {
#define SIM_END(cls) } using NS_##cls::cls;

#define CHILD_SORT(...) AutoRegisterSimulationOrder register_order(VectorizeArgs(__VA_ARGS__));

class QNanoPainter;
class Canvas2D;

class Layout;
class Panel;
class SimulationBase;


class SimulationInstance
{
    std::random_device rd;
    std::mt19937 gen;

    Options* options = nullptr;

protected:

    friend class Panel;

    std::vector<Panel*> mounted_to_panels;

    // Mounting to/from panel
    void registerMount(Panel* panel);
    void registerUnmount(Panel* panel);


public:

    std::shared_ptr<void> temporary_environment;

    struct LaunchConfig {};

    //static_assert(has_launch_config<SimulationInstance>::value, "Derived class must define InternalClass");

    SimulationBase* main = nullptr;
    Camera* camera = nullptr;
    CacheContext* cache = nullptr;

    MouseInfo *mouse;

    SimulationInstance() : gen(rd()) {}
    SimulationInstance(LaunchConfig& info) : gen(rd()) {}
    virtual ~SimulationInstance() = default;

    void mountTo(Panel* panel);
    void mountTo(Layout& panels);
    void mountToAll(Layout& panels);
    
    virtual void instanceAttributes(Options* options) {}
    virtual void start() {}
    virtual void mount(Panel *ctx) {}
    virtual void stop() {}
    virtual void destroy() {}
    virtual void processScene() = 0;
    virtual void processPanel(Panel* ctx) {}
    virtual void draw(Panel* ctx) = 0;

    virtual void mouseDown() {}
    virtual void mouseUp() {}
    virtual void mouseMove() {}
    virtual void mouseWheel() {}

    void _destroy()
    {
        destroy();
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

class Panel : public DrawingContext
{
protected:

    friend class SimulationBase;
    friend class SimulationInstance;
    friend class Layout;

    int panel_index;
    int panel_grid_x;
    int panel_grid_y;

    Layout* layout;
    SimulationInstance* sim;
    Options* options;

    double x;
    double y;
    
public:

    Panel(
        Layout *layout,
        Options *options,
        int panel_index, 
        int grid_x,
        int grid_y
    );

    ~Panel();

    void draw(QNanoPainter* p);

    int panelIndex() { return panel_index; }
    int panelGridX() { return panel_grid_x; }
    int panelGridY() { return panel_grid_y; }

    template<typename T, typename... Args>
    T* construct(Args&&... args)
    {
        qDebug() << "Instance constructed. Mounting to Panel: " << panel_index;

        sim = new T(std::forward<Args>(args)...);
        sim->registerMount(this);
        return dynamic_cast<T*>(sim);
    }

    template<typename T>
    T* mountInstance(T *_sim)
    {
        qDebug() << "Mounting existing instance to Panel: " << panel_index;

        sim = _sim;
        sim->registerMount(this);
        return _sim;
    }
};

class Layout
{
    std::vector<Panel*> panels;

protected:

    friend class SimulationBase;
    friend class SimulationInstance;

    Options* options;

    // If 0, panels expand in that direction. If both 0, expand whole grid.
    int targ_panels_x = 0;
    int targ_panels_y = 0;
    int cols = 0;
    int rows = 0;

    std::vector<SimulationInstance*> all_instances;

public:

    using iterator = typename std::vector<Panel*>::iterator;
    using const_iterator = typename std::vector<Panel*>::const_iterator;

    ~Layout()
    {
        // Only invoked when you switch simulation
        clear();
    }

    void clear()
    {
        // layout freed each time you call setLayout
        for (Panel* p : panels)
            delete p;

        panels.clear();
    }

    void setSize(int targ_panels_x, int targ_panels_y)
    {
        this->targ_panels_x = targ_panels_x;
        this->targ_panels_y = targ_panels_y;
    }

    void add(
        int _panel_index,
        int _grid_x,
        int _grid_y)
    {
        Panel* panel = new Panel(this, options, _panel_index, _grid_x, _grid_y);
        panels.push_back(panel);
    }

    template<typename T, typename... Args>
    Layout *constructAll(Args&&... args)
    {
        for (Panel* panel : panels)
            panel->construct<T>(std::forward<Args>(args)...);
        return this;
    }

    Panel* operator[](int i) 
    {
        expandCheck(i+1);
        return panels[i]; 
    }

    Layout& operator <<(SimulationInstance* instance)
    {
        instance->mountTo(*this);
        return *this;
    }

    void resize(int panel_count);
    void expandCheck(size_t count);

    iterator begin() { return panels.begin(); }
    iterator end() { return panels.end(); }

    const_iterator begin() const { return panels.begin(); }
    const_iterator end() const { return panels.end(); }

    int count() const {
        return static_cast<int>(panels.size());
    }

};

class SimulationBase : public QObject
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

    Layout panels;

public:

    // Factory methods
    static std::vector<shared_ptr<SimulationInfo>> &simulationInfoList()
    {
        static std::vector<shared_ptr<SimulationInfo>> info_list;
        return info_list;
    }

    static shared_ptr<SimulationInfo> findSimulationInfo(int sim_uid)
    {
        for (auto& info : simulationInfoList())
        {
            if (info->sim_uid == sim_uid)
                return info;
        }
        return nullptr;
    }

    static void addSimulationInfo(const std::vector<QString> &tree_path, const CreatorFunc& func)
    {
        static int factory_sim_index = 0;
        simulationInfoList().push_back(std::make_shared<SimulationInfo>(SimulationInfo(
            tree_path, 
            func, 
            factory_sim_index++,
            SimulationInfo::INACTIVE 
        )));
    }
    
    std::shared_ptr<SimulationInfo> getSimulationInfo()
    {
        return findSimulationInfo(sim_uid);
    }

    void setSimulationInfoState(SimulationInfo::State state)
    {
        getSimulationInfo()->state = state;
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
    Layout& newLayout(int _panels_x, int _panels_y);


    void configure(int sim_uid, Canvas2D *canvas, Options *options);

    int canvasWidth();  // Screen dimensions of canvas
    int canvasHeight(); // Screen dimensions of canvas
    int getFrameTimeDelta() { return frame_dt; }

    void updatePanelRects();
    Vec2 surfaceSize(); // Dimensions of FBO (depends on whether recording or not)

    virtual void projectAttributes(Options* options) {}

    void _prepare();
    void _start();
    void _stop();
    void _pause();
    void _destroy();
    void _process();

    virtual void prepare() = 0;
    virtual void start() {}
    virtual void stop() {}
    virtual void destroy() {}

    virtual void postProcess();

    void _mouseDown(int x, int y, Qt::MouseButton btn);
    void _mouseUp(int x, int y, Qt::MouseButton btn);
    void _mouseMove(int x, int y);
    void _mouseWheel(int x, int y, int delta);

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

struct SimInstanceList : public std::vector<SimulationInstance*>
{
    void mountTo(Layout& panels)
    {
        for (size_t i = 0; i < size(); i++)
            at(i)->mountTo(panels[i]);
    }
};

// Subclass SimulationBase so Simulation contains instance type information
template <typename T>
class Simulation : public SimulationBase
{
public:
    typedef T Instance;
    typedef typename T::LaunchConfig LaunchConfig;

    // Methods for spawning new instances

    static Instance* makeInstance()
    {
        shared_ptr<LaunchConfig> config = make_shared<LaunchConfig>();
        Instance* instance;

        if constexpr (std::is_constructible_v<Instance, LaunchConfig&>)
        {
            instance = new Instance(*config);
            instance->temporary_environment = config;
        }
        else
            instance = new Instance();

        return instance;
    }

    static Instance* makeInstance(LaunchConfig config)
    {
        shared_ptr<LaunchConfig> config_ptr = make_shared<LaunchConfig>(config);

        Instance* instance = new Instance(*config_ptr);
        instance->temporary_environment = config_ptr;
        return instance;
    }

    static Instance* makeInstance(shared_ptr<LaunchConfig> config)
    {
        if (!config)
            throw "Launch Config wasn't created";

        Instance* instance = new Instance(*config);
        instance->temporary_environment = config;
        return instance;
    }

    static shared_ptr<SimInstanceList> makeInstances(int count)
    {
        auto ret = make_shared<SimInstanceList>();
        for (int i = 0; i < count; i++)
            ret->push_back(makeInstance());
        return ret;
    }
};

template <typename T>
struct AutoRegisterSimulation
{
    AutoRegisterSimulation(const std::vector<QString> &tree_path)
    {
        SimulationBase::addSimulationInfo(tree_path, []() -> SimulationBase* {
            return (SimulationBase*)(new T());
        });
    }
};
