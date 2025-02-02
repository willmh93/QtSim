#include "Simulation.h"

#include "Canvas2D.h"


//#include <cstdlib>
//#include <ctime>
//#include <iostream>

#include <QThread>

//std::vector<std::function<Simulation* (void)>> Simulation::sim_factory;

#ifdef _WIN32
HMODULE avcodecLib = nullptr;
HMODULE avformatLib = nullptr;
HMODULE avutilLib = nullptr;
HMODULE swscaleLib = nullptr;
#else
void* avcodecLib = nullptr;
void* avformatLib = nullptr;
void* avutilLib = nullptr;
void* swscaleLib = nullptr;
#endif

/*bool LoadFFmpegLibraries()
{
    HMODULE avcodecLib = LoadLibrary(L"avcodec.dll");

#ifdef _WIN32
    avcodecLib = LOAD_LIBRARY("avcodec.dll");
    avformatLib = LOAD_LIBRARY("avformat.dll");
    avutilLib = LOAD_LIBRARY("avutil.dll");
    swscaleLib = LOAD_LIBRARY("swscale.dll");
#else
    avcodecLib = LOAD_LIBRARY("libavcodec.so");
    avformatLib = LOAD_LIBRARY("libavformat.so");
    avutilLib = LOAD_LIBRARY("libavutil.so");
    swscaleLib = LOAD_LIBRARY("libswscale.so");
#endif

    if (!avcodecLib || !avformatLib || !avutilLib || !swscaleLib)
    {
        return false;
    }

    return true;
}*/

/*void UnloadFFmpegLibraries()
{
    if (avcodecLib) CLOSE_LIBRARY(avcodecLib);
    if (avformatLib) CLOSE_LIBRARY(avformatLib);
    if (avutilLib) CLOSE_LIBRARY(avutilLib);
    if (swscaleLib) CLOSE_LIBRARY(swscaleLib);
}*/

bool FFmpegWorker::startRecording()
{
    //if (!LoadFFmpegLibraries())
    //    return false;

    const int fps = 60;

    // Output file
    const char* filename = output_path.c_str();

    // Allocate format context
    format_context = nullptr;
    if (avformat_alloc_output_context2(&format_context, nullptr, nullptr, filename) < 0)
        return false;

    // Find H.264 codec
    const AVCodec* codec = avcodec_find_encoder(AV_CODEC_ID_H264);
    if (!codec)
        return false;

    // Create video stream
    stream = avformat_new_stream(format_context, codec);
    if (!stream)
        return false;

    // Allocate codec context
    codec_context = avcodec_alloc_context3(codec);
    if (!codec_context)
        return false;

    // Set codec parameters
    codec_context->bit_rate = 64000000; // 8mb/s kbps
    codec_context->width = targ_w;
    codec_context->height = targ_h;
    codec_context->time_base = { 1, fps };
    codec_context->framerate = { fps, 1 };
    codec_context->gop_size = 12; // Group of pictures
    codec_context->max_b_frames = 1;
    codec_context->pix_fmt = AV_PIX_FMT_YUV420P;

    // Set preset for quality/speed tradeoff
    //av_opt_set(codec_context->priv_data, "crf", "5", 0);
    av_opt_set(codec_context->priv_data, "preset", "veryslow", 0);
    //av_opt_set(codec_context->priv_data, "tune", "animation", 0);
    av_opt_set(codec_context->priv_data, "profile", "high", 0);
    //av_opt_set(codec_context->priv_data, "preset", "slow", 0);

    // Open codec
    if (avcodec_open2(codec_context, codec, nullptr) < 0)
        return false;

    // Associate codec parameters with stream
    avcodec_parameters_from_context(stream->codecpar, codec_context);
    stream->time_base = codec_context->time_base;

    // Open output file
    if (!(format_context->oformat->flags & AVFMT_NOFILE)) {
        if (avio_open(&format_context->pb, filename, AVIO_FLAG_WRITE) < 0)
            return false;
    }

    // Write the stream header
    if (avformat_write_header(format_context, nullptr) < 0)
        return false;

    // Allocate frame for YUV420P
    frame = av_frame_alloc();
    if (!frame)
        return false;

    frame->format = codec_context->pix_fmt;
    frame->width = targ_w;
    frame->height = targ_h;
    if (av_frame_get_buffer(frame, 32) < 0)
        return false;

    // Allocate frame for RGB
    rgb_frame = av_frame_alloc();
    if (!rgb_frame)
        return false;

    rgb_frame->format = AV_PIX_FMT_RGBA;
    rgb_frame->width = src_w;
    rgb_frame->height = src_h;
    if (av_frame_get_buffer(rgb_frame, 32) < 0)
        return false;

    // Initialize scaling context
    sws_ctx = sws_getContext(
        src_w, src_h, AV_PIX_FMT_RGBA,     // Source format
        codec_context->width, codec_context->height, AV_PIX_FMT_YUV420P,  // Destination format
        SWS_BILINEAR, nullptr, nullptr, nullptr);

    if (!sws_ctx)
        return false;

    // Allocate packet
    packet = av_packet_alloc();
    if (!packet)
        return false;

    frame_index = 0;
    qDebug() << "<<Recording>>";

    return true;
}

bool FFmpegWorker::encodeFrame(uint8_t* data)
{
    busy = true;

    av_frame_make_writable(rgb_frame);

    //int src_linesize = src_w * 4;
    //memcpy(rgb_frame->data[0], data, src_h * src_w * 4);

    int targ_y = 0;
    for (int src_y = src_h-1; src_y >=0; src_y--) 
    {
        memcpy(rgb_frame->data[0] + targ_y * rgb_frame->linesize[0],
            data + src_y * src_w * 4,
            src_w * 4);
        targ_y++;
    }

    // Convert RGB frame to YUV420P
    sws_scale(
        sws_ctx,
        rgb_frame->data, rgb_frame->linesize,  // Source frame data
        0, src_h, // Source frame height
        frame->data, frame->linesize); // Output frame data

    qDebug() << "<<Encoding Frame>> " << frame_index;
    frame->pts = frame_index++;

    if (avcodec_send_frame(codec_context, frame) < 0)
        return false;

    while (avcodec_receive_packet(codec_context, packet) == 0)
    {
        av_packet_rescale_ts(packet, codec_context->time_base, stream->time_base);
        packet->stream_index = stream->index;

        if (av_interleaved_write_frame(format_context, packet) < 0)
            return false;

        av_packet_unref(packet);
    }

    busy = false;

    if (finalizing)
        doFinalize();

    emit frameFlushed();
    return true;
}

void FFmpegWorker::doFinalize()
{
    qDebug() << "<<Finalizing>>";

    // Flush the encoder
    avcodec_send_frame(codec_context, nullptr);
    while (avcodec_receive_packet(codec_context, packet) == 0) {
        av_packet_rescale_ts(packet, codec_context->time_base, stream->time_base);
        packet->stream_index = stream->index;
        av_interleaved_write_frame(format_context, packet);
        av_packet_unref(packet);
    }

    // Write trailer
    av_write_trailer(format_context);

    // Free resources
    sws_freeContext(sws_ctx);
    av_frame_free(&frame);
    av_frame_free(&rgb_frame);
    av_packet_free(&packet);
    avcodec_free_context(&codec_context);
    avio_close(format_context->pb);
    avformat_free_context(format_context);
}




Simulation::Simulation() {}
Simulation::~Simulation() {}

void Simulation::configure()
{
    started = false;
    paused = false;

    //main_cam.enabled = true;

    

}

void Simulation::_destroy()
{
    //attachedCameras.clear();
}

/*void Simulation::setFocusedCamera(Camera* cam, bool panning, bool zooming)
{
    configureCamera(cam);

    focused_cam = cam;
    cam->panning_enabled = panning;
    cam->zooming_enabled = zooming;

    if (std::find(attachedCameras.begin(), attachedCameras.end(), cam) == attachedCameras.end())
        attachedCameras.push_back(cam);

    //configureCamera(cam, panning, zooming);
    //attachedCameras.push_back(cam);
}

void Simulation::configureCamera(Camera* cam)
{
    cam->viewport_w = width();
    cam->viewport_h = height();
}*/

void Simulation::_start()
{
    start();

    for (Panel* panel : panels)
    {
        panel->sim->width = panel->width;
        panel->sim->height = panel->height;
        panel->sim->prepare();
    }

    if (options->getRecordChecked())
        startRecording();
    
    // Test without record
    /*canvas->render_to_offscreen = true;
    canvas->offscreen_w = record_w;
    canvas->offscreen_h = record_h;*/
}

void Simulation::_stop()
{
    stop();

    if (recording)
        finalizeRecording();
}

void Simulation::_process()
{
    /*for (Camera* cam : attachedCameras)
    {
        cam->viewport_w = width();
        cam->viewport_h = height();
    }*/

    double w = width();
    double h = height();
    double panel_width = w / panels_x;
    double panel_height = h / panels_y;

    for (Panel* panel : panels)
    {
        panel->x = panel->panel_x * panel_width;
        panel->y = panel->panel_y * panel_height;
        panel->width = panel_width - 1;
        panel->height = panel_height - 1;
        panel->ctx.main_cam.viewport_w = panel_width - 1;
        panel->ctx.main_cam.viewport_h = panel_height - 1;
        //context.main_cam.cameraToViewport(0, 0, panel_width, panel_height);
    }

    if (!encoder_busy)
    {
        // Process each panel
        timer.start();
        
        int i = 0;
        for (Panel* panel : panels)
        {
            //active_context = context;
            //process(context);

            active_panel = panel;
            active_panel->sim->process(&panel->ctx);
        }
        
        frame_dt = timer.elapsed();

        // Prepare to encode the next frame
        encode_next_paint = true;
    }
}

void Simulation::prepare()
{
}

void Simulation::destroy()
{
}

void Simulation::postProcess()
{
    for (Panel* panel : panels)
    {
        active_panel = panel;
        panel->sim->postProcess();
    }
}

void Simulation::_draw(QNanoPainter* p)
{
    p->setFillStyle({ 255,255,255 });
    p->setStrokeStyle({ 255,255,255 });

    int i = 0;
    for (Panel* panel : panels)
    {
        active_panel = panel;

        p->setClipRect(panel->x, panel->y, panel->width, panel->height);
        panel->ctx.drawPanel(p, panel->width, panel->height);
        p->resetClipping();
    }

    for (Panel* panel : panels)
    {
        p->setLineWidth(6);
        p->setStrokeStyle("#2E2E3E");
        p->beginPath();

        // Draw vert line
        if (panel->panel_x < panels_x - 1)
        {
            double line_x = floor(panel->x + panel->width) + 0.5;
            p->moveTo(line_x, panel->y);
            p->lineTo(line_x, panel->y + panel->height + 1);
        }

        // Draw horiz line
        if (panel->panel_y < panels_y - 1)
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

void SimulationInstance::postProcess()
{
    // Keep delta until entire frame processed and drawn
    mouse.scroll_delta = 0;
}

bool Simulation::startRecording()
{
    canvas->render_to_offscreen = true;

    Size record_resolution = options->getRecordResolution();

    canvas->offscreen_w = record_resolution.x;
    canvas->offscreen_h = record_resolution.y;

    ffmpeg_worker = new FFmpegWorker();
    ffmpeg_thread = new QThread();

    ffmpeg_worker->moveToThread(ffmpeg_thread);
    ffmpeg_worker->initialize(options->getRecordPath().toStdString(), record_resolution.x, record_resolution.y);

    connect(ffmpeg_thread, &QThread::started, ffmpeg_worker,
        &FFmpegWorker::startRecording);

    connect(this, &Simulation::frameReady, ffmpeg_worker,
        &FFmpegWorker::encodeFrame);

    connect(ffmpeg_worker, &FFmpegWorker::frameFlushed, ffmpeg_worker, [this]()
    {
        encoder_busy = false;
    });

    connect(this, &Simulation::endRecording, ffmpeg_worker,
        &FFmpegWorker::finalizeRecording);

    //connect(ffmpeg_thread, &QThread::finished, thread, &QThread::deleteLater);

    ffmpeg_thread->start();

    recording = true;
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


int Simulation::width()
{
    return canvas->width();
}

int Simulation::height()
{
    return canvas->height();
}

void DrawingContext::drawPanel(QNanoPainter* p, double vw, double vh)
{
    painter = p;

    p->save();

    // Move origin relative to viewport
    //p->translate(
    //    x + (vw * origin_ratio_x),
    //    y + (vh * origin_ratio_y)
    //);

    // Scale by default (use built-in transformer)
    scaleGraphics(true, true);

    //if (!main_cam.scale_graphics)
    //    save();

    panel->sim->draw(this);

    scaleGraphics(false, true);

    while (scale_stack.size())
        restore();

    p->restore();
}

inline void DrawingContext::scaleGraphics(bool b, bool force)
{
    if ((force && b) || (!main_cam.scale_graphics && b))
    {
        // If we WEREN'T scaling, but now we ARE
        painter->save();

        /// Transform world to stage

        // Always zoom to CENTER of viewport, regardless of origin
        //painter->translate(width / 2, height / 2);

        // Move origin relative to viewport (when resizing window, this doesn't move)
        double viewport_cx = (panel->width / 2.0);
        double viewport_cy = (panel->height / 2.0);

        double origin_ox = (panel->width * (origin_ratio_x - 0.5) * main_cam.zoom_x);
        double origin_oy = (panel->height * (origin_ratio_y - 0.5) * main_cam.zoom_y);

        painter->translate(panel->x + viewport_cx, panel->y + viewport_cy);

        painter->translate(
            (main_cam.pan_x * main_cam.zoom_x) + origin_ox,
            (main_cam.pan_y * main_cam.zoom_y) + origin_oy
        );

        painter->rotate(main_cam.rotation);
        painter->translate(
            -main_cam.x * main_cam.zoom_x,
            -main_cam.y * main_cam.zoom_y
        );

        painter->scale(main_cam.zoom_x, main_cam.zoom_y);

        //painter->translate(-width / 2, -height / 2);
    }
    else if ((force && !b) || (!b && main_cam.scale_graphics))
    {
        // If we WERE scaling, but now we're NOT
        painter->restore();
    }
    main_cam.scale_graphics = b;
}
