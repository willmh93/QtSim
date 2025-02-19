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

#include "Project.h"
#include "Canvas2D.h"

void Scene::registerMount(Viewport* viewport)
{
    options = viewport->options;
    camera = &viewport->camera;

    mounted_to_viewports.push_back(viewport);
    qDebug() << "Mounted to Viewport: " << viewport->viewportIndex();

    std::vector<Scene*>& all_scenes = viewport->layout->all_scenes;

    // Only add scene to list if it's not already in the layout (mounted to another viewport)
    if (std::find(all_scenes.begin(), all_scenes.end(), this) == all_scenes.end())
        viewport->layout->all_scenes.push_back(this);
}

void Scene::registerUnmount(Viewport* viewport)
{
    std::vector<Scene*>& all_scenes = viewport->layout->all_scenes;

    // Only remove scene from list if the layout includes it (which it should)
    auto scene_it = std::find(all_scenes.begin(), all_scenes.end(), this);
    if (scene_it != all_scenes.end())
    {
        qDebug() << "Erasing project scene from viewport: " << viewport->viewportIndex();
        all_scenes.erase(scene_it);
    }

    // Only unmount from viewport if the layout includes the viewport (which it should)
    auto viewport_it = std::find(mounted_to_viewports.begin(), mounted_to_viewports.end(), viewport);
    if (viewport_it != mounted_to_viewports.end())
    {
        qDebug() << "Unmounting scene from viewport: " << viewport->viewportIndex();
        mounted_to_viewports.erase(viewport_it);
    }
}

void Scene::mountTo(Viewport* viewport)
{
    viewport->mountScene(this);
}

void Scene::mountTo(Layout& viewports)
{
    viewports[viewports.count()]->mountScene(this);
}

void Scene::mountToAll(Layout& viewports)
{
    for (Viewport* viewport : viewports)
        viewport->mountScene(this);
}

Viewport::Viewport(Layout* layout, Options* options, int viewport_index, int grid_x, int grid_y) :
    layout(layout),
    options(options),
    viewport_index(viewport_index), 
    viewport_grid_x(grid_x),
    viewport_grid_y(grid_y)
{
    camera.viewport = this;
}

Viewport::~Viewport()
{
    if (scene)
    {
        // Unmount sim from viewport
        scene->registerUnmount(this);

        // If sim is no longer mounted to any viewports, it's safe to destroy
        if (scene->mounted_to_viewports.size() == 0)
        {
            qDebug() << "Unmounted scene is mounted to no other viewports. Destroying scene";
            scene->_destroy();
            delete scene;
        }
    }
    qDebug() << "Viewport destroyed: " << viewport_index;
}

void Viewport::draw(QNanoPainter* p)
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

    // Draw mounted project to this viewport
    scene->camera = &camera;
    scene->viewportDraw(this);
}

/// Layout

void Layout::resize(int viewport_count)
{
    if (targ_viewports_x <= 0 || targ_viewports_y <= 0)
    {
        // Spread proportionally
        rows = sqrt(viewport_count);
        cols = viewport_count / rows;
    }
    else if (targ_viewports_y <= 0)
    {
        // Expand down
        cols = targ_viewports_x;
        rows = (int)ceil((float)viewport_count / (float)cols);
    }
    else if (targ_viewports_x <= 0)
    {
        // Expand right
        rows = targ_viewports_y;
        cols = (int)ceil((float)viewport_count / (float)rows);
    }

    // Expand rows down by default if not perfect fit
    if (viewport_count > rows * cols)
        rows++;

    int count = (cols * rows);

    for (int y = 0; y < rows; y++)
    {
        for (int x = 0; x < cols; x++)
        {
            int i = (y * cols) + x;
            if (i >= viewport_count)
            {
                goto break_nested;
            }

            if (i < viewports.size())
            {
                viewports[i]->viewport_grid_x = x;
                viewports[i]->viewport_grid_y = y;
            }
            else
            {
                Viewport* viewport = new Viewport(this, options, i, x, y);
                viewports.push_back(viewport);
            }
        }
    }
    break_nested:;

    // todo: Unmount remaining viewport sims
}

void Layout::expandCheck(size_t count)
{
    if (count > viewports.size())
        resize(count);
}


/// ProjectBase

Layout& ProjectBase::newLayout() 
{
    viewports.clear();
    return viewports;
}

Layout& ProjectBase::newLayout(int targ_viewports_x, int targ_viewports_y)
{
    viewports.options = options;

    viewports.clear();
    viewports.setSize(targ_viewports_x, targ_viewports_y);

    return viewports;
}

void ProjectBase::configure(int _sim_uid, Canvas2D* _canvas, Options* _options)
{
    sim_uid = _sim_uid;
    canvas = _canvas;
    options = _options;

    started = false;
    paused = false;
}

void ProjectBase::_projectPrepare()
{
    viewports.clear();

    options->clearAllPointers();
    projectAttributes(options);

    // Prepare project and create layout
    // Note: This is where old viewports get replaced
    projectPrepare();

    for (Viewport* viewport : this->viewports)
        viewport->scene->sceneAttributes(options);
}

void ProjectBase::_projectStart()
{
    if (paused)
    {
        // If paused - just resume, don't restart
        paused = false;
        return;
    }

    // Prepare layout
    _projectPrepare();

    // Update layout rects
    updateViewportRects();

    // Call ProjectBase::projectStart()
    projectStart();

    cache.init("cache.bin");

    // Start project scenes
    for (Scene* scene : viewports.all_scenes)
    {
        scene->cache = &cache;
        scene->mouse = &mouse;
        scene->sceneStart();
    }

    // Mount to viewports
    for (Viewport* viewport : viewports)
    {
        viewport->scene->camera = &viewport->camera;
        viewport->scene->sceneMounted(viewport);
    }

    if (record_on_start)
        startRecording();

    setProjectInfoState(ProjectInfo::ACTIVE);

    started = true;
}

void ProjectBase::_projectStop()
{
    projectStop();
    cache.finalize();

    if (recording)
        finalizeRecording();

    setProjectInfoState(ProjectInfo::INACTIVE);
    started = false;
}

void ProjectBase::_projectDestroy()
{
    setProjectInfoState(ProjectInfo::INACTIVE);
    projectDestroy();
}

void ProjectBase::_projectPause()
{
    paused = true;
}

void ProjectBase::updateViewportRects()
{
    Vec2 surface_size = surfaceSize();

    // Update viewport sizes
    double viewport_width = surface_size.x / viewports.cols;
    double viewport_height = surface_size.y / viewports.rows;

    // Update viewport rects
    for (Viewport* viewport : viewports)
    {
        viewport->x = viewport->viewport_grid_x * viewport_width;
        viewport->y = viewport->viewport_grid_y * viewport_height;
        viewport->width = viewport_width - 1;
        viewport->height = viewport_height - 1;
        viewport->camera.viewport_w = viewport_width - 1;
        viewport->camera.viewport_h = viewport_height - 1;
    }
}

Vec2 ProjectBase::surfaceSize()
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

void ProjectBase::_projectProcess()
{
    updateViewportRects();

    // Determine whether to process project this frame or not (depends on recording status)
    bool attaching_encoder = (recording && !ffmpeg_worker->isInitialized());
    if (!attaching_encoder && !encoder_busy && !paused)
    {
        dt_timer.start();

        for (Viewport* viewport : viewports)
        {
            double viewport_mx = mouse.client_x - viewport->x;
            double viewport_my = mouse.client_y - viewport->y;

            if (viewport_mx >= 0 && viewport_my >= 0 &&
                viewport_mx <= viewport->width && viewport_my <= viewport->height)
            {
                Camera& cam = viewport->camera;

                Vec2 world_mouse = cam.toWorld(viewport_mx, viewport_my);
                mouse.viewport = viewport;
                mouse.stage_x = viewport_mx;
                mouse.stage_y = viewport_my;
                mouse.world_x = world_mouse.x;
                mouse.world_y = world_mouse.y;
            }
        }

        // Process each scene scene
        for (Scene* scene : viewports.all_scenes)
        {
            //scene->updateMouseInfo();
            scene->mouse = &mouse;
            scene->sceneProcess();
        }

        // Allow project to handle process on each Viewport
        for (Viewport* viewport : viewports)
        {
            viewport->camera.panProcess();
            viewport->scene->camera = &viewport->camera;
            viewport->scene->viewportProcess(viewport);
        }
        
        frame_dt = dt_timer.elapsed();

        // Prepare to encode the next frame
        encode_next_paint = true;
    }
}

void ProjectBase::postProcess()
{
    // Keep delta until entire frame processed and drawn
    mouse.scroll_delta = 0;

    //for (Viewport* viewport : viewports)
    //    viewport->scene->postProcess();
}

void ProjectBase::_mouseDown(int x, int y, Qt::MouseButton btn)
{
    for (Viewport* viewport : viewports)
    {
        double viewport_mx = x - viewport->x;
        double viewport_my = y - viewport->y;

        if (viewport_mx >= 0 && viewport_my >= 0 &&
            viewport_mx <= viewport->width && viewport_my <= viewport->height)
        {
            Camera& cam = viewport->camera;
            if (cam.panning_enabled && btn == Qt::MiddleButton)
                cam.panBegin(x, y);

            Vec2 world_mouse = cam.toWorld(viewport_mx, viewport_my);
            mouse.viewport = viewport;
            mouse.btn = btn;
            mouse.stage_x = viewport_mx;
            mouse.stage_y = viewport_my;
            mouse.world_x = world_mouse.x;
            mouse.world_y = world_mouse.y;
            viewport->scene->mouseDown();
        }
    }
}

void ProjectBase::_mouseUp(int x, int y, Qt::MouseButton btn)
{
    for (Viewport* viewport : viewports)
    {
        double viewport_mx = x - viewport->x;
        double viewport_my = y - viewport->y;

        if (viewport_mx >= 0 && viewport_my >= 0 &&
            viewport_mx <= viewport->width && viewport_my <= viewport->height)
        {
            Camera& cam = viewport->camera;
            if (cam.panning_enabled && btn == Qt::MiddleButton)
                cam.panEnd(x, y);

            Vec2 world_mouse = cam.toWorld(viewport_mx, viewport_my);
            mouse.viewport = viewport;
            mouse.btn = btn;
            mouse.stage_x = viewport_mx;
            mouse.stage_y = viewport_my;
            mouse.world_x = world_mouse.x;
            mouse.world_y = world_mouse.y;
        }
    }

    for (Scene* scene : viewports.all_scenes)
        scene->mouseDown();
}

void ProjectBase::_mouseMove(int x, int y)
{
    mouse.client_x = x;
    mouse.client_y = y;

    for (Viewport* viewport : viewports)
    {
        double viewport_mx = x - viewport->x;
        double viewport_my = y - viewport->y;
        Camera& cam = viewport->camera;

        if (viewport_mx >= 0 && viewport_my >= 0 &&
            viewport_mx <= viewport->width && viewport_my <= viewport->height)
        {
            Vec2 world_mouse = cam.toWorld(viewport_mx, viewport_my);
            mouse.viewport = viewport;
            mouse.stage_x = viewport_mx;
            mouse.stage_y = viewport_my;
            mouse.world_x = world_mouse.x;
            mouse.world_y = world_mouse.y;
            viewport->scene->mouseMove();
        }

        if (cam.panning_enabled)
            cam.panDrag(x, y);
    }
}

void ProjectBase::_mouseWheel(int x, int y, int delta)
{
    for (Viewport* viewport : viewports)
    {
        double viewport_mx = x - viewport->x;
        double viewport_my = y - viewport->y;

        if (viewport_mx >= 0 && viewport_my >= 0 &&
            viewport_mx <= viewport->width && viewport_my <= viewport->height)
        {
            Camera& cam = viewport->camera;

            if (cam.zooming_enabled)
            {
                cam.targ_zoom_x += (((double)delta) * cam.targ_zoom_x) / 1000.0;
                cam.targ_zoom_y = cam.targ_zoom_x;

                /// todo: With zoom/pan easing, handle this every frame, check if pixel pos changes
            }

            Vec2 world_mouse = cam.toWorld(viewport_mx, viewport_my);
            mouse.viewport = viewport;
            mouse.stage_x = viewport_mx;
            mouse.stage_y = viewport_my;
            mouse.world_x = world_mouse.x;
            mouse.world_y = world_mouse.y;
            mouse.scroll_delta = delta;
            viewport->scene->mouseWheel();
        }
    }
}

void ProjectBase::_keyPress(QKeyEvent* e)
{
    for (Scene* scene : viewports.all_scenes)
    {
        scene->keyPressed(e);
    }
}

void ProjectBase::_keyRelease(QKeyEvent* e)
{
    for (Scene* scene : viewports.all_scenes)
    {
        scene->keyReleased(e);
    }
}

void ProjectBase::_draw(QNanoPainter* p)
{
    Vec2 surface_size = surfaceSize();

    p->setFillStyle({ 10,10,15 });
    p->fillRect(0, 0, surface_size.x, surface_size.y);

    p->setFillStyle({ 255,255,255 });
    p->setStrokeStyle({ 255,255,255 });

    // Draw each viewport
    int i = 0;
    for (Viewport* viewport : viewports)
    {
        p->setClipRect(viewport->x, viewport->y, viewport->width, viewport->height);
        p->save();

        // Move to viewport position
        p->translate(floor(viewport->x), floor(viewport->y));

        // Set default transform
        viewport->camera.worldTransform();
        
        viewport->scene->camera = &viewport->camera;
        viewport->draw(p);

        p->restore();
        p->resetClipping();
    }

    // Draw viewport splitters
    for (Viewport* viewport : viewports)
    {
        p->setLineWidth(6);
        p->setStrokeStyle("#2E2E3E");
        p->beginPath();

        // Draw vert line
        if (viewport->viewport_grid_x < viewports.cols - 1)
        {
            double line_x = floor(viewport->x + viewport->width) + 0.5;
            p->moveTo(line_x, viewport->y);
            p->lineTo(line_x, viewport->y + viewport->height + 1);
        }

        // Draw horiz line
        if (viewport->viewport_grid_y < viewports.rows - 1)
        {
            double line_y = floor(viewport->y + viewport->height) + 0.5;
            p->moveTo(viewport->x + viewport->width + 1, line_y);
            p->lineTo(viewport->x, line_y);
        }

        p->stroke();
    }
}

void ProjectBase::onPainted(const std::vector<GLubyte> *frame)
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

bool ProjectBase::startRecording()
{
    if (recording)
        throw "Error, already recording...";

    if (!allow_start_recording)
        return false; // Haven't finished finalizing the previous recording yet

    // Determine save paths
    QString projects_dir = options->getProjectsDirectory();
    QString project_dir = QDir::toNativeSeparators(projects_dir + "/" + getProjectInfo()->path.back());
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
    connect(this, &ProjectBase::frameReady, ffmpeg_worker, &FFmpegWorker::encodeFrame);

    // When the frame is succesfully encoded, permit processing the next frame
    connect(ffmpeg_worker, &FFmpegWorker::frameFlushed, ffmpeg_worker, [this]()
    {
        encoder_busy = false;
    });

    // Wait for "end recording" onClicked signal, then signal FFmpeg worker to finish up
    connect(this, &ProjectBase::endRecording, ffmpeg_worker, &FFmpegWorker::finalizeRecording);

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

bool ProjectBase::encodeFrame(uint8_t* data)
{
    encoder_busy = true;
    emit frameReady(data);
    return true;
}

void ProjectBase::finalizeRecording()
{
    recording = false;
    canvas->render_to_offscreen = false;
    emit endRecording();
}

int ProjectBase::canvasWidth()
{
    return canvas->width();
}

int ProjectBase::canvasHeight()
{
    return canvas->height();
}
