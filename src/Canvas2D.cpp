#include "Canvas2D.h"
#include "Simulation.h"


Canvas2D::Canvas2D(QWidget* parent)
    : QNanoWidget(parent)
{
    setFillColor("#000000");
    setMouseTracking(true);
}

Canvas2D::~Canvas2D()
{}

void Canvas2D::mousePressEvent(QMouseEvent* event)
{
    if (!sim) return;
    QPointF mousePos = event->position();
    sim->_mouseDown(mousePos.x(), mousePos.y(), event->button());
    update();
}

void Canvas2D::mouseReleaseEvent(QMouseEvent* event)
{
    if (!sim) return;
    QPointF mousePos = event->position();
    sim->_mouseUp(mousePos.x(), mousePos.y(), event->button());
    update();
}

void Canvas2D::mouseMoveEvent(QMouseEvent* event)
{
    if (!sim) return;
    QPointF mousePos = event->position();
    sim->_mouseMove(mousePos.x(), mousePos.y());
    update();
}

void Canvas2D::wheelEvent(QWheelEvent* event)
{
    if (!sim) return;
    QPointF mousePos = event->position();
    sim->_mouseWheel(mousePos.x(), mousePos.y(), event->angleDelta().y());
    update();
}

void Canvas2D::initializeGL()
{
    initializeOpenGLFunctions();
    QNanoWidget::initializeGL();
}

void Canvas2D::paint(QNanoPainter* p)
{
    QScreen* screen = this->screen();// w.windowHandle()->screen();
    qreal scaleFactor = screen->devicePixelRatio();

    int vw = width();
    int vh = height();

    p->beginFrame(vw*scaleFactor, vh*scaleFactor);
    p->scale(scaleFactor);

    if (!sim || !sim->started)
    {
        p->setFillStyle({ 10,10,15 });
        p->fillRect(0, 0, vw, vh);
    }
    

    if (sim && sim->started)
    {
        if (!render_to_offscreen)
        {
            sim->_draw(p);
            sim->onPainted(nullptr);
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
            sim->onPainted(&offscreen_nano_painter.getPixels());

            // Draw offscreen painter to main painter
            offscreen_nano_painter.drawToPainter(p,
                off_x,
                off_y,
                scale_x * offscreen_w,
                scale_y * offscreen_h);
        }
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

