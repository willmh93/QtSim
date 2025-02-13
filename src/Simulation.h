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
#define SIM_END }

#define SIBLING_ORDER(...) AutoRegisterSimulationOrder register_order(VectorizeArgs(__VA_ARGS__));

class QNanoPainter;
class Canvas2D;

class Layout;
class Panel;
class Simulation;

class SimulationInstance
{
    std::random_device rd;
    std::mt19937 gen;

    Options* options = nullptr;

protected:

    friend class Panel;

    std::vector<Panel*> mounted_to_panels;

public:


    Simulation* main = nullptr;
    Camera* camera = nullptr;
    CacheContext* cache = nullptr;

    MouseInfo mouse;

    SimulationInstance() : gen(rd()) {}
    virtual ~SimulationInstance() = default;

    // Mounting to/from panel
    void mountToPanel(Panel* panel);
    void unmountFromPanel(Panel* panel);

    virtual void instanceAttributes(Options* options) {}
    virtual void start() {}
    virtual void mount(Panel *ctx) {}
    virtual void stop() {}
    virtual void destroy() {}
    virtual void processScene() = 0;
    virtual void processPanel(Panel* ctx) {}
    virtual void draw(Panel* ctx) = 0;

    virtual void postProcess()
    {
        // Keep delta until entire frame processed and drawn
        mouse.scroll_delta = 0;
    }

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

    friend class Simulation;
    friend class SimulationInstance;

    int panel_index;
    int panel_grid_x;
    int panel_grid_y;

    Layout* layout;
    SimulationInstance* sim;
    Options* options;
    Simulation* main;

    double x;
    double y;
    
public:

    Panel(
        Layout *layout, 
        Simulation *main, 
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
        sim->mountToPanel(this);
        return dynamic_cast<T*>(sim);
    }

    template<typename T>
    T* mountInstance(T *_sim)
    {
        qDebug() << "Mounting existing instance to Panel: " << panel_index;

        sim = _sim;
        sim->mountToPanel(this);
        return _sim;
    }
};

class Layout
{
    std::vector<Panel*> panels;

protected:

    friend class Simulation;
    friend class SimulationInstance;

    Simulation* main;
    Options* options;

    int panels_x;
    int panels_y;

    std::vector<SimulationInstance*> all_instances;

public:

    using iterator = typename std::vector<Panel*>::iterator;
    using const_iterator = typename std::vector<Panel*>::const_iterator;

    ~Layout()
    {
        // Only invoked when you SWITCH simulation
        clear();
    }

    void clear()
    {
        // Panels freed each time you call setLayout
        for (Panel* p : panels)
            delete p;

        panels.clear();
    }

    void add(
        int _panel_index,
        int _grid_x,
        int _grid_y)
    {
        Panel* panel = new Panel(this, main, options, _panel_index, _grid_x, _grid_y);
        panels.push_back(panel);
    }

    template<typename T, typename... Args>
    Layout *constructAll(Args&&... args)
    {
        for (Panel* panel : panels)
            panel->construct<T>(std::forward<Args>(args)...);
        return this;
    }

    Panel* operator[](int i) { return panels[i]; }

    iterator begin() { return panels.begin(); }
    iterator end() { return panels.end(); }

    const_iterator begin() const { return panels.begin(); }
    const_iterator end() const { return panels.end(); }
};

class Simulation : public QObject
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
    bool encoder_busy = false;
    bool encode_next_paint = false;
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

    static void hintOrder(const std::vector<QString> &tree_nodes)
    {

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

    Layout& setLayout(int _panels_x, int _panels_y);
    Layout& setLayout(int panel_count);

    void configure(int sim_uid, Canvas2D *canvas, Options *options);

    int canvasWidth();  // Screen dimensions of canvas
    int canvasHeight(); // Screen dimensions of canvas
    Vec2 surfaceSize(); // Dimensions of FBO (depends on whether recording or not)
    int getFrameTimeDelta() { return frame_dt; }

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

    virtual void mouseDown(MouseInfo mouse) {}
    virtual void mouseUp(MouseInfo mouse) {}
    virtual void mouseMove(MouseInfo mouse) {}
    virtual void mouseWheel(MouseInfo mouse) {}

    void _updateSimMouseInfo(int x, int y)
    {
        /*mouse.stage_x = x;
        mouse.stage_y = y;

        //if (focused_cam)
        {
            Vec2 wp = camera.toWorld(mouse.stage_x, mouse.stage_y);
            mouse.world_x = wp.x;
            mouse.world_y = wp.y;
        }*/
    }

    void _mouseDown(int x, int y, Qt::MouseButton btn)
    {
        for (Panel* panel : panels)
        {
            double panel_mx = x - panel->x;
            double panel_my = y - panel->y;

            if (panel_mx >= 0 && panel_my >= 0 &&
                panel_mx <= panel->width && panel_my <= panel->height)
            {
                Camera& cam = panel->camera;// panel->ctx.camera;
                if (cam.panning_enabled && btn == Qt::MiddleButton)
                {
                    cam.panBegin(x, y);
                }
            }
        }

        /*if (focused_cam)
        {
            if (focused_cam->panning_enabled && btn == Qt::MiddleButton)
            {
                focused_cam->panBegin(x, y);
            }
        }

        _updateSimMouseInfo(x, y);
        mouseDown(mouse);*/
    }

    void _mouseUp(int x, int y, Qt::MouseButton btn)
    {
        for (Panel* panel : panels)
        {
            double panel_mx = x - panel->x;
            double panel_my = y - panel->y;

            if (panel_mx >= 0 && panel_my >= 0 &&
                panel_mx <= panel->width && panel_my <= panel->height)
            {
                Camera& cam = panel->camera; ;// panel->ctx.camera;
                if (cam.panning_enabled && btn == Qt::MiddleButton)
                {
                    cam.panEnd(x, y);
                }
            }
        }

        /*
        _updateSimMouseInfo(x, y);
        mouseUp(mouse);*/
    }

    void _mouseMove(int x, int y)
    {
        for (Panel* panel : panels)
        {
            double panel_mx = x - panel->x;
            double panel_my = y - panel->y;

            Camera& cam = panel->camera; ;// panel->ctx.camera;
            if (cam.panning_enabled)
                cam.panDrag(x, y);
        }

        /*
        _updateSimMouseInfo(x, y);
        mouseMove(mouse);*/
    }

    void _mouseWheel(int x, int y, int delta)
    {
        for (Panel* panel : panels)
        {
            double panel_mx = x - panel->x;
            double panel_my = y - panel->y;

            if (panel_mx >= 0 && panel_my >= 0 &&
                panel_mx <= panel->width && panel_my <= panel->height)
            {
                Camera& cam = panel->camera; ;// panel->ctx.camera;
                if (cam.zooming_enabled)
                {
                    cam.targ_zoom_x += (((double)delta) * cam.targ_zoom_x) / 1000.0;
                    cam.targ_zoom_y = cam.targ_zoom_x;
                }
            }
        }

        /*
        mouse.scroll_delta = delta;
        mouseWheel(mouse);*/
    }


    void _draw(QNanoPainter* p);
    void onPainted(const std::vector<GLubyte>& frame);

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
struct AutoRegisterSimulation
{
    AutoRegisterSimulation(const std::vector<QString> &tree_path)
    {
        Simulation::addSimulationInfo(tree_path, []() -> Simulation* {
            return (Simulation*)(new T());
        });
    }
};

struct AutoRegisterSimulationOrder
{
    AutoRegisterSimulationOrder(const std::vector<QString> &node_names)
    {
        //Simulation::getOrderMap();
    }
};
