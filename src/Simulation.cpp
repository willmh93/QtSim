#include <QThread>
#include "Simulation.h"
#include "Canvas2D.h"

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

Layout& SimulationBase::setLayout(int _panels_x, int _panels_y)
{
    panels.main = this;
    panels.options = options;

    panels.clear();
    panels.panels_x = _panels_x;
    panels.panels_y = _panels_y;

    panels_x = _panels_x;
    panels_y = _panels_y;

    int count = (panels_x * panels_y);
    double panel_w = width() / panels_x;
    double panel_h = height() / panels_y;

    for (int y = 0; y < panels_y; y++)
    {
        for (int x = 0; x < panels_x; x++)
        {
            int i = (y * panels_x) + x;

            panels.add(
                i, x, y,
                x * panel_w,
                y * panel_h,
                panel_w - 1,
                panel_h - 1
            );

            DrawingContext& context = panels[i]->ctx;

            context.camera.viewport_w = panel_w - 2;
            context.camera.viewport_h = panel_h - 2;
            context.origin_ratio_x = 0.5;
            context.origin_ratio_y = 0.5;
        }
    }

    return panels;
}

Layout& SimulationBase::setLayout(int panel_count)
{
    int panels_y = sqrt(panel_count);
    int panels_x = panel_count / panels_y;
    return setLayout(panels_x, panels_y);
}

void SimulationBase::configure()
{
    started = false;
    paused = false;
}

void SimulationBase::_destroy()
{
    destroy();
}

void SimulationBase::_start()
{
    _prepare();
    start();

    // Start simulation instances
    for (Panel* panel : panels)
    {
        panel->sim->width = panel->width;
        panel->sim->height = panel->height;
        panel->sim->start();
    }

    if (options->getRecordChecked())
        startRecording();
    
    // Test without record
    /*canvas->render_to_offscreen = true;
    canvas->offscreen_w = record_w;
    canvas->offscreen_h = record_h;*/
}

void SimulationBase::_stop()
{
    stop();

    if (recording)
        finalizeRecording();
}

void SimulationBase::_process()
{
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
        panel->ctx.camera.viewport_w = panel_width - 1;
        panel->ctx.camera.viewport_h = panel_height - 1;
    }

    if (!encoder_busy)
    {
        // Process each panel
        dt_timer.start();
        
        int i = 0;
        for (Panel* panel : panels)
            panel->sim->process(&panel->ctx);
        
        frame_dt = dt_timer.elapsed();

        // Prepare to encode the next frame
        encode_next_paint = true;
    }
}

void SimulationBase::postProcess()
{
    for (Panel* panel : panels)
        panel->sim->postProcess();
}

void SimulationBase::_draw(QNanoPainter* p)
{
    p->setFillStyle({ 255,255,255 });
    p->setStrokeStyle({ 255,255,255 });

    int i = 0;
    for (Panel* panel : panels)
    {
        p->setClipRect(panel->x, panel->y, panel->width, panel->height);
        p->save();

        // Move to panel
        p->translate(
            panel->x,
            panel->y
        );

        panel->ctx.camera.worldTransform();
        
        panel->ctx.drawPanel(p, panel->width, panel->height);

        p->restore();
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

void SimulationBase::onPainted(const std::vector<GLubyte> &frame)
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

bool SimulationBase::startRecording()
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

    connect(this, &SimulationBase::frameReady, ffmpeg_worker,
        &FFmpegWorker::encodeFrame);

    connect(ffmpeg_worker, &FFmpegWorker::frameFlushed, ffmpeg_worker, [this]()
    {
        encoder_busy = false;
    });

    connect(this, &SimulationBase::endRecording, ffmpeg_worker,
        &FFmpegWorker::finalizeRecording);

    //connect(ffmpeg_thread, &QThread::finished, thread, &QThread::deleteLater);

    ffmpeg_thread->start();

    recording = true;
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

int SimulationBase::width()
{
    return canvas->width();
}

int SimulationBase::height()
{
    return canvas->height();
}

void DrawingContext::drawPanel(QNanoPainter* p, double vw, double vh)
{
    painter = p;
    setTextBaseline(TextBaseline::BASELINE_TOP);
    
    default_viewport_transform = painter->currentTransform();

    // Move origin relative to viewport (when resizing window, this doesn't move)
    double viewport_cx = (panel->width / 2.0);
    double viewport_cy = (panel->height / 2.0);
    double origin_ox = (panel->width * (origin_ratio_x - 0.5) * camera.zoom_x);
    double origin_oy = (panel->height * (origin_ratio_y - 0.5) * camera.zoom_y);

    // Move to panel
    painter->translate(
        viewport_cx + origin_ox, 
        viewport_cy + origin_oy
    );

    /// Do transform
    painter->translate(
        (camera.pan_x * camera.zoom_x),
        (camera.pan_y * camera.zoom_y)
    );

    painter->rotate(camera.rotation);
    painter->translate(
        -camera.x * camera.zoom_x,
        -camera.y * camera.zoom_y
    );

    painter->scale(camera.zoom_x, camera.zoom_y);
    panel->sim->draw(this);
}

double roundAxisTickStep(double step)
{
    if (step <= 0)
        return 0; // or handle error

    // Determine the order of magnitude of 'step'
    double exponent = floor(log10(step));
    double factor = pow(10.0, exponent);

    // Normalize step to the range [1, 10)
    double base = step / factor;
    double niceMultiplier;

    // Choose the largest candidate from {1, 2, 2.5, 5, 10} that is <= base.
    if (base >= 10.0)
        niceMultiplier = 10.0;
    else if (base >= 5.0)
        niceMultiplier = 5.0;
    else if (base >= 2.5)
        niceMultiplier = 2.5;
    else if (base >= 2.0)
        niceMultiplier = 2.0;
    else
        niceMultiplier = 1.0;

    return niceMultiplier * factor;
}

double roundAxisValue(double v, double step)
{
    return floor(v / step) * step;
}

double ceilAxisValue(double v, double step)
{
    return ceil(v / step) * step;
}

double getAngle(Vec2 a, Vec2 b)
{
    return (b - a).angle();
}

void DrawingContext::drawGraphGrid()
{
    painter->save();

    // Fist, draw axis
    Vec2 stage_origin = camera.toStage(0, 0);// camera.toWorld(0, 0);
    FRect stage_rect = { 0, 0, panel->width, panel->height };

    // World quad
    Vec2 world_tl = camera.toWorld(0, 0);
    Vec2 world_tr = camera.toWorld(panel->width, 0);
    Vec2 world_br = camera.toWorld(panel->width, panel->height);
    Vec2 world_bl = camera.toWorld(0, panel->height);

    double world_w = world_br.x - world_tl.x;
    double world_h = world_br.y - world_tl.y;

    double stage_size = sqrt(panel->width * panel->width + panel->height * panel->height);
    double world_size = sqrt(world_w * world_w + world_h * world_h);
    double world_zoom = (world_size / stage_size);
    double angle = camera.rotation;


    // Axis rays
    Ray stage_axis_pos_x = { stage_origin, angle + 0 };
    Ray stage_axis_pos_y = { stage_origin, angle + M_PI_2 };

    Vec2 negX_intersect, posX_intersect, negY_intersect, posY_intersect;
    bool x_axis_visible = getRayRectIntersection(&negX_intersect, &posX_intersect, stage_rect, stage_axis_pos_x);
    bool y_axis_visible = getRayRectIntersection(&negY_intersect, &posY_intersect, stage_rect, stage_axis_pos_y);

    Vec2 negX_intersect_world = camera.toWorld(negX_intersect);
    Vec2 posX_intersect_world = camera.toWorld(posX_intersect);
    Vec2 negY_intersect_world = camera.toWorld(negY_intersect);
    Vec2 posY_intersect_world = camera.toWorld(posY_intersect);

    camera.setTransformFilters(true, false, false);
    setStrokeStyle(255, 255, 255, 100);
    setFillStyle(255, 255, 255, 100);
    setLineWidth(1);

    // Draw main axis
    beginPath();
    moveTo(negX_intersect_world.x, negX_intersect_world.y);
    lineTo(posX_intersect_world.x, posX_intersect_world.y);
    moveTo(negY_intersect_world.x, negY_intersect_world.y);
    lineTo(posY_intersect_world.x, posY_intersect_world.y);
    stroke();

    // Draw axis ticks
    camera.setTransformFilters(false, false, false);
    double step_wx = roundAxisTickStep(100.0 / camera.zoom_x);
    double step_wy = roundAxisTickStep(100.0 / camera.zoom_y);

    Vec2 x_perp_off = camera.toStageOffset(0, 6 * world_zoom);
    Vec2 x_perp_norm = camera.toStageOffset(0, 1).normalized();

    Vec2 y_perp_off = camera.toStageOffset(6 * world_zoom, 0);
    Vec2 y_perp_norm = camera.toStageOffset(1, 0).normalized();
    
    setTextAlign(TextAlign::ALIGN_CENTER);
    setTextBaseline(TextBaseline::BASELINE_MIDDLE);

    QNanoFont font(QNanoFont::FontId::DEFAULT_FONT_NORMAL);
    font.setPixelSize(12);
    setFont(font);
    //painter->setPixelAlignText(QNanoPainter::PixelAlign::PIXEL_ALIGN_HALF);

    double spacing = 8;

    // Get world bounds, regardless of rotation
    double world_minX = std::min({ world_tl.x, world_tr.x, world_br.x, world_bl.x });
    double world_maxX = std::max({ world_tl.x, world_tr.x, world_br.x, world_bl.x });
    double world_minY = std::min({ world_tl.y, world_tr.y, world_br.y, world_bl.y });
    double world_maxY = std::max({ world_tl.y, world_tr.y, world_br.y, world_bl.y });

    // Draw gridlines
    {
        setStrokeStyle(255, 255, 255, 10);
        beginPath();

        Vec2 p1, p2;
        for (double wx = ceilAxisValue(world_minX, step_wx); wx < world_maxX; wx += step_wx)
        {
            if (abs(wx) < 1e-9) continue;
            Ray line_ray(camera.toStage(wx, 0), angle + M_PI_2);
            if (!getRayRectIntersection(&p1, &p2, stage_rect, line_ray)) break;

            moveTo(p1.floored(0.5));
            lineTo(p2.floored(0.5));
        }
        for (double wy = ceilAxisValue(world_minY, step_wy); wy < world_maxY; wy += step_wy)
        {
            if (abs(wy) < 1e-9) continue;
            Ray line_ray(camera.toStage(0, wy), angle);
            if (!getRayRectIntersection(&p1, &p2, stage_rect, line_ray)) break;

            moveTo(p1.floored(0.5));
            lineTo(p2.floored(0.5));
        }

        stroke();
    }

    setStrokeStyle(255, 255, 255, 120);
    beginPath();

    // Draw x-axis labels
    for (double wx = ceilAxisValue(world_minX, step_wx); wx < world_maxX; wx += step_wx)
    {
        if (abs(wx) < 1e-9) continue;

        Vec2 stage_pos = camera.toStage(wx, 0);
        QString txt = QString("%1").arg(wx);
        Vec2 txt_size = measureText(txt);

        double txt_dist = (abs(cos(angle)) * txt_size.y + abs(sin(angle)) * txt_size.x) * 0.5 + spacing;

        Vec2 tick_anchor = stage_pos + x_perp_off + (x_perp_norm * txt_dist);

        moveTo(stage_pos - x_perp_off);
        lineTo(stage_pos + x_perp_off);
        fillText(txt, tick_anchor);
    }

    // Draw y-axis labels
    for (double wy = ceilAxisValue(world_minY, step_wy); wy < world_maxY; wy += step_wy)
    {
        if (abs(wy) < 1e-9) continue;

        Vec2 stage_pos = camera.toStage(0, wy);
        QString txt = QString("%1").arg(wy);
        Vec2 txt_size = measureText(txt);

        double txt_dist = (abs(cos(angle)) * txt_size.y + abs(sin(angle)) * txt_size.x) * 0.5 + spacing;

        Vec2 tick_anchor = stage_pos + y_perp_off + (y_perp_norm * txt_dist);
        
        moveTo(stage_pos - y_perp_off);
        lineTo(stage_pos + y_perp_off);
        fillText(txt, tick_anchor);
    }

    stroke();
    painter->restore();
}
