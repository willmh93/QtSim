#pragma once

#include "qnanopainter.h"
#include "World.h"

using LineCap = QNanoPainter::LineCap;
using LineJoin = QNanoPainter::LineJoin;
using TextAlign = QNanoPainter::TextAlign;
using TextBaseline = QNanoPainter::TextBaseline;

class Project;
class Bitmap;

struct DrawingContext
{
    friend class Project;

private:
    double _avgZoom()
    {
        return (camera.zoom_x + camera.zoom_y) * 0.5;
    }

protected:

    double old_width = 0;
    double old_height = 0;
    bool just_resized = false;

public:

    // Determined each frame
    double width = 0;
    double height = 0;

    bool resized()
    {
        return just_resized;
    }

    Camera camera;
    QNanoPainter* painter = nullptr;
    QTransform default_viewport_transform;
    QNanoFont font;

    DrawingContext() : font(QNanoFont::FontId::DEFAULT_FONT_NORMAL)
    {}

    Vec2 PT(double x, double y)
    {
        if (camera.transform_coordinates)
        {
            Vec2 o = camera.toWorldOffset({ camera.stage_ox, camera.stage_oy });
            return { x + o.x, y + o.y };
        }
        else
        {
            // (x,y) represents stage coordinate, but transform is active
            Vec2 ret = camera.toWorld(x + camera.stage_ox, y + camera.stage_oy);
            return ret;
        }
    }

    double sharpX(double x)
    {
        //return x;
        //return floor(x) + 0.5;
        return std::floor(x * camera.zoom_x) / camera.zoom_x + 0.5 / camera.zoom_x;
    }

    double sharpY(double y)
    {
        //return y;
        //return floor(y) + 0.5;
        return std::floor(y * camera.zoom_y) / camera.zoom_y + 0.5 / camera.zoom_y;
    }

    Vec2 sharp(const Vec2& p)
    {
        //return p;
        //return {
        //    floor(p.x) + 0.5,
        //    floor(p.y) + 0.5
        //};
         
        return {
            std::round(p.x * camera.zoom_x) / camera.zoom_x + 0.5 / camera.zoom_x,
            std::round(p.y * camera.zoom_y) / camera.zoom_y + 0.5 / camera.zoom_y
        };
    }

    void save()
    {
        painter->save();
    }

    void restore()
    {
        painter->restore();
    }

    void translate(double tx, double ty)
    {
        painter->translate(tx, ty);
    }

    void rotate(double angle)
    {
        painter->rotate(angle);
    }

    void beginPath()
    {
        painter->beginPath();
    }

    void setFillStyle(int r, int g, int b, int a = 255)
    {
        painter->setFillStyle({ r,g,b,a });
    }

    void setFillStyle(const QNanoColor& color)
    {
        painter->setFillStyle(color);
    }

    //void setFillStyle(const QColor &color)
    //{
    //    painter->setFillStyle(QNanoColor::fromQColor(color));
    //}

    void setFillStyle(Qt::GlobalColor color)
    {
        painter->setFillStyle(QNanoColor::fromQColor(color));
    }

    void setStrokeStyle(int r, int g, int b, int a = 255)
    {
        painter->setStrokeStyle({ r,g,b,a });
    }

    void setStrokeStyle(const QNanoColor& color)
    {
        painter->setStrokeStyle(color);
    }

    double line_width = 1;
    void setLineWidth(double w)
    {
        this->line_width = w;

        if (camera.scale_lines_text)
        {
            painter->setLineWidth(w);
        }
        else
        {
            painter->setLineWidth(w / _avgZoom());
        }
    }

    void setLineCap(LineCap cap)
    {
        painter->setLineCap(cap);
    }

    void fill()
    {
        painter->fill();
    }

    void stroke()
    {
        painter->stroke();
    }

    void circle(double cx, double cy, double r)
    {
        Vec2 pt = PT(cx, cy);
        if (camera.scale_lines_text)
        {
            painter->circle(pt.x, pt.y, r);
        }
        else
        {
            painter->circle(pt.x, pt.y, r / _avgZoom());
        }
    }

    void circle(Vec2 cen, double r)
    {
        Vec2 pt = PT(cen.x, cen.y);
        if (camera.scale_lines_text)
        {
            painter->circle(pt.x, pt.y, r);
        }
        else
        {
            painter->circle(pt.x, pt.y, r / _avgZoom());
        }
    }

    void moveTo(double px, double py)
    {
        Vec2 pt = PT(px, py);
        painter->moveTo(pt.x, pt.y);
    }

    void moveTo(Vec2 p)
    {
        Vec2 pt = PT(p.x, p.y);
        painter->moveTo(pt.x, pt.y);
    }

    void lineTo(double px, double py)
    {
        Vec2 pt = PT(px, py);
        painter->lineTo(pt.x, pt.y);
    }

    void lineTo(Vec2 p)
    {
        Vec2 pt = PT(p.x, p.y);
        painter->lineTo(pt.x, pt.y);
    }

    void moveToSharp(double px, double py)
    {
        moveTo(px, py);
        //Vec2 pt = sharp(PT(px, py));
        //painter->moveTo(pt.x, pt.y);
    }

    void moveToSharp(Vec2 p)
    {
        moveTo(p.x, p.y);
        //Vec2 pt = sharp(PT(p.x, p.y));
        //painter->moveTo(pt.x, pt.y);
    }

    void lineToSharp(double px, double py)
    {
        lineTo(px, py);
        //Vec2 pt = sharp(PT(px, py));
        //painter->lineTo(pt.x, pt.y);
    }

    void lineToSharp(Vec2 p)
    {
        lineTo(p.x, p.y);
        //Vec2 pt = sharp(PT(p.x, p.y));
        //painter->lineTo(pt.x, pt.y);
    }

    void strokeRect(double x, double y, double w, double h)
    {
        if (camera.transform_coordinates)
        {
            if (camera.scale_lines_text)
            {
                painter->strokeRect(x, y, w, h);
            }
            else
            {
                double old_linewidth = line_width;
                painter->setLineWidth(line_width / _avgZoom());
                painter->strokeRect(x, y, w, h);
                painter->setLineWidth(old_linewidth);
            }
        }
        else
        {
            QTransform cur_transform = painter->currentTransform();
            painter->resetTransform();
            painter->transform(default_viewport_transform);

            if (camera.scale_lines_text)
            {
                double old_linewidth = line_width;
                painter->setLineWidth(line_width * _avgZoom());
                painter->strokeRect(x, y, w, h);
                painter->setLineWidth(old_linewidth);
            }
            else
            {
                painter->setLineWidth(line_width); // Refresh cached line width
                painter->strokeRect(x, y, w, h);
            }

            painter->resetTransform();
            painter->transform(cur_transform);
        }
    }

    void strokeRect(const FRect& r)
    {
        painter->strokeRect(
            r.x1,
            r.y1,
            r.x2 - r.x1,
            r.y2 - r.y1
        );
    }

    void drawBitmap(Bitmap* bmp, double x, double y, double w, double h);;

    void setFont(QNanoFont font)
    {
        this->font = font;
        painter->setFont(this->font);
    }

    void fillRect(double x, double y, double w, double h)
    {
        painter->fillRect(x, y, w, h);
    }

    void fillRect(const FRect& r)
    {
        painter->strokeRect(
            r.x1,
            r.y1,
            r.x2 - r.x1,
            r.y2 - r.y1
        );
    }

    void fillText(const QString& txt, double px, double py)
    {
        if (camera.transform_coordinates)
        {
            if (camera.scale_lines_text)
            {
                painter->fillText(txt, px, py);
            }
            else
            {
                //double old_linewidth = line_width;
                //painter->setLineWidth(line_width / _avgZoom());
                //painter->strokeRect(x, y, w, h);
                //painter->setLineWidth(old_linewidth);
                painter->fillText(txt, px, py);
            }
        }
        else
        {
            QTransform cur_transform = painter->currentTransform();
            painter->resetTransform();
            painter->transform(default_viewport_transform);

            if (camera.scale_lines_text)
            {
                //double old_linewidth = line_width;
                //painter->setLineWidth(line_width * _avgZoom());
                //painter->strokeRect(x, y, w, h);
                //painter->setLineWidth(old_linewidth);
                painter->fillText(txt, px, py);
            }
            else
            {
                //painter->setLineWidth(line_width); // Refresh cached line width
                //painter->strokeRect(x, y, w, h);
                painter->fillText(txt, px, py);
            }

            painter->resetTransform();
            painter->transform(cur_transform);
        }
    }

    /*void fillText(const QString& txt, double px, double py)
    {
        Vec2 pt = PT(px, py);

        if (camera.scale_lines_text)
        {
            if (camera.rotate_text)
            {
                painter->fillText(txt, pt.x, pt.y);
            }
            else
            {
                painter->save();
                painter->translate(pt.x, pt.y);
                painter->rotate(-camera.rotation);
                painter->fillText(txt, 0, 0);
                painter->restore();
            }
        }
        else
        {
            double s = 1.0 / _avgZoom();
            painter->save();

            painter->translate(pt.x, pt.y);
            painter->scale(s);

            if (!camera.rotate_text)
                painter->rotate(-camera.rotation);

            painter->fillText(txt, 0, 0);
            painter->restore();
        }
    }*/

    void fillText(const QString& txt, const Vec2& pos)
    {
        painter->setPixelAlignText(QNanoPainter::PixelAlign::PIXEL_ALIGN_NONE);
        fillText(txt, pos.x, pos.y);
    }

    void fillTextSharp(const QString& txt, const Vec2& pos)
    {
        painter->setPixelAlignText(QNanoPainter::PixelAlign::PIXEL_ALIGN_FULL);
        fillText(txt, /*sharpX*/(pos.x), /*sharpY*/(pos.y));
    }

    void fillNumberScientific(double v, Vec2 pos, float fontSize=12)
    {
        QString qTxt = QString::number(v);
        int ePos = qTxt.indexOf("e");
        if (ePos >= 0)
        {
            QString exponent_part = qTxt.mid(ePos+1);
            int exponent = exponent_part.toInt();
            QString mantissa_txt = qTxt.slice(0, ePos) + "x10";
            QString exponent_txt = QString::number(exponent);

            painter->setFont(font);
            font.setPixelSize(fontSize);

            pos.x = floor(pos.x);
            pos.y = floor(pos.y);

            float mantissaWidth = boundingBox(mantissa_txt).x2 + 1;
            fillTextSharp(mantissa_txt, pos);

            pos.x += mantissaWidth;
            pos.y -= (int)(fontSize * 0.7 + 1);

            font.setPixelSize((int)(fontSize * 0.85));
            painter->setFont(font);
            fillTextSharp(exponent_txt, pos);

            font.setPixelSize(fontSize);
            painter->setFont(font);
        }
        else
        {
            font.setPixelSize(fontSize);
            painter->setFont(font);
            fillTextSharp(qTxt, pos);
        }
    }

    FRect boundingBox(const QString& txt)
    {
        painter->save();
        painter->resetTransform();
        painter->transform(default_viewport_transform);
        auto r = painter->textBoundingBox(txt, 0, 0);
        painter->restore();

        return r;
    }

    FRect boundingBoxNumberScientific(double v, float fontSize=12)
    {
        QString qTxt = QString::number(v);
        int ePos = qTxt.indexOf("e");
        if (ePos >= 0)
        {
            QString exponent_part = qTxt.mid(ePos + 1);
            int exponent = exponent_part.toInt();

            QString mantissa_txt = qTxt.slice(0, ePos) + "x10";
            QString exponent_txt = QString::number(exponent);

            font.setPixelSize(fontSize);
            painter->setFont(font);

            FRect mantissaRect = boundingBox(mantissa_txt);
            
            font.setPixelSize((int)(fontSize * 0.85));
            painter->setFont(font);

            FRect exponentRect = boundingBox(exponent_txt);
            FRect ret = mantissaRect;

            ret.y1 -= (int)(fontSize * 0.7 + 1);
            ret.x2 += exponentRect.width();

            font.setPixelSize(fontSize);
            painter->setFont(font);

            return ret;
        }
        else
        {
            return boundingBox(QString::number(v));
        }
    }

    Vec2 measureText(const QString& txt)
    {
        painter->save();
        painter->resetTransform();
        painter->transform(default_viewport_transform);
        auto r = painter->textBoundingBox(txt, 0, 0);
        painter->restore();

        return { r.width(), r.height() };
    }

    void setTextAlign(TextAlign align)
    {
        painter->setTextAlign(align);
    }

    void setTextBaseline(TextBaseline align)
    {
        painter->setTextBaseline(align);
    }

    void drawWorldAxis(
        double axis_opacity=0.3,
        double grid_opacity=0.04, 
        double text_opacity=0.4
    );
};