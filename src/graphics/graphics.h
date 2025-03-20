#pragma once
#include <QObject>
#include <QImage>
#include <QOpenGLExtraFunctions>
#include <QOpenGLFramebufferObject>
#include "glwrappers.h"

#include "qnanopainter.h"
#include "types.h"

struct PaintContext;

/*class Bitmap
{
    std::vector<uchar> data;
    int bmp_width;
    int bmp_height;

    QImage img;
    QNanoImage nano_img;

    //bool smoothing;

    static int bmp_index;

public:

    Bitmap() : bmp_width(0), bmp_height(0)
    {
    }

    //Bitmap(const char *filepath) : bmp_width(0), bmp_height(0), nano_img(filepath)
    //{
    //}

    void create(int w, int h, bool smoothing=false)
    {
        bmp_width = w;
        bmp_height = h;
        data.resize(w * h * 4);

        int len = w * h * 4;
        for (int i = 0; i < len; i++)
            data[i] = 0;

        img = QImage(data.data(), w, h, QImage::Format::Format_RGBA8888);

        QString id = QString("img_%1").arg(bmp_index++);
        nano_img = QNanoImage(img, id, smoothing ? QNanoImage::ImageFlag(0) : QNanoImage::ImageFlag::NEAREST);
    }

    void clear(uchar r=0, uchar g=0, uchar b=0, uchar a=255)
    {
        int len = bmp_width * bmp_height * 4;
        for (int i = 0; i < len; i++)
            data[i] = 0;
    }

    void draw(PaintContext* ctx, double x, double y, double w, double h);

    void draw(PaintContext* ctx, const Vec2 &pt, const Vec2 &size);

    void setPixel(int x, int y, uchar r, uchar g, uchar b, uchar a)
    {
        int i = (y * bmp_width + x) * 4;
        data[i] = r;
        data[i + 1] = g;
        data[i + 2] = b;
        data[i + 3] = a;
    }

    void setPixelSafe(int x, int y, uchar r, uchar g, uchar b, uchar a)
    {
        if (x < 0 || x >= bmp_width ||
            y < 0 || y >= bmp_height)
        {
            return;
        }

        int i = (y * bmp_width + x) * 4;
        data[i] = r;
        data[i + 1] = g;
        data[i + 2] = b;
        data[i + 3] = a;
    }
};*/

struct GLSurface
{
    std::shared_ptr<QOpenGLFramebufferObject> fbo = nullptr;
    GLint prev_fbo = 0;
    int width = 0;
    int height = 0;
    int old_width = 0;
    int old_height = 0;

    GLSurface()
    {}

    GLSurface(const GLSurface& rhs)
    {
        fbo = rhs.fbo;
        prev_fbo = rhs.prev_fbo;
        width = rhs.width;
        height = rhs.height;
        old_width = rhs.old_width;
        old_height = rhs.old_height;
    }
    
    ~GLSurface()
    {
        if (fbo)
        {
            // Unbind if currently bound
            fbo->release();
            //delete fbo;
            //fbo = nullptr;
        }
    }

    void prepare(int w, int h)
    {
        if (fbo == nullptr || w != width || h != height)
        {
            width = w;
            height = h;

            // Delete the old unbound FBO (if it already exists)
            ///if (fbo)
            ///    delete fbo;

            QOpenGLFramebufferObjectFormat format;
            format.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
            format.setInternalTextureFormat(GL_RGBA8);
            //fbo = new QOpenGLFramebufferObject(w, h, format);
            fbo = std::make_shared<QOpenGLFramebufferObject>(w, h, format);
        }
    }

    void bind()
    {
        QOpenGLExtraFunctions* glF = QOpenGLContext::currentContext()->extraFunctions();

        // Cache old viewport size for restoring in end()
        GLint viewport[4];
        glF->glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prev_fbo);
        glF->glGetIntegerv(GL_VIEWPORT, viewport);

        old_width = viewport[2];  // Width of the viewport
        old_height = viewport[3];  // Height of the viewport

        // Bind FBO and set default viewport
        fbo->bind();
        glF->glViewport(0, 0, width, height);

        // Pixel write-access
        ///glF->glGenBuffers(1, &pbo);
        ///glF->glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo);
        ///glF->glBufferData(GL_PIXEL_UNPACK_BUFFER, width * height * 4, NULL, GL_DYNAMIC_DRAW);
        ///glF->glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
    }

    void release()
    {
        QOpenGLExtraFunctions* glF = QOpenGLContext::currentContext()->extraFunctions();

        ///GLuint texID = fbo->texture(); // Get FBO texture ID
        ///glF->glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo);
        ///
        ///// Write pixels
        ///pixels = (uint8_t*)glF->glMapBufferRange(GL_PIXEL_UNPACK_BUFFER, 0,
        ///    width * height * 4, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
        ///    //width * height * 4, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
        ///
        ///if (pixels) {
        ///    memset(pixels, 100, width * height * 4);
        ///    glF->glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
        ///}
        ///
        ///// Copy from PBO to FBO's texture
        ///glF->glBindTexture(GL_TEXTURE_2D, texID);
        ///glF->glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, 0);
        ///
        ///// Cleanup
        ///glF->glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);


        fbo->release();

        

        // Restore previously active FBO / Viewport Size
        glF->glBindFramebuffer(GL_FRAMEBUFFER, prev_fbo);
        glF->glViewport(0, 0, old_width, old_height);
    }

    void clear(float r = 0.0f, float g = 0.0f, float b = 0.0f, float a = 0.0f)
    {
        QOpenGLExtraFunctions* glF = QOpenGLContext::currentContext()->extraFunctions();

        // Clear the framebuffer
        glF->glClearColor(r, g, b, a);
        glF->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    bool prepared()
    {
        return (fbo != nullptr);
    }

    float aspectRatio()
    {
        return (width / height);
    }
};

//typedef std::shared_ptr<GLSurface> GLSurface;

class GLBitmap : public GLSurface
{
    uint8_t* data = nullptr;

    int bmp_width;
    int bmp_height;
    size_t bmp_size;

    GLuint pbo;

public:

    void create(int w, int h)
    {
        bmp_width = w;
        bmp_height = h;
        bmp_size = w * h * 4;
        //prepare(w, h);

        if (data)
            delete[] data;

        data = new uint8_t[bmp_size];
    }

    size_t size()
    {
        return bmp_size;
    }

    int width()
    {
        return bmp_width;
    }

    int height()
    {
        return bmp_height;
    }

    void bind()
    {
        QOpenGLExtraFunctions* glF = QOpenGLContext::currentContext()->extraFunctions();

        GLSurface::bind();

        // Pixel write-access
        glF->glGenBuffers(1, &pbo);
        glF->glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo);
        glF->glBufferData(GL_PIXEL_UNPACK_BUFFER, bmp_width * bmp_height * 4, NULL, GL_DYNAMIC_DRAW);
        glF->glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

        GLSurface::release();
    }

    void updatePixels()
    {
        if (!prepared())
            prepare(bmp_width, bmp_height);

        bind();

        QOpenGLExtraFunctions* glF = QOpenGLContext::currentContext()->extraFunctions();

        glF->glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo);

        // Write pixels
        uint8_t* pixels = (uint8_t*)glF->glMapBufferRange(GL_PIXEL_UNPACK_BUFFER, 0,
            bmp_width * bmp_height * 4, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);

        if (pixels)
        {
            memcpy(pixels, data, bmp_width * bmp_height * 4);
            glF->glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
        }

        // Copy from PBO to FBO's texture
        glF->glBindTexture(GL_TEXTURE_2D, fbo->texture());
        glF->glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, bmp_width, bmp_height, GL_RGBA, GL_UNSIGNED_BYTE, 0);
        glF->glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

        release();
        ///GLSurface::release();
    }

    void setPixel(int x, int y, uint8_t r, uint8_t g, uint8_t b, uint8_t a)
    {
        int i = (y * bmp_width + x) * 4;
        data[i] = r;
        data[++i] = g;
        data[++i] = b;
        data[++i] = a;
    }

    void setPixelSafe(int x, int y, uint8_t r, uint8_t g, uint8_t b, uint8_t a)
    {
        if (x < 0 || x >= bmp_width ||
            y < 0 || y >= bmp_height)
        {
            return;
        }

        int i = (y * bmp_width + x) * 4;
        data[i] = r;
        data[++i] = g;
        data[++i] = b;
        data[++i] = a;
    }

    void update()
    {

    }
};

///typedef std::shared_ptr<GLBitmap> GLBitmap;

/*
class Bitmap
{
    std::vector<GLubyte> data;

    QOpenGLFramebufferObject* m_fbo = nullptr;
    GLint previousFBO = 0;

    int width = 0;
    int height = 0;

public:

    Bitmap()
    {
    }

    ~Bitmap()
    {
        if (m_fbo)
        {
            m_fbo->release();
            delete m_fbo;
        }
    }

    void create(int w, int h, bool smoothing = false)
    {
        width = w;
        height = h;
    }

    void draw(PaintContext* ctx, double x, double y)
    {

    }

    void setPixelSafe(int x, int y, uchar r, uchar g, uchar b, uchar a)
    {
        if (x < 0 || x >= width ||
            y < 0 || y >= height)
        {
            return;
        }

        int i = (y * width + x) * 4;
        data[i] = r;
        data[i + 1] = g;
        data[i + 2] = b;
        data[i + 3] = a;
    }
};
*/
class OffscreenNanoPainter
{
    int width = 0;
    int height = 0;

    int old_vw = 0;
    int old_vh = 0;

    bool capture_frame = false;
    std::vector<GLubyte> data;

    QNanoPainter* painter = nullptr;
    QOpenGLFramebufferObject* m_fbo = nullptr; 

    GLint previousFBO = 0;
    
   void readPixels();

public:

    OffscreenNanoPainter();
    ~OffscreenNanoPainter();

    QNanoPainter *begin(int w, int h, bool capture_pixels);
    void end();

    void drawToPainter(QNanoPainter* p, double x = 0, double y = 0);
    void drawToPainter(QNanoPainter* p, double x, double y, double w, double h);

    const std::vector<GLubyte> &getPixels();

    QImage toImage() const;
    QNanoPainter* getPainter()
    {
        return painter;
    }
};



/*class OffscreenGLSurface
{
    int m_width = 0;
    int m_height = 0;

    int old_vw = 0;
    int old_vh = 0;

    std::vector<std::unique_ptr<GLSurface>> fbos;
    size_t current_fbo_index = 0;

    QOpenGLFramebufferObject* activeFBO()
    {
        return fbos[current_fbo_index]->fbo;
    }

    GLint previousFBO = 0;


public:

    OffscreenGLSurface();
    ~OffscreenGLSurface();

    GLSurface* newSurface();
    //void setActiveSurface(GLSurface* surface, int w=0, int h=0);

    //QOpenGLExtraFunctions* begin(int w, int h);
    //void end();


    void newFrame()
    {
        current_fbo_index = 0;
    }


    //void drawToPainter(QNanoPainter* p, double x = 0, double y = 0);

};*/

namespace Draw
{
    void arrow(PaintContext* ctx, Vec2 &a, Vec2 &b, QColor color = QColor({ 255,255,255 }), double arrow_size=-1.0);
}