#pragma once
#include <QObject>
#include <QImage>
#include <QOpenGLFramebufferObject>
#include "qnanopainter.h"
#include "types.h"

struct DrawingContext;

class Bitmap
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

    void draw(DrawingContext* ctx, double x, double y, double w, double h);

    void draw(DrawingContext* ctx, const Vec2 &pt, const Vec2 &size);

    void setPixel(int x, int y, uchar r, uchar g, uchar b, uchar a)
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

class OffscreenNanoPainter
{
    int m_width = 0;
    int m_height = 0;

    int old_vw = 0;
    int old_vh = 0;

    bool capture_frame = false;
    std::vector<GLubyte> data;

    QNanoPainter* painter = nullptr;
    QOpenGLFramebufferObject* m_fbo = nullptr; 
    
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

namespace Draw
{
    void arrow(DrawingContext* ctx, Vec2 &a, Vec2 &b, QColor color = QColor({ 255,255,255 }), double arrow_size=-1.0);
}