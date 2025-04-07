#pragma once
#include <QObject>
#include <QImage>
#include <QOpenGLExtraFunctions>
#include <QOpenGLFramebufferObject>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLTexture>
#include "glwrappers.h"

#include "qnanopainter.h"
#include "types.h"

struct PaintContext;

class Bitmap
{
protected:
    std::vector<uchar> data;
    int bmp_width = 0;
    int bmp_height = 0;

    QImage img;
    QNanoImage nano_img;

    static int bmp_index;

public:

    Bitmap() : bmp_width(0), bmp_height(0)
    {
    }

    //Bitmap(const char *filepath) : bmp_width(0), bmp_height(0), nano_img(filepath)
    //{
    //}

    int width()
    {
        return bmp_width;
    }

    int height()
    {
        return bmp_height;
    }

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
};

class CanvasBitmapObject : public Bitmap, public CanvasObject
{
protected:
    double bmp_fw = 0;
    double bmp_fh = 0;

    ///FRect old_world_rect;
    bool needs_reshading = false;
    FQuad prev_world_quad;

    friend class PaintContext;

public:
    //std::function<void(void)> shader = nullptr;

    /*CanvasBitmapObject(CoordinateType t)
    {
        coordinate_type = t;
    }*/

    void setNeedsReshading(bool b = true)
    {
        needs_reshading = b;
    }

    bool needsReshading(PaintContext* ctx)
    {
        FQuad world_quad = getWorldQuad(ctx);
        if (needs_reshading || (world_quad != prev_world_quad))
        {
            needs_reshading = false; // todo: Move to when drawn?
            prev_world_quad = world_quad;
            return true;
        }
        prev_world_quad = world_quad;
        return false;
    }

    /*bool needsReshadingStage(PaintContext* ctx)
    {
        coordinate_type = CoordinateType::STAGE;

        FQuad world_quad = getWorldQuad(ctx);
        if (needs_reshading || (world_quad != prev_world_quad))
        {
            needs_reshading = false; // todo: Move to when drawn?
            prev_world_quad = world_quad;
            return true;
        }
        prev_world_quad = world_quad;
        return false;
    }

    bool needsReshadingWorld(PaintContext* ctx)
    {
        coordinate_type = CoordinateType::WORLD;

        FQuad world_quad = getWorldQuad(ctx);
        if (needs_reshading || (world_quad != prev_world_quad))
        {
            needs_reshading = false; // todo: Move to when drawn?
            prev_world_quad = world_quad;
            return true;
        }
        prev_world_quad = world_quad;
        return false;
    }*/

    /*bool reshadingWorld(double x0, double y0, double x1, double y1)
    {
        _x0 = x0;
        _y0 = y0;
        _x1 = x1;
        _y1 = y1;

        if (transform_stage)
        {
            transform_stage = false;
            needs_reshading = true;
        }

        setWorldRect(x0, y0, x1, y1);

        if (needs_reshading)
        {
            qDebug() << "Reshading";
            needs_reshading = false;
            return true;
        }
        return false;
    }

    bool reshadingStage(Camera* camera, double x0, double y0, double x1, double y1)
    {
        _x0 = x0;
        _y0 = y0;
        _x1 = x1;
        _y1 = y1;

        if (!transform_stage)
        {
            transform_stage = true;
            needs_reshading = true;
        }

        FRect wr = camera->toWorldRect(x0, y0, x1, y1);
        setWorldRect(wr.x1, wr.y1, wr.x2, wr.y2);

        if (needs_reshading)
        {
            needs_reshading = false;
            return true;
        }
        return false;
    }*/

    void setBitmapSize(int bmp_w, int bmp_h)
    {
        if (bmp_w < 0) bmp_w = 0;
        if (bmp_h < 0) bmp_h = 0;

        if (bmp_width != bmp_w || bmp_height != bmp_h)
        {
            bmp_fw = static_cast<double>(bmp_w);
            bmp_fh = static_cast<double>(bmp_h);

            qDebug() << "Resizing";
            create(bmp_w, bmp_h);
            needs_reshading = true;
        }
    }

    FQuad getWorldQuad(PaintContext* ctx);

    // Callback format: void(int x, int y, double wx, double wy)
    template<typename Callback>
    void forEachWorldPixel(
        PaintContext* ctx, 
        Callback&& callback, 
        int thread_count = QThread::idealThreadCount())
    {
        static_assert(std::is_invocable_r_v<void, Callback, int, int, double, double>,
            "Callback must be: void(int x, int y, double wx, double wy)");

        FQuad world_quad = getWorldQuad(ctx);

        double step_wx = (world_quad.b.x - world_quad.a.x) / bmp_fw;
        double step_wy = (world_quad.b.y - world_quad.a.y) / bmp_fw;

        double scanline_origin_dx = (world_quad.d.x - world_quad.a.x) / bmp_fh;
        double scanline_origin_dy = (world_quad.d.y - world_quad.a.y) / bmp_fh;

        double scanline_origin_dist_x = (world_quad.d.x - world_quad.a.x);
        double scanline_origin_dist_y = (world_quad.d.y - world_quad.a.y);

        double scanline_origin_ax = world_quad.a.x;
        double scanline_origin_ay = world_quad.a.y;

        auto row_ranges = splitRanges(bmp_height, thread_count);
        std::vector<QFuture<void>> futures(thread_count);

        if (thread_count > 0)
        {
            for (int ti = 0; ti < thread_count; ++ti)
            {
                auto& row_range = row_ranges[ti];
                futures[ti] = QtConcurrent::run([&, row_range]()
                {
                    double ax = world_quad.a.x;
                    double ay = world_quad.a.y;
                    double bx = world_quad.b.x;
                    double by = world_quad.b.y;
                    double cx = world_quad.c.x;
                    double cy = world_quad.c.y;
                    double dx = world_quad.d.x;
                    double dy = world_quad.d.y;

                    for (int bmp_y = row_range.first; bmp_y < row_range.second; ++bmp_y)
                    {
                        double v = static_cast<double>(bmp_y) / static_cast<double>(bmp_fh);

                        // Interpolate left and right edges of the scanline
                        double scan_left_x = world_quad.a.x + (world_quad.d.x - world_quad.a.x) * v;
                        double scan_left_y = world_quad.a.y + (world_quad.d.y - world_quad.a.y) * v;

                        double scan_right_x = world_quad.b.x + (world_quad.c.x - world_quad.b.x) * v;
                        double scan_right_y = world_quad.b.y + (world_quad.c.y - world_quad.b.y) * v;

                        for (int bmp_x = 0; bmp_x < bmp_width; ++bmp_x)
                        {
                            double u = static_cast<double>(bmp_x) / static_cast<double>(bmp_fw);

                            double wx = scan_left_x + (scan_right_x - scan_left_x) * u;
                            double wy = scan_left_y + (scan_right_y - scan_left_y) * u;

                            std::forward<Callback>(callback)(bmp_x, bmp_y, wx, wy);
                        }
                    }
                });
            }

            for (auto& future : futures)
                future.waitForFinished();
        }
        else
        {
            double origin_x = world_quad.a.x;
            double origin_y = world_quad.a.y;

            for (int bmp_y = 0; bmp_y < bmp_height; bmp_y++)
            {
                double wx = origin_x;
                double wy = origin_y;
                for (int bmp_x = 0; bmp_x < bmp_width; bmp_x++)
                {
                    std::forward<Callback>(callback)(bmp_x, bmp_y, wx, wy);
                    wx += step_wx;
                    wy += step_wy;
                }
                origin_x += scanline_origin_dx;
                origin_y += scanline_origin_dy;
            }
        }
    }
};

struct FBOHolder 
{
    std::shared_ptr<QOpenGLFramebufferObject> fbo;

    FBOHolder(int width, int height, const QOpenGLFramebufferObjectFormat& format)
        : fbo(std::make_shared<QOpenGLFramebufferObject>(width, height, format)) {}

    void recreate(int width, int height, const QOpenGLFramebufferObjectFormat& format) {
        fbo = std::make_shared<QOpenGLFramebufferObject>(width, height, format);
    }

    void bind()
    {
        fbo->bind();
    }

    void release()
    {
        fbo->release();
    }

    GLuint texture()
    {
        return fbo->texture();
    }
};

struct GLSurface
{
    std::shared_ptr<FBOHolder> holder;

    GLint prev_fbo = 0;
    int width = 0;
    int height = 0;
    int old_width = 0;
    int old_height = 0;

    GLSurface() {}
    GLSurface(const GLSurface& rhs)
    {
        holder = rhs.holder;
        prev_fbo = rhs.prev_fbo;
        width = rhs.width;
        height = rhs.height;
        old_width = rhs.old_width;
        old_height = rhs.old_height;
    }

    const QOpenGLFramebufferObject* fbo() const
    {
        return holder->fbo.get();
    }

    void prepare(int w, int h)
    {
        if (!holder || w != width || h != height)
        {
            width = w;
            height = h;

            QOpenGLFramebufferObjectFormat format;
            format.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
            format.setInternalTextureFormat(GL_RGBA8);

            if (!holder)
                holder = std::make_shared<FBOHolder>(width, height, format);
            else
                holder->recreate(width, height, format);
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
        holder->bind();
        glF->glViewport(0, 0, width, height);
    }

    void release()
    {
        holder->release();

        // Restore previously active FBO / Viewport Size
        QOpenGLExtraFunctions* glF = QOpenGLContext::currentContext()->extraFunctions();
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
        return (holder != nullptr);
    }

    float aspectRatio()
    {
        return (width / height);
    }
};

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

/*namespace Draw
{
    void arrow(PaintContext* ctx, Vec2 &a, Vec2 &b, QColor color = QColor({ 255,255,255 }), double arrow_size=-1.0);
}*/