/*
 * This file is part of QtSim
 *
 * Copyright (C) 2025 William Hemsworth
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include <QMainWindow>
#include <QThread>
#include <QDir>
#include <QRegularExpression>

#include "Simulation.h"
#include "Canvas2D.h"

void SimulationInstance::registerMount(Panel* panel)
{
    options = panel->options;
    camera = &panel->camera;

    mounted_to_panels.push_back(panel);
    qDebug() << "Mounted to Panel: " << panel->panelIndex();

    std::vector<SimulationInstance*>& all_instances = panel->layout->all_instances;

    // Only add instance to list if it's not already in the layout (mounted to another panel)
    if (std::find(all_instances.begin(), all_instances.end(), this) == all_instances.end())
        panel->layout->all_instances.push_back(this);
}

void SimulationInstance::registerUnmount(Panel* panel)
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

void SimulationInstance::mountTo(Panel* panel)
{
    panel->mountInstance(this);
}

void SimulationInstance::mountTo(Layout& panels)
{
    panels[panels.count()]->mountInstance(this);
}

void SimulationInstance::mountToAll(Layout& panels)
{
    for (Panel* panel : panels)
        panel->mountInstance(this);
}

Panel::Panel(Layout* layout, Options* options, int panel_index, int grid_x, int grid_y) :
    layout(layout),
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
        sim->registerUnmount(this);

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

    // When resizing window, world coordinate is fixed given viewport anchor
    // If TOP_LEFT, the world coordinate at top left remains fixed
    // If CENTER, world coordinate at middle of viewport remains fixed

    /// Do transform
    p->translate(
        floor(camera.originPixelOffset().x + camera.panPixelOffset().x),
        floor(camera.originPixelOffset().y + camera.panPixelOffset().y) 
    );
    
    p->rotate(camera.rotation);
    p->translate(
        floor(-camera.x * camera.zoom_x),
        floor(-camera.y * camera.zoom_y)
    );

    p->scale(camera.zoom_x, camera.zoom_y);

    // Draw mounted simulation to this viewport
    sim->camera = &camera;
    sim->draw(this);
}

/// Layout

void Layout::resize(int panel_count)
{
    if (targ_panels_x <= 0 || targ_panels_y <= 0)
    {
        // Spread proportionally
        rows = sqrt(panel_count);
        cols = panel_count / rows;
    }
    else if (targ_panels_y <= 0)
    {
        // Expand down
        cols = targ_panels_x;
        rows = (int)ceil((float)panel_count / (float)cols);
    }
    else if (targ_panels_x <= 0)
    {
        // Expand right
        rows = targ_panels_y;
        cols = (int)ceil((float)panel_count / (float)rows);
    }

    // Expand rows down by default if not perfect fit
    if (panel_count > rows * cols)
        rows++;

    int count = (cols * rows);

    for (int y = 0; y < rows; y++)
    {
        for (int x = 0; x < cols; x++)
        {
            int i = (y * cols) + x;
            if (i >= panel_count)
            {
                goto break_nested;
            }

            if (i < panels.size())
            {
                panels[i]->panel_grid_x = x;
                panels[i]->panel_grid_y = y;
            }
            else
            {
                Panel* panel = new Panel(this, options, i, x, y);
                panels.push_back(panel);
            }
        }
    }
    break_nested:;

    // todo: Unmount remaining panel sims
}

void Layout::expandCheck(size_t count)
{
    if (count > panels.size())
        resize(count);
}


/// SimulationBase

Layout& SimulationBase::newLayout() 
{
    panels.clear();
    return panels;
}

Layout& SimulationBase::newLayout(int targ_panels_x, int targ_panels_y)
{
    panels.options = options;

    panels.clear();
    panels.setSize(targ_panels_x, targ_panels_y);

    return panels;
}

void SimulationBase::configure(int _sim_uid, Canvas2D* _canvas, Options* _options)
{
    sim_uid = _sim_uid;
    canvas = _canvas;
    options = _options;

    started = false;
    paused = false;
}

void SimulationBase::_prepare()
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

void SimulationBase::_start()
{
    if (paused)
    {
        // If paused - just resume, don't restart
        paused = false;
        return;
    }

    // Prepare layout
    _prepare();

    // Update layout rects
    updatePanelRects();

    // Call SimulationBase::start()
    start();

    cache.init("cache.bin");

    // Start simulation instances
    for (SimulationInstance* instance : panels.all_instances)
    {
        instance->cache = &cache;
        instance->mouse = &mouse;
        instance->start();
    }

    // Mount to panels
    for (Panel* panel : panels)
    {
        panel->sim->camera = &panel->camera;
        panel->sim->mount(panel);
    }

    if (record_on_start)
        startRecording();

    setSimulationInfoState(SimulationInfo::ACTIVE);

    started = true;
}

void SimulationBase::_stop()
{
    stop();
    cache.finalize();

    if (recording)
        finalizeRecording();

    setSimulationInfoState(SimulationInfo::INACTIVE);
    started = false;
}

void SimulationBase::_destroy()
{
    setSimulationInfoState(SimulationInfo::INACTIVE);
    destroy();
}

void SimulationBase::_pause()
{
    paused = true;
}

void SimulationBase::updatePanelRects()
{
    Vec2 surface_size = surfaceSize();

    // Update panel sizes
    double panel_width = surface_size.x / panels.cols;
    double panel_height = surface_size.y / panels.rows;

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
}

Vec2 SimulationBase::surfaceSize()
{
    double w;
    double h;

    // Determine surface size to draw to (depends on if recording or not)
    if (!recording || window_capture)
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

void SimulationBase::_process()
{
    updatePanelRects();

    // Determine whether to process simulation this frame or not (depends on recording status)
    bool attaching_encoder = (recording && !ffmpeg_worker->isInitialized());
    if (!attaching_encoder && !encoder_busy && !paused)
    {
        dt_timer.start();

        for (Panel* panel : panels)
        {
            double panel_mx = mouse.client_x - panel->x;
            double panel_my = mouse.client_y - panel->y;

            if (panel_mx >= 0 && panel_my >= 0 &&
                panel_mx <= panel->width && panel_my <= panel->height)
            {
                Camera& cam = panel->camera;

                Vec2 world_mouse = cam.toWorld(panel_mx, panel_my);
                mouse.panel = panel;
                mouse.stage_x = panel_mx;
                mouse.stage_y = panel_my;
                mouse.world_x = world_mouse.x;
                mouse.world_y = world_mouse.y;
            }
        }

        // Process each instance scene
        for (SimulationInstance* instance : panels.all_instances)
        {
            //instance->updateMouseInfo();
            instance->mouse = &mouse;
            instance->processScene();
        }

        // Allow simulation to handle process on each Panel
        for (Panel* panel : panels)
        {
            panel->camera.panProcess();
            panel->sim->camera = &panel->camera;
            panel->sim->processPanel(panel);
        }
        
        frame_dt = dt_timer.elapsed();

        // Prepare to encode the next frame
        encode_next_paint = true;
    }
}

void SimulationBase::postProcess()
{
    // Keep delta until entire frame processed and drawn
    mouse.scroll_delta = 0;

    //for (Panel* panel : panels)
    //    panel->sim->postProcess();
}

void SimulationBase::_mouseDown(int x, int y, Qt::MouseButton btn)
{
    for (Panel* panel : panels)
    {
        double panel_mx = x - panel->x;
        double panel_my = y - panel->y;

        if (panel_mx >= 0 && panel_my >= 0 &&
            panel_mx <= panel->width && panel_my <= panel->height)
        {
            Camera& cam = panel->camera;
            if (cam.panning_enabled && btn == Qt::MiddleButton)
                cam.panBegin(x, y);

            Vec2 world_mouse = cam.toWorld(panel_mx, panel_my);
            mouse.panel = panel;
            mouse.btn = btn;
            mouse.stage_x = panel_mx;
            mouse.stage_y = panel_my;
            mouse.world_x = world_mouse.x;
            mouse.world_y = world_mouse.y;
            panel->sim->mouseDown();
        }
    }
}

void SimulationBase::_mouseUp(int x, int y, Qt::MouseButton btn)
{
    for (Panel* panel : panels)
    {
        double panel_mx = x - panel->x;
        double panel_my = y - panel->y;

        if (panel_mx >= 0 && panel_my >= 0 &&
            panel_mx <= panel->width && panel_my <= panel->height)
        {
            Camera& cam = panel->camera;
            if (cam.panning_enabled && btn == Qt::MiddleButton)
                cam.panEnd(x, y);

            Vec2 world_mouse = cam.toWorld(panel_mx, panel_my);
            mouse.panel = panel;
            mouse.btn = btn;
            mouse.stage_x = panel_mx;
            mouse.stage_y = panel_my;
            mouse.world_x = world_mouse.x;
            mouse.world_y = world_mouse.y;
        }
    }

    for (SimulationInstance* instance : panels.all_instances)
        instance->mouseDown();
}

void SimulationBase::_mouseMove(int x, int y)
{
    mouse.client_x = x;
    mouse.client_y = y;

    for (Panel* panel : panels)
    {
        double panel_mx = x - panel->x;
        double panel_my = y - panel->y;
        Camera& cam = panel->camera;

        if (panel_mx >= 0 && panel_my >= 0 &&
            panel_mx <= panel->width && panel_my <= panel->height)
        {
            Vec2 world_mouse = cam.toWorld(panel_mx, panel_my);
            mouse.panel = panel;
            mouse.stage_x = panel_mx;
            mouse.stage_y = panel_my;
            mouse.world_x = world_mouse.x;
            mouse.world_y = world_mouse.y;
            panel->sim->mouseMove();
        }

        if (cam.panning_enabled)
            cam.panDrag(x, y);
    }
}

void SimulationBase::_mouseWheel(int x, int y, int delta)
{
    for (Panel* panel : panels)
    {
        double panel_mx = x - panel->x;
        double panel_my = y - panel->y;

        if (panel_mx >= 0 && panel_my >= 0 &&
            panel_mx <= panel->width && panel_my <= panel->height)
        {
            Camera& cam = panel->camera;

            if (cam.zooming_enabled)
            {
                cam.targ_zoom_x += (((double)delta) * cam.targ_zoom_x) / 1000.0;
                cam.targ_zoom_y = cam.targ_zoom_x;

                /// todo: With zoom/pan easing, handle this every frame, check if pixel pos changes
            }

            Vec2 world_mouse = cam.toWorld(panel_mx, panel_my);
            mouse.panel = panel;
            mouse.stage_x = panel_mx;
            mouse.stage_y = panel_my;
            mouse.world_x = world_mouse.x;
            mouse.world_y = world_mouse.y;
            mouse.scroll_delta = delta;
            panel->sim->mouseWheel();
        }
    }
}

void SimulationBase::_draw(QNanoPainter* p)
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
        p->translate(floor(panel->x), floor(panel->y));

        // Set default transform
        panel->camera.worldTransform();
        
        panel->sim->camera = &panel->camera;
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
        if (panel->panel_grid_x < panels.cols - 1)
        {
            double line_x = floor(panel->x + panel->width) + 0.5;
            p->moveTo(line_x, panel->y);
            p->lineTo(line_x, panel->y + panel->height + 1);
        }

        // Draw horiz line
        if (panel->panel_grid_y < panels.rows - 1)
        {
            double line_y = floor(panel->y + panel->height) + 0.5;
            p->moveTo(panel->x + panel->width + 1, line_y);
            p->lineTo(panel->x, line_y);
        }

        p->stroke();
    }
}

void SimulationBase::onPainted(const std::vector<GLubyte> *frame)
{
    if (!recording)
        return;

    if (encode_next_paint)
    {
        if (window_capture)
        {
            // Capture window
            QScreen* screen = QGuiApplication::primaryScreen();
            if (!screen)
                throw "No screen detected";

            QMainWindow* mainWindow = qobject_cast<QMainWindow*>(options->topLevelWidget());
            QImage window_image = screen->grabWindow(mainWindow->winId()).toImage();
            window_rgba_image = window_image.convertToFormat(QImage::Format_RGBA8888);
            
            encodeFrame(window_rgba_image.bits());
            encode_next_paint = false;
        }
        else
        {
            // Capture canvas offscreen surface
            frame_buffer = std::move(*frame);

            encodeFrame(frame_buffer.data());
            encode_next_paint = false;
        }
    }
}

bool SimulationBase::startRecording()
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
    window_capture = options->isWindowCapture();
    int record_fps = options->getRecordFPS();
    Size record_resolution;

    // If custom resolution, tell canvas to render to offscreen FBO with given resolution
    // If window capture, render viewport like normal, but set up record resolution to window size
    if (window_capture)
    {
        auto* mainWindow = options->topLevelWidget();
        record_resolution = mainWindow->size();
    }
    else
    {
        record_resolution = options->getRecordResolution();

        canvas->render_to_offscreen = true;
        canvas->offscreen_w = record_resolution.x;
        canvas->offscreen_h = record_resolution.y;
    }

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
        record_fps,
        !window_capture // flip
    );

    // When the thread is opened, initialize FFMpeg
    connect(ffmpeg_thread, &QThread::started, ffmpeg_worker, &FFmpegWorker::startRecording);

    // Listen for frame updates, forward pixel data to FFmpeg encoder
    connect(this, &SimulationBase::frameReady, ffmpeg_worker, &FFmpegWorker::encodeFrame);

    // When the frame is succesfully encoded, permit processing the next frame
    connect(ffmpeg_worker, &FFmpegWorker::frameFlushed, ffmpeg_worker, [this]()
    {
        encoder_busy = false;
    });

    // Wait for "end recording" onClicked signal, then signal FFmpeg worker to finish up
    connect(this, &SimulationBase::endRecording, ffmpeg_worker, &FFmpegWorker::finalizeRecording);

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

bool SimulationBase::encodeFrame(uint8_t* data)
{
    encoder_busy = true;
    emit frameReady(data);
    return true;
}

void SimulationBase::finalizeRecording()
{
    recording = false;
    canvas->render_to_offscreen = false;
    emit endRecording();
}

int SimulationBase::canvasWidth()
{
    return canvas->width();
}

int SimulationBase::canvasHeight()
{
    return canvas->height();
}
