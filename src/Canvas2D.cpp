#include "Canvas2D.h"
#include "Simulation.h"


/*void generate_random_color_frame2(uint8_t* data, int width, int height, int linesize) {
    // Generate a random RGB color
    uint8_t r = std::rand() % 256;
    uint8_t g = std::rand() % 256;
    uint8_t b = std::rand() % 256;

    // Fill the frame with the random color
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            data[y * linesize + x * 3 + 0] = r; // Red
            data[y * linesize + x * 3 + 1] = g; // Green
            data[y * linesize + x * 3 + 2] = b; // Blue
        }
    }
}

int test2()
{
    // Initialize random number generator
    std::srand(std::time(nullptr));

    // Video parameters
    const int width = 640;
    const int height = 480;
    const int fps = 30;
    const int total_frames = 100;

    // Output file
    const char* filename = "output2.mp4";

    // Allocate format context
    AVFormatContext* format_context = nullptr;
    if (avformat_alloc_output_context2(&format_context, nullptr, nullptr, filename) < 0) {

        return -1;
    }

    // Find H.264 codec
    const AVCodec* codec = avcodec_find_encoder(AV_CODEC_ID_H264);
    if (!codec) {
        return -1;
    }

    // Create video stream
    AVStream* stream = avformat_new_stream(format_context, codec);
    if (!stream) {
        return -1;
    }

    // Allocate codec context
    AVCodecContext* codec_context = avcodec_alloc_context3(codec);
    if (!codec_context) {
        return -1;
    }

    // Set codec parameters
    codec_context->bit_rate = 400000; // 400 kbps
    codec_context->width = width;
    codec_context->height = height;
    codec_context->time_base = { 1, fps };
    codec_context->framerate = { fps, 1 };
    codec_context->gop_size = 12; // Group of pictures
    codec_context->max_b_frames = 2;
    codec_context->pix_fmt = AV_PIX_FMT_YUV420P;

    // Set preset for quality/speed tradeoff
    av_opt_set(codec_context->priv_data, "preset", "medium", 0);

    // Open codec
    int ret = avcodec_open2(codec_context, codec, nullptr);
    if (ret < 0) {
        char errbuf[128];
        av_strerror(ret, errbuf, sizeof(errbuf));
        return -1;
    }

    // Associate codec parameters with stream
    avcodec_parameters_from_context(stream->codecpar, codec_context);
    stream->time_base = codec_context->time_base;

    // Open output file
    if (!(format_context->oformat->flags & AVFMT_NOFILE)) {
        if (avio_open(&format_context->pb, filename, AVIO_FLAG_WRITE) < 0) {
            return -1;
        }
    }

    // Write the stream header
    if (avformat_write_header(format_context, nullptr) < 0) {
        return -1;
    }

    // Allocate frame for YUV420P
    AVFrame* frame = av_frame_alloc();
    if (!frame) {
        return -1;
    }
    frame->format = codec_context->pix_fmt;
    frame->width = codec_context->width;
    frame->height = codec_context->height;
    if (av_frame_get_buffer(frame, 32) < 0) {
        return -1;
    }

    // Allocate frame for RGB
    AVFrame* rgb_frame = av_frame_alloc();
    if (!rgb_frame) {
        return -1;
    }
    rgb_frame->format = AV_PIX_FMT_RGB24;
    rgb_frame->width = width;
    rgb_frame->height = height;
    if (av_frame_get_buffer(rgb_frame, 32) < 0) {
        return -1;
    }

    // Initialize scaling context
    SwsContext* sws_ctx = sws_getContext(
        width, height, AV_PIX_FMT_RGB24,     // Source format
        width, height, AV_PIX_FMT_YUV420P,  // Destination format
        SWS_BILINEAR, nullptr, nullptr, nullptr);

    if (!sws_ctx) {
        return -1;
    }

    // Allocate packet
    AVPacket* packet = av_packet_alloc();
    if (!packet) {
        return -1;
    }

    // Encode and write frames
    for (int i = 0; i < total_frames; ++i) {
        av_frame_make_writable(rgb_frame);
        generate_random_color_frame2(rgb_frame->data[0], width, height, rgb_frame->linesize[0]);

        // Convert RGB frame to YUV420P
        sws_scale(
            sws_ctx,
            rgb_frame->data, rgb_frame->linesize,
            0, height,
            frame->data, frame->linesize);

        frame->pts = i;

        if (avcodec_send_frame(codec_context, frame) < 0) {
            break;
        }

        while (avcodec_receive_packet(codec_context, packet) == 0) {
            av_packet_rescale_ts(packet, codec_context->time_base, stream->time_base);
            packet->stream_index = stream->index;

            if (av_interleaved_write_frame(format_context, packet) < 0) {
                break;
            }
            av_packet_unref(packet);
        }
    }

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

    return 0;
}*/

/*void FFmpegWorker::finished()
{
    int a = 5;
}*/

Canvas2D::Canvas2D(QWidget* parent)
    : QNanoWidget(parent)
{
    setFillColor("#000000");
    setMouseTracking(true);
}

Canvas2D::~Canvas2D()
{
}


void Canvas2D::mousePressEvent(QMouseEvent* event)
{
    if (!sim) return;
    QPointF mousePos = event->position();
    sim->mouse_x = mousePos.x();
    sim->mouse_y = mousePos.y();
    sim->mouseDown(sim->mouse_x, sim->mouse_y, event->button());
    update();
}

void Canvas2D::mouseReleaseEvent(QMouseEvent* event)
{
    if (!sim) return;
    QPointF mousePos = event->position();
    sim->mouse_x = mousePos.x();
    sim->mouse_y = mousePos.y();
    sim->mouseUp(sim->mouse_x, sim->mouse_y, event->button());
    update();
}

void Canvas2D::mouseMoveEvent(QMouseEvent* event)
{
    if (!sim) return;
    QPointF mousePos = event->position();
    sim->mouse_x = mousePos.x();
    sim->mouse_y = mousePos.y();
    sim->mouseMove(sim->mouse_x, sim->mouse_y);
    update();
}

void Canvas2D::wheelEvent(QWheelEvent* event)
{
    if (!sim) return;
    sim->mouseWheel(event->angleDelta().y());
    update();
}

void Canvas2D::initializeGL()
{
    initializeOpenGLFunctions();
    QNanoWidget::initializeGL();
}

void Canvas2D::paint(QNanoPainter* p)
{
    if (!sim || !sim->started)
        return;

    int vw = width();
    int vh = height();
    p->beginFrame(vw, vh);
    
    if (!render_to_offscreen)
    {
        sim->_draw(p);
    }
    else
    {
        double offscreen_aspect_ratio = ((double)offscreen_w / (double)offscreen_h);
        double viewport_aspect_ratio = ((double)vw / (double)vh);
        double off_x = 0, off_y = 0, scale_x = 1, scale_y = 1;

        if (offscreen_aspect_ratio > viewport_aspect_ratio)
        {
            // Offscreen is wider relative to viewport
            scale_x = static_cast<double>(vw) / static_cast<double>(offscreen_w);
            scale_y = scale_x;

            // Center vertically
            off_y = (vh - (offscreen_h * scale_y)) / 2.0;
        }
        else
        {
            // Offscreen is taller relative to viewport
            scale_y = static_cast<double>(vh) / static_cast<double>(offscreen_h);
            scale_x = scale_y;

            // Center horizontally
            off_x = (vw - (offscreen_w * scale_x)) / 2.0;
        }

        // Draw simulation to offscreen painter
        auto offscreen_painter = offscreen_nano_painter.begin(offscreen_w, offscreen_h, true);
        sim->_draw(offscreen_painter);
        offscreen_nano_painter.end();

        // Provide simulation with frame pixels
        sim->onPainted(offscreen_nano_painter.getPixels());

        // Draw offscreen painter to main painter
        offscreen_nano_painter.drawToPainter(p, 
            off_x, 
            off_y, 
            scale_x * offscreen_w, 
            scale_y * offscreen_h);
    }

    p->endFrame();
}

/*
OffscreenNanoPainter painter1;
        QNanoPainter* p2 = painter1.begin(400, 400, true);
        p2->setFillStyle("#3498db");
        p2->fillRect(50, 50, 100, 100);
        painter1.end();
        painter1.drawToPainter(p);
        */

/*qDebug() << "Width: " << width() << "   Height: " << height();



//painter.paint();

//QImage image = painter.toImage();
//image.save("output.png");






float dp = p->devicePixelRatio();

QOpenGLFunctions glF(QOpenGLContext::currentContext());
p->cancelFrame();

// Create and bind fbo1 into use
bool recreate_fbo = (!m_fbo1 || m_fbo1->width() != w || m_fbo1->height() != h);

if (recreate_fbo)
{
    if (m_fbo1)
        delete m_fbo1;

    QOpenGLFramebufferObjectFormat format;
    format.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
    m_fbo1 = new QOpenGLFramebufferObject(w, h, format);

    // Clear fbo intially
    m_fbo1->bind();
    glF.glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glF.glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

m_fbo1->bind();

// Clear fbo
glF.glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
glF.glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);*/


/*OffscreenPainter painter2(400, 400);
    painter2.begin();
    painter2.setFillStyle("#00ffdb");
    painter2.fillRect(50, 50, 100, 100);
    painter2.end();
    painter2.drawToPainter(p, 600, 50);*/


    /*bool captured_frame = false;

    // Raw pixel manipulation
    //if (recording)
     {
        // 1) Read back the pixels from the currently bound FBO
        //int texW = m_fbo1->width();
        //int texH = m_fbo1->height();
        std::vector<GLubyte> data(w * h * 4); // RGBA
        frame_data.reserve(w * h * 4);

        glF.glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, data.data());
        //if (data.size())
        //    captured_frame = true;

        //glReadPixels(0, 0, texW, texH, GL_RGBA, GL_UNSIGNED_BYTE, data.data());

        //for (int i = 0; i < w * h; ++i) {
        //    data[i * 4 + 0] = 255 - data[i * 4 + 0]; // R
        //    data[i * 4 + 1] = 255 - data[i * 4 + 1]; // G
        //    data[i * 4 + 2] = 255 - data[i * 4 + 2]; // B
        //}

        //uint8_t* pixel_data = frame_data.data();
        //encodeFrame(pixel_data);


        // 3) Re‐upload the modified data back into the FBO’s texture
        GLuint oldTexture = 0;
        glF.glGetIntegerv(GL_TEXTURE_BINDING_2D, reinterpret_cast<GLint*>(&oldTexture));

        GLuint fboTextureID = m_fbo1->texture();  // The texture behind your QOpenGLFramebufferObject
        glF.glBindTexture(GL_TEXTURE_2D, fboTextureID);
        glF.glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, data.data());

        // Restore previous texture binding
        //glBindTexture(GL_TEXTURE_2D, oldTexture);
        glF.glBindTexture(GL_TEXTURE_2D, oldTexture);

        if (sim)
            sim->onPainted(data);
    }

    // We are done with fbo1, so end frame and unbind it
    m_fbo1->release();

    QOpenGLFramebufferObject::bindDefault();
    p->beginFrameAt(0, 0, w, h);
    glF.glViewport(0, 0, w, h);

    // Draw fbo1 as image
    QNanoImage fbo1Image = QNanoImage::fromFrameBuffer(m_fbo1);
    p->drawImage(fbo1Image, 0, 0, w, h);
    */

/*
void Canvas2D::paint(QNanoPainter* p)
{
    int w = width();
    int h = height();

    QOpenGLFunctions glF(QOpenGLContext::currentContext());
    p->cancelFrame();



    // Create an offscreen FBO
    QOpenGLFramebufferObjectFormat fboFormat;
    fboFormat.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
    QOpenGLFramebufferObject fbo(QSize(w, h), fboFormat);

    // Bind the FBO for rendering
    fbo.bind();
    glViewport(0, 0, w, h);
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);

    // Render with QNanoPainter
    if (sim)
        sim->_draw(p);

    // Read pixel data from FBO
    std::vector<GLubyte> pixels(w * h * 4); // RGBA
    glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());

    // Perform simple pixel manipulation (invert colors)
    for (size_t i = 0; i < pixels.size(); i += 4) {
        pixels[i] = rand() % 255;       // R
        pixels[i + 1] = 255 - pixels[i + 1]; // G
        pixels[i + 2] = 255 - pixels[i + 2]; // B
        // Alpha (pixels[i + 3]) remains unchanged
    }

    // Upload the modified pixel data to a new texture
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Unbind the FBO
    fbo.release();

    // Use QNanoImage to draw the modified texture
    QNanoImage nanoImage = QNanoImage::fromFrameBuffer(&fbo, QNanoImage::GENERATE_MIPMAPS);
    p->drawImage(nanoImage, 0, 0, w, h);

    // Cleanup
    glDeleteTextures(1, &texture);
}
*/

/*int w = 100;
    int h = 100;

    QOpenGLFunctions glF(QOpenGLContext::currentContext());
    //p->cancelFrame();

    // Create and bind fbo1 into use
    bool recreate_fbo = (!m_fbo1 || m_fbo1->width() != w || m_fbo1->height() != h);

    if (recreate_fbo)
    {
        if (m_fbo1)
            delete m_fbo1;

        QOpenGLFramebufferObjectFormat format;
        format.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
        m_fbo1 = new QOpenGLFramebufferObject(w, h, format);
    }

    m_fbo1->bind();

    // Clear fbo
    glF.glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glF.glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    //p->beginFrame(w, h);

    // Render with QNanoPainter
    p->setFillStyle({ 255,0,255 });
    p->fillRect(300, 300, 100, 200);

    //p->endFrame();

    // Manually draw to frame buffer object
    {
        std::vector<GLubyte> data(w * h * 4); // RGBA
        for (int i = 0; i < w * h; ++i) {
            data[i * 4 + 0] = 255;
            data[i * 4 + 1] = 0;
            data[i * 4 + 2] = 0;
            data[i * 4 + 3] = 127;
        }

        // 3) Upload the modified data back into the FBO’s texture
        GLuint oldTexture = 0;
        glF.glGetIntegerv(GL_TEXTURE_BINDING_2D, reinterpret_cast<GLint*>(&oldTexture));

        GLuint fboTextureID = m_fbo1->texture();  // The texture behind your QOpenGLFramebufferObject
        glF.glBindTexture(GL_TEXTURE_2D, fboTextureID);
        glF.glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, data.data());

        // Restore previous texture binding
        glF.glBindTexture(GL_TEXTURE_2D, oldTexture);
    }

    // We are done with fbo1, so end frame and unbind it
    m_fbo1->release();

    //QOpenGLFramebufferObject::bindDefault();
    //p->beginFrameAt(0, 0, w, h);
    //glF.glViewport(0, 0, w, h);

    // Draw fbo1 as image
    QNanoImage fbo1Image = QNanoImage::fromFrameBuffer(m_fbo1);
    p->drawImage(fbo1Image, 0, 0, w, h);*/

