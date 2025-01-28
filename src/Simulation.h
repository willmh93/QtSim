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

//class Simulation;


#define SIM_BEG(id) namespace NS_##id { //struct Sim; inline AutoRegisterSimulation<Sim> register_##id;
#define SIM_DECLARE(id, name) namespace NS_##id { AutoRegisterSimulation<Sim> register_##id(name);
#define BASE_SIM(id) using namespace NS_##id; typedef NS_##id::Sim id;
//#define BASE_SIM(id) namespace NS_##id {
#define SIM_END }

class Canvas2D;
//class Options;
class Simulation : public QObject
{
    Q_OBJECT

    QString name;

    std::random_device rd;
    std::mt19937 gen;

    // We keep the global list in a function-scope static 
    // so it is only instantiated once.
    using CreatorFunc = std::function<Simulation* ()>;
    
    Canvas2D* canvas;
    QElapsedTimer timer;

    QThread* ffmpeg_thread;
    FFmpegWorker* ffmpeg_worker;

public:

    int frame_dt;

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

    // Type of the function that creates a new Simulation-derived object

    // Allows insertion of a CreatorFunc into our factory list
    static void addFactoryItem(QString name, const CreatorFunc& func)
    {
        getNames().push_back(name);
        getCreators().push_back(func);
    }

    // For demonstration, a function that calls all registered creators:
    static std::vector<Simulation*> createAll()
    {
        std::vector<Simulation*> result;
        for (auto& f : getCreators()) {
            result.push_back(f());
        }
        return result;
    }


public:

    //static void addFactoryItem(std::function<Simulation* (void)> creator)
    

    Camera main_cam;

    vector<Camera*> attachedCameras;
    void attachCameraControls(
        Camera *cam,
        bool panning = true,
        bool zooming = true
    )
    {
        cam->panning_enabled = panning;
        cam->zooming_enabled = zooming;
        attachedCameras.push_back(cam);
    }

    bool started;
    bool paused;

    Options* options;

    int mouse_x;
    int mouse_y;

    Simulation();
    ~Simulation();

    virtual void prepare() {}
    virtual void start() {}
    virtual void stop() {}
    virtual void destroy() {}
    virtual void process() {}
    virtual void draw(QNanoPainter* p) {}
    virtual void postProcess();

    void _start();
    void _stop();
    void _process();

    //void startRecording();
    //void endRecording();

    virtual void mouseDown(int x, int y, Qt::MouseButton btn)
    {
        for (Camera* cam : attachedCameras)
        {
            if (cam->enabled && 
                cam->panning_enabled &&
                btn == Qt::MiddleButton)
            {
                cam->panBegin(x, y);
            }
        }
    }

    virtual void mouseUp(int x, int y, Qt::MouseButton btn)
    {
        for (Camera* cam : attachedCameras)
        {
            if (cam->enabled &&
                cam->panning_enabled &&
                btn == Qt::MiddleButton)
            {
                cam->panEnd(x, y);
            }

        }
    }

    virtual void mouseMove(int x, int y)
    {
        for (Camera* cam : attachedCameras)
        {
            if (cam->enabled &&
                cam->panning_enabled &&
                cam->panning)
            {
                cam->panProcess(x, y);
            }
        }
    }

    virtual void mouseWheel(int delta)
    {
        for (Camera* cam : attachedCameras)
        {
            if (cam->enabled &&
                cam->zooming_enabled)
            {
                cam->zoom_x += (((double)delta) * cam->zoom_x) / 1000.0;
                cam->zoom_y = cam->zoom_x;
            }
        }
    }


    bool recording = false;
    bool encoder_busy = false;
    bool encode_next_paint = false;
    //bool frame_ready = false;

    //int record_w = 1920;
    //int record_h = 1080;

    std::vector<GLubyte> frame_buffer; // Not changed until next process

    void _draw(QNanoPainter* p);
    void onPainted(const std::vector<GLubyte> &frame);

    int width();
    int height();

    double random(double min=0, double max=1)
    {
        std::uniform_real_distribution<> dist(min, max);
        return dist(gen);
    }

    void setCanvas(Canvas2D* _canvas) { 
        canvas = _canvas;
        //canvas->onPainted = ;
    }
    void setOptions(Options* _options) { options = _options; }
    void setName(const QString& _name) { name = _name; }

    bool startRecording();
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

