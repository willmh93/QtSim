#pragma once

#include <QObject>
#include <QString>
#include <QColor>
#include <QElapsedTimer>

#include <random>
#include <cmath>
#include <vector>
#include <queue>
#include <limits>
#include <set>
#include <unordered_map>


#include "qnanopainter.h"

#include "Options.h"
#include "World.h"
#include "graphics.h"
#include "helpers.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/opt.h>
#include <libavutil/imgutils.h>
}

/*#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#define LOAD_LIBRARY(name) LoadLibraryA(name)
#define GET_PROC_ADDR(lib, func) GetProcAddress(lib, func)
#define CLOSE_LIBRARY(lib) FreeLibrary(lib)
#else
#include <dlfcn.h>
#define LOAD_LIBRARY(name) dlopen(name, RTLD_LAZY)
#define GET_PROC_ADDR(lib, func) dlsym(lib, func)
#define CLOSE_LIBRARY(lib) dlclose(lib)
#endif*/

// Load FFmpeg dynamically
bool LoadFFmpegLibraries();
void UnloadFFmpegLibraries();

class FFmpegWorker : public QObject
{
    Q_OBJECT

    int src_w;
    int src_h;
    int targ_w;
    int targ_h;
    int frame_index;

    bool finalizing = false;
    bool busy = false;

    std::string output_path;
    AVFormatContext* format_context;
    AVStream* stream;
    AVCodecContext* codec_context;
    AVFrame* frame;
    AVFrame* rgb_frame;
    SwsContext* sws_ctx;
    AVPacket* packet;

    void doFinalize();

public:

    void initialize(
        std::string save_path,
        int src_width,
        int src_height,
        int targ_width = 640,
        int targ_height = 480)
    {
        output_path = save_path;
        src_w = src_width;
        src_h = src_height;
        targ_w = src_width;// targ_width;
        targ_h = src_height;// targ_height;
    }

signals:

    void frameFlushed();

public slots:

    bool startRecording();
    bool encodeFrame(uint8_t* data);
    void finalizeRecording()
    {
        if (busy)
            finalizing = true;
        else
            doFinalize();
    }

};

using namespace std;


#define SIM_BEG(cls) namespace NS_##cls { //struct Sim; inline AutoRegisterSimulation<Sim> register_##id;
#define SIM_DECLARE(cls, name) namespace NS_##cls { AutoRegisterSimulation<cls> register_##cls(name);
#define BASE_SIM(cls) using namespace NS_##cls; //typedef NS_##cls::Sim cls;
//#define BASE_SIM(id) namespace NS_##id {
#define SIM_END }

class Canvas2D;
class Simulation;
struct Panel;

using LineCap = QNanoPainter::LineCap;
using LineJoin = QNanoPainter::LineJoin;
using TextAlign = QNanoPainter::TextAlign;
using TextBaseline = QNanoPainter::TextBaseline;

struct DrawingContext
{
    //int panel_index;
    //int panel_x;
    //int panel_y;

    //double x;
    //double y;
    //double width;
    //double height;

    Panel* panel = nullptr;

    double origin_ratio_x;
    double origin_ratio_y;

    Camera main_cam;
    //Camera* focused_cam = nullptr;

    // Drawing abstraction
    QNanoPainter* painter;
    std::vector<bool> scale_stack;

    DrawingContext() {}
    /*DrawingContext(
        int _panel_index, 
        int _panel_x, 
        int _panel_y, 
        double _x, double _y, 
        double _w, double _h)
    {
        panel_index = _panel_index;
        panel_x = _panel_x;
        panel_y = _panel_y;
        x = _x;
        y = _y;
        width = _w;
        height = _h;
    }*/

    void drawPanel(QNanoPainter *p, double vw, double vh);

    Vec2 PT(double x, double y)
    {
        if (main_cam.scale_graphics)
            return { x, y };
        else
            return main_cam.toStage(x, y);
    }

    void save()
    {
        //painter->save();
        scale_stack.push_back(main_cam.scale_graphics);
    }

    void restore()
    {
        //painter->restore();

        bool b = scale_stack.back();
        scale_stack.pop_back();

        scaleGraphics(b);
    }

    void scaleGraphics(bool b, bool force = false);

    void beginPath()
    {
        painter->beginPath();
    }

    void setFillStyle(int r, int g, int b, int a = 255)
    {
        painter->setFillStyle({ r,g,b,a });
    }

    void setFillStyle(const QNanoColor& color)
    {
        painter->setFillStyle(color);
    }

    void setStrokeStyle(int r, int g, int b, int a = 255)
    {
        painter->setStrokeStyle({ r,g,b,a });
    }

    void setStrokeStyle(const QNanoColor& color)
    {
        painter->setStrokeStyle(color);
    }

    void setLineWidth(int width)
    {
        painter->setLineWidth(width);
    }

    void setLineCap(LineCap cap)
    {
        painter->setLineCap(cap);
    }

    void fill()
    {
        painter->fill();
    }

    void stroke()
    {
        painter->stroke();
    }

    void circle(double cx, double cy, double r)
    {
        Vec2 pt = PT(cx, cy);
        painter->circle(pt.x, pt.y, r);
    }

    void moveTo(double px, double py)
    {
        Vec2 pt = PT(px, py);
        painter->moveTo(pt.x, pt.y);
    }

    void lineTo(double px, double py)
    {
        Vec2 pt = PT(px, py);
        painter->lineTo(pt.x, pt.y);
    }

    void setTextAlign(TextAlign align)
    {
        painter->setTextAlign(align);
    }

    void setTextBaseline(TextBaseline align)
    {
        painter->setTextBaseline(align);
    }
};

//template<typename T>
class SimulationInstance
{
    std::random_device rd;
    std::mt19937 gen;

public:


    Simulation* main;
    Panel* panel;
    Camera* camera;

    MouseInfo mouse;

    double width;
    double height;

    SimulationInstance() : gen(rd())
    {}

    virtual void prepare() {}
    virtual void start() {}
    virtual void stop() {}
    virtual void destroy() {}
    virtual void process(DrawingContext* ctx) {}
    virtual void draw(DrawingContext* ctx) {}
    virtual void postProcess();

    double random(double min = 0, double max = 1)
    {
        std::uniform_real_distribution<> dist(min, max);
        return dist(gen);
    }
};

struct Panel
{
    DrawingContext ctx;
    SimulationInstance* sim;

    int panel_index;
    int panel_x;
    int panel_y;
    double x;
    double y;
    double width;
    double height;

    Panel()
    {}

    template<typename T, typename... Args>
    T* construct(Args&&... args)
    {
        sim = new T(std::forward<Args>(args)...);
        sim->panel = ctx.panel;
        sim->camera = &ctx.main_cam;
        return dynamic_cast<T*>(sim);
    }
};

struct Layout
{
    std::vector<Panel*> panels;
    int panels_x;
    int panels_y;

    // Custom iterator
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
        int _panel_x,
        int _panel_y,
        double _x, double _y,
        double _w, double _h)
    {
        Panel* panel = new Panel();
        panel->ctx.panel = panel;
        panel->panel_index = _panel_index;
        panel->panel_x = _panel_x;
        panel->panel_y = _panel_y;
        panel->x = _x;
        panel->y = _y;
        panel->width = _w;
        panel->height = _h;
        panels.push_back(panel);
    }

    Panel* operator[](int i)
    {
        return panels[i];
    }

    iterator begin() { return panels.begin(); }
    iterator end() { return panels.end(); }

    const_iterator begin() const { return panels.begin(); }
    const_iterator end() const { return panels.end(); }
};


class Simulation : public QObject
{
    Q_OBJECT

public:

    using CreatorFunc = std::function<Simulation*()>;

    // Factory methods
    static std::vector<CreatorFunc>& getCreators()
    {
        static std::vector<CreatorFunc> creators;
        return creators;
    }
    static std::vector<QString>& getNames()
    {
        static std::vector<QString> names;
        return names;
    }
    static std::vector<Simulation*> createAll()
    {
        std::vector<Simulation*> result;
        for (auto& f : getCreators())
            result.push_back(f());
        return result;
    }
    static void addFactoryItem(QString name, const CreatorFunc& func)
    {
        getNames().push_back(name);
        getCreators().push_back(func);
    }

private:

    QString name;

    Canvas2D* canvas;
    QElapsedTimer dt_timer;

    QThread* ffmpeg_thread;
    FFmpegWorker* ffmpeg_worker;

    Layout panels;
    int panels_x = 1;
    int panels_y = 1;

public:

    Options* options;

    bool started;
    bool paused;

    int frame_dt;

    // Recording states
    bool recording = false;
    bool encoder_busy = false;
    bool encode_next_paint = false;
    std::vector<GLubyte> frame_buffer; // Not changed until next process
    
    //Simulation() {}
    //~Simulation() {}

    Layout& setLayout(int _panels_x, int _panels_y);
    Layout& setLayout(int panel_count);


    void configure();

    void _destroy();
    void _prepare();
    void _start();
    void _stop();
    void _process();

    virtual void prepare();
    virtual void destroy();
    virtual void postProcess();

    virtual void start() {}
    virtual void stop() {}

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
            Vec2 wp = main_cam.toWorld(mouse.stage_x, mouse.stage_y);
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
                Camera& cam = panel->ctx.main_cam;
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
                Camera& cam = panel->ctx.main_cam;
                if (cam.panning_enabled && btn == Qt::MiddleButton)
                {
                    cam.panEnd(x, y);
                }
            }
        }

        /*if (focused_cam)
        {
            if (focused_cam->panning_enabled && btn == Qt::MiddleButton)
            {
                focused_cam->panEnd(x, y);
            }
        }

        _updateSimMouseInfo(x, y);
        mouseUp(mouse);*/
    }

    void _mouseMove(int x, int y)
    {
        for (Panel* panel : panels)
        {
            double panel_mx = x - panel->x;
            double panel_my = y - panel->y;

            Camera& cam = panel->ctx.main_cam;
            if (cam.panning_enabled)
                cam.panProcess(x, y);
        }

        /*if (focused_cam)
        {
            if (focused_cam->panning_enabled &&focused_cam->panning)
            {
                focused_cam->panProcess(x, y);
            }
        }

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
                Camera& cam = panel->ctx.main_cam;
                if (cam.zooming_enabled)
                {
                    cam.zoom_x += (((double)delta) * cam.zoom_x) / 1000.0;
                    cam.zoom_y = cam.zoom_x;
                }
            }
        }

        /*if (focused_cam)
        {
            if (focused_cam->zooming_enabled)
            {
                focused_cam->zoom_x += (((double)delta) * focused_cam->zoom_x) / 1000.0;
                focused_cam->zoom_y = focused_cam->zoom_x;
            }
        }
        
        mouse.scroll_delta = delta;
        mouseWheel(mouse);*/
    }


    void _draw(QNanoPainter* p);
    void onPainted(const std::vector<GLubyte> &frame);

    int width();
    int height();

    void setCanvas(Canvas2D* _canvas) { canvas = _canvas; }
    void setOptions(Options* _options) { options = _options; }
    void setName(const QString& _name) { name = _name; }

    bool startRecording();
    bool encodeFrame(uint8_t* data);
    void finalizeRecording();

    /*
void updateTransform()
{
    painter->translate(
        (main_cam.pan_x * main_cam.zoom_x),
        (main_cam.pan_y * main_cam.zoom_y)
    );

    painter->rotate(main_cam.rotation);
    painter->translate(
        -main_cam.x * main_cam.zoom_x,
        -main_cam.y * main_cam.zoom_y
    );
    if (main_cam.scale_graphics)
    {
        painter->scale(main_cam.zoom_x, main_cam.zoom_y);
    }
}
*/

    /*void save() { active_context->save(); }
    void restore() { active_context->restore(); }
    void scaleGraphics(bool b, bool force = false) { active_context->scaleGraphics(b, force); }
    void beginPath() { active_context->beginPath(); }
    void setFillStyle(int r, int g, int b, int a = 255) { active_context->setFillStyle(r,g,b,a); }
    void setStrokeStyle(int r, int g, int b, int a = 255) { active_context->setStrokeStyle(r,g,b,a); }
    void fill() { active_context->fill(); }
    void stroke() { active_context->stroke(); }
    void circle(double cx, double cy, double r) { active_context->circle(cx, cy, r); }*/

signals:

    void workerReady();
    void frameReady(uint8_t* data);
    void endRecording();
};

template <typename T>
struct AutoRegisterSimulation
{
    AutoRegisterSimulation(QString name)
    {
        // Here, we push a lambda that creates a new T()
        // (which is a subclass of Simulation).
        Simulation::addFactoryItem(name, []() -> Simulation* {
            return new T();
        });
    }
};



/*void addFactoryItem(std::function<Simulation* (void)> creator)
{  
}
struct Test
{
    Test(std::function<Simulation* (void)> creator)
    {
        Simulation::sim_factory.push_back(creator);
    }
};*/

