#include <QThread>
#include <QDir>
#include <QRegularExpression>

#include "Simulation.h"
#include "Canvas2D.h"


void SimulationInstance::mountToPanel(Panel* panel)
{
    options = panel->options;
    main = panel->main;
    camera = &panel->camera;

    mounted_to_panels.push_back(panel);
    qDebug() << "Mounted to Panel: " << panel->panelIndex();

    std::vector<SimulationInstance*>& all_instances = panel->layout->all_instances;

    // Only add instance to list if it's not already in the layout (mounted to another panel)
    if (std::find(all_instances.begin(), all_instances.end(), this) == all_instances.end())
        panel->layout->all_instances.push_back(this);
}

void SimulationInstance::unmountFromPanel(Panel* panel)
{
    std::vector<SimulationInstance*>& all_instances = panel->layout->all_instances;

    // Only remove instance from list if the layout includes it (which it should)
    auto instance_it = std::find(all_instances.begin(), all_instances.end(), this);
    if (instance_it != all_instances.end())
    {
        qDebug() << "Erasing simulation instance from panel: " << panel->panelIndex();
        all_instances.erase(instance_it);
    }

    // Only unmount from panel if the layout includes the panel (which it should)
    auto panel_it = std::find(mounted_to_panels.begin(), mounted_to_panels.end(), panel);
    if (panel_it != mounted_to_panels.end())
    {
        qDebug() << "Unmounting instance from panel: " << panel->panelIndex();
        mounted_to_panels.erase(panel_it);
    }
}


Panel::Panel(
    Layout* layout, 
    Simulation *main,
    Options* options,
    int panel_index, 
    int grid_x,
    int grid_y
) :
    layout(layout),
    main(main),
    options(options),
    panel_index(panel_index), 
    panel_grid_x(grid_x),
    panel_grid_y(grid_y)
{
    camera.panel = this;
}

Panel::~Panel()
{
    if (sim)
    {
        // Unmount sim from panel
        sim->unmountFromPanel(this);

        // If sim is no longer mounted to any panels, it's safe to destroy
        if (sim->mounted_to_panels.size() == 0)
        {
            qDebug() << "Unmounted instance is mounted to no other panels. Destroying instance";
            sim->_destroy();
            delete sim;
        }
    }
    qDebug() << "Panel destroyed: " << panel_index;
}

void Panel::draw(QNanoPainter* p)
{
    // Attach QNanoPainter for viewport draw operations
    painter = p;

    // Set defaults
    setTextAlign(TextAlign::ALIGN_LEFT);
    setTextBaseline(TextBaseline::BASELINE_TOP);

    // Take snapshot of default transformation
    default_viewport_transform = p->currentTransform();

    // Move origin relative to viewport.
    // When resizing window, origin is fixed to width/height ratio
    double viewport_cx = (width / 2.0);
    double viewport_cy = (height / 2.0);
    double origin_ox = (width * (camera.origin_ratio_x - 0.5) * camera.zoom_x);
    double origin_oy = (height * (camera.origin_ratio_y - 0.5) * camera.zoom_y);

    // Move to origin
    p->translate(
        viewport_cx + origin_ox,
        viewport_cy + origin_oy
    );

    /// Do transform
    p->translate(
        (camera.pan_x * camera.zoom_x),
        (camera.pan_y * camera.zoom_y)
    );

    p->rotate(camera.rotation);
    p->translate(
        -camera.x * camera.zoom_x,
        -camera.y * camera.zoom_y
    );

    p->scale(camera.zoom_x, camera.zoom_y);

    // Draw mounted simulation to this viewport
    sim->camera = &camera;
    sim->draw(this);
}



Layout& Simulation::setLayout(int panels_x, int panels_y)
{
    panels.main = this;
    panels.options = options;

    panels.clear();
    panels.panels_x = panels_x;
    panels.panels_y = panels_y;

    int count = (panels_x * panels_y);
    for (int y = 0; y < panels_y; y++)
    {
        for (int x = 0; x < panels_x; x++)
        {
            int i = (y * panels_x) + x;
            panels.add(i, x, y);
        }
    }

    return panels;
}

Layout& Simulation::setLayout(int panel_count)
{
    int panels_y = sqrt(panel_count);
    int panels_x = panel_count / panels_y;
    return setLayout(panels_x, panels_y);
}

void Simulation::configure(int _sim_uid, Canvas2D* _canvas, Options* _options)
{
    sim_uid = _sim_uid;
    canvas = _canvas;
    options = _options;

    started = false;
    paused = false;
}

inline void Simulation::_prepare()
{
    panels.clear();

    options->clearAllPointers();
    projectAttributes(options);

    // Prepare project and create layout
    // Note: This is where old panels get replaced
    prepare();

    for (Panel* panel : this->panels)
        panel->sim->instanceAttributes(options);
}

void Simulation::_start()
{
    if (paused)
    {
        // If paused - just resume, don't restart
        paused = false;
        return;
    }

    _prepare();
    start();

    cache.init("cache.bin");

    // Start simulation instances
    for (SimulationInstance* instance : panels.all_instances)
    {
        instance->cache = &cache;
        instance->start();
    }

    // Mount to panels
    for (Panel* panel : panels)
       panel->sim->mount(panel);

    if (record_on_start)
        startRecording();

    setSimulationInfoState(SimulationInfo::ACTIVE);

    started = true;
}

void Simulation::_stop()
{
    stop();
    cache.finalize();

    if (recording)
        finalizeRecording();

    setSimulationInfoState(SimulationInfo::INACTIVE);
    started = false;
}

void Simulation::_destroy()
{
    setSimulationInfoState(SimulationInfo::INACTIVE);
    destroy();
}

void Simulation::_pause()
{
    paused = true;
}

Vec2 Simulation::surfaceSize()
{
    double w;
    double h;

    // Determine surface size to draw to (depends on if recording or not)
    if (!recording)
    {
        w = canvasWidth();
        h = canvasHeight();
    }
    else
    {
        Size record_resolution = options->getRecordResolution();

        w = record_resolution.x;
        h = record_resolution.y;
    }

    return Vec2(w, h);
}

void Simulation::_process()
{
    Vec2 surface_size = surfaceSize();

    // Update panel sizes
    double panel_width = surface_size.x / panels.panels_x;
    double panel_height = surface_size.y / panels.panels_y;

    // Update panel rects
    for (Panel* panel : panels)
    {
        panel->x = panel->panel_grid_x * panel_width;
        panel->y = panel->panel_grid_y * panel_height;
        panel->width = panel_width - 1;
        panel->height = panel_height - 1;
        panel->camera.viewport_w = panel_width - 1;
        panel->camera.viewport_h = panel_height - 1;
    }

    // Determine whether to process simulation this frame or not (depends on recording status)
    bool attaching_encoder = (recording && !ffmpeg_worker->isInitialized());
    if (!attaching_encoder && !encoder_busy && !paused)
    {
        dt_timer.start();

        // Process each instance scene
        for (SimulationInstance* instance : panels.all_instances)
            instance->processScene();

        // Allow simulation to handle process on each Panel
        for (Panel* panel : panels)
        {
            panel->camera.panProcess();
            panel->sim->processPanel(panel);
        }
        
        frame_dt = dt_timer.elapsed();

        // Prepare to encode the next frame
        encode_next_paint = true;
    }
}

void Simulation::postProcess()
{
    for (Panel* panel : panels)
        panel->sim->postProcess();
}

void Simulation::_draw(QNanoPainter* p)
{
    Vec2 surface_size = surfaceSize();

    p->setFillStyle({ 10,10,15 });
    p->fillRect(0, 0, surface_size.x, surface_size.y);

    p->setFillStyle({ 255,255,255 });
    p->setStrokeStyle({ 255,255,255 });

    // Draw each panel
    int i = 0;
    for (Panel* panel : panels)
    {
        p->setClipRect(panel->x, panel->y, panel->width, panel->height);
        p->save();

        // Move to panel position
        p->translate(panel->x, panel->y);

        // Set default transform
        panel->camera.worldTransform();
        
        panel->draw(p);

        p->restore();
        p->resetClipping();
    }

    // Draw panel splitters
    for (Panel* panel : panels)
    {
        p->setLineWidth(6);
        p->setStrokeStyle("#2E2E3E");
        p->beginPath();

        // Draw vert line
        if (panel->panel_grid_x < panels.panels_x - 1)
        {
            double line_x = floor(panel->x + panel->width) + 0.5;
            p->moveTo(line_x, panel->y);
            p->lineTo(line_x, panel->y + panel->height + 1);
        }

        // Draw horiz line
        if (panel->panel_grid_y < panels.panels_y - 1)
        {
            double line_y = floor(panel->y + panel->height) + 0.5;
            p->moveTo(panel->x + panel->width + 1, line_y);
            p->lineTo(panel->x, line_y);
        }

        p->stroke();
    }
}

void Simulation::onPainted(const std::vector<GLubyte> &frame)
{
    if (!recording)
        return;

    if (encode_next_paint)
    {
        frame_buffer = frame;
        encodeFrame(frame_buffer.data());
        encode_next_paint = false;
    }
}

bool Simulation::startRecording()
{
    if (recording)
        throw "Error, already recording...";

    if (!allow_start_recording)
        return false; // Haven't finished finalizing the previous recording yet

    // Determine save paths
    QString projects_dir = options->getProjectsDirectory();
    QString project_dir = QDir::toNativeSeparators(projects_dir + "/" + getSimulationInfo()->path.back());
    QString project_videos_dir = QDir::toNativeSeparators(project_dir + "/recordings");

    // Recursively create folders
    QDir dir;
    if (!dir.mkpath(project_videos_dir))
    {
        qDebug() << "Failed to create directory:" << project_videos_dir;
        return false;
    }

    // Determine next save filename (e.g: "clip3.mp4")
    QStringList files = QDir(project_videos_dir).entryList(QDir::Files | QDir::NoDotAndDotDot);

    int max_clip_index = 0;
    for (const QString& file : files)
    {
        QRegularExpression regex(R"(^clip(\d+)\.\w+$)"); // Match "clip{index}.mp4"
        QRegularExpressionMatch match = regex.match(file);

        if (match.hasMatch()) 
        {
            int clip_index = match.captured(1).toInt(); // Extract and convert to int
            if (clip_index > max_clip_index)
                max_clip_index = clip_index;
        }
    }

    QString filename = QDir::toNativeSeparators(
        project_videos_dir + "/clip" + QString::number(max_clip_index+1) + ".mp4"
    );

    // Get record options
    Size record_resolution = options->getRecordResolution();
    int record_fps = options->getRecordFPS();

    // Tell canvas to render to offscreen FBO with given resolution
    canvas->render_to_offscreen = true;
    canvas->offscreen_w = record_resolution.x;
    canvas->offscreen_h = record_resolution.y;

    // Prepare worker/thread
    ffmpeg_worker = new FFmpegWorker();
    ffmpeg_thread = new QThread();
    ffmpeg_worker->moveToThread(ffmpeg_thread);

    ffmpeg_worker->setOutputInfo(
        filename.toStdString(), 
        record_resolution.x, // src size
        record_resolution.y, // src size
        record_resolution.x, // dest size
        record_resolution.y, // dest size
        record_fps
    );

    // When the thread is opened, initialize FFMpeg
    connect(ffmpeg_thread, &QThread::started, ffmpeg_worker, &FFmpegWorker::startRecording);

    // Listen for frame updates, forward pixel data to FFmpeg encoder
    connect(this, &Simulation::frameReady, ffmpeg_worker, &FFmpegWorker::encodeFrame);

    // When the frame is succesfully encoded, permit processing the next frame
    connect(ffmpeg_worker, &FFmpegWorker::frameFlushed, ffmpeg_worker, [this]()
    {
        encoder_busy = false;
    });

    // Wait for "end recording" onClicked signal, then signal FFmpeg worker to finish up
    connect(this, &Simulation::endRecording, ffmpeg_worker, &FFmpegWorker::finalizeRecording);

    // Gracefully shut down worker/thread once FFmpeg finalizes encoding
    connect(ffmpeg_worker, &FFmpegWorker::onFinalized, ffmpeg_worker, [this]()
    {
        ffmpeg_thread->quit();
        ffmpeg_thread->wait();
        ffmpeg_worker->deleteLater();
        ffmpeg_thread->deleteLater();
        allow_start_recording = true;
    });

    // Start the thread
    ffmpeg_thread->start();

    recording = true;
    allow_start_recording = false;

    return true;
}

bool Simulation::encodeFrame(uint8_t* data)
{
    encoder_busy = true;
    emit frameReady(data);
    return true;
}

void Simulation::finalizeRecording()
{
    recording = false;
    canvas->render_to_offscreen = false;
    emit endRecording();
}

int Simulation::canvasWidth()
{
    return canvas->width();
}

int Simulation::canvasHeight()
{
    return canvas->height();
}