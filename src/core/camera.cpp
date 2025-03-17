#include <cmath>
#include "camera.h"
#include "project.h"

void Camera::setTransformFilters(bool _transform_coordinates, bool _scale_line_txt, bool _rotate_text)
{
    transform_coordinates = _transform_coordinates;
    scale_lines_text = _scale_line_txt;
    rotate_text = _rotate_text;
    viewport->setLineWidth(viewport->line_width);
}

void Camera::setTransformFilters(bool all)
{
    transform_coordinates = all;
    scale_lines_text = all;
    rotate_text = all;
    viewport->setLineWidth(viewport->line_width);
}

void Camera::worldTransform()
{
    transform_coordinates = true;
    scale_lines_text = true;
    rotate_text = true;
    viewport->setLineWidth(viewport->line_width);
}

void Camera::stageTransform()
{
    transform_coordinates = false;
    scale_lines_text = false;
    rotate_text = false;
    viewport->setLineWidth(viewport->line_width);
}

void Camera::labelTransform()
{
    transform_coordinates = true;
    scale_lines_text = false;
    rotate_text = false;
    viewport->setLineWidth(viewport->line_width);
}

void Camera::saveCameraTransform()
{
    saved_transform_coordinates = transform_coordinates;
    saved_scale_lines_text = scale_lines_text;
    saved_rotate_text = rotate_text;
}

void Camera::restoreCameraTransform()
{
    transform_coordinates = saved_transform_coordinates;
    scale_lines_text = saved_scale_lines_text;
    rotate_text = saved_rotate_text;
    viewport->setLineWidth(viewport->line_width);
}

// Scale world to fit viewport rect
void Camera::cameraToViewport(
    double left,
    double top,
    double right,
    double bottom)
{
    double world_w = (right - left);
    double world_h = (bottom - top);

    double port_w = viewport_w;
    double port_h = viewport_h;

    //enabled = true;
    rotation = 0;
    zoom_x = port_w / world_w;
    zoom_y = port_h / world_h;
    x = (port_w / 2) / zoom_x;
    y = (port_h / 2) / zoom_y;
}

void Camera::focusWorldRect(double left, double top, double right, double bottom, bool stretch)
{
    double world_w = right - left;
    double world_h = bottom - top;
    rotation = 0;
    pan_x = 0;
    pan_y = 0;

    if (stretch)
    {
        targ_zoom_x = zoom_x = (viewport_w / world_w);
        targ_zoom_y = zoom_y = (viewport_h / world_h);

        Vec2 _originWorldOffset = originWorldOffset();
        x = left + _originWorldOffset.x; /*+ (viewport_w / 2) / zoom_x*/;
        y = top + _originWorldOffset.y; /*+ (viewport_h / 2) / zoom_y*/;
    }
    else
    {
        double aspect_view = viewport_w / viewport_h;
        double aspect_rect = world_w / world_h;

        Vec2 stage_rect_tl;

        if (aspect_rect > aspect_view)
        {
            // Shrink Height, gap top and bottom
            zoom_x = zoom_y = (viewport_w / world_w);
            targ_zoom_x = zoom_x;
            targ_zoom_y = zoom_y;

            Vec2 _originWorldOffset = originWorldOffset();

            double world_ox = 0;
            double world_oy = ((viewport_h - (world_h * zoom_y)) / 2.0) / zoom_y;

            x = _originWorldOffset.x + left - world_ox;
            y = _originWorldOffset.y + top - world_oy;
        }
        else
        {
            // Shrink Width, gap left and right
            zoom_x = zoom_y = (viewport_h / world_h);
            targ_zoom_x = zoom_x;
            targ_zoom_y = zoom_y;

            Vec2 _originWorldOffset = originWorldOffset();

            double world_ox = ((viewport_w - (world_w * zoom_x)) / 2.0) / zoom_x;
            double world_oy = 0;

            x = _originWorldOffset.x + left - world_ox;
            y = _originWorldOffset.y + top - world_oy;
        }

        //x = (left + (viewport_w / 2) / zoom_x);
        //y = (top + (viewport_h / 2) / zoom_y);
    }
}

void Camera::focusWorldRect(const FRect& r, bool stretch)
{
    focusWorldRect(r.x1, r.y1, r.x2, r.y2, stretch);
}

void Camera::originToCenterViewport()
{
    x = 0;
    y = 0;
}

Vec2 Camera::originPixelOffset()
{
    double viewport_cx = (viewport->width / 2.0);
    double viewport_cy = (viewport->height / 2.0);

    return Vec2(
         viewport_cx +  viewport->width * (focal_anchor_x - 0.5),// * zoom_x,
         viewport_cy +  viewport->height * (focal_anchor_y - 0.5)// * zoom_y
    );
}

Vec2 Camera::originWorldOffset()
{
    return originPixelOffset() / Vec2(zoom_x, zoom_y);
}

Vec2 Camera::panPixelOffset()
{
    return Vec2(
        pan_x * zoom_x,
        pan_y * zoom_y
    );
}

Vec2 Camera::toWorld(const Vec2& pt)
{
    // World coordinate
    double px = pt.x;
    double py = pt.y;

    double cos_r = cos(rotation);
    double sin_r = sin(rotation);

    

    double origin_ox = originPixelOffset().x;//(viewport->width * (focal_anchor_x - 0.5) * zoom_x);
    double origin_oy = originPixelOffset().y;//(viewport->height * (focal_anchor_y - 0.5) * zoom_y);

    px -= origin_ox;
    py -= origin_oy;

    px -= panPixelOffset().x; //pan_x * zoom_x;
    py -= panPixelOffset().y; //pan_y * zoom_y;

    double world_x = (px * cos_r + py * sin_r);
    double world_y = (py * cos_r - px * sin_r);

    world_x -= -this->x * zoom_x;
    world_y -= -this->y * zoom_y;

    world_x /= zoom_x;
    world_y /= zoom_y;

    return { world_x, world_y };
}

Vec2 Camera::toWorld(double x, double y)
{
    return toWorld({ x, y });
}

FRect Camera::toWorldRect(const FRect& r)
{
    Vec2 tl = toWorld(r.x1, r.y1);
    Vec2 br = toWorld(r.x2, r.y2);
    return { tl.x, tl.y, br.x, br.y };
}

FRect Camera::toWorldRect(double x1, double y1, double x2, double y2)
{
    Vec2 tl = toWorld(x1, y1);
    Vec2 br = toWorld(x2, y2);
    return { tl.x, tl.y, br.x, br.y };
}

Vec2 Camera::toStage(const Vec2& pt)
{
    // World coordinate
    double px = pt.x;
    double py = pt.y;

    double cos_r = cos(rotation);
    double sin_r = sin(rotation);

    double origin_ox   = originPixelOffset().x;//(viewport->width * (focal_anchor_x - 0.5) * zoom_x);
    double origin_oy   = originPixelOffset().y;//(viewport->height * (focal_anchor_y - 0.5) * zoom_y);

    /// Do transform

    px *= zoom_x;
    py *= zoom_y;

    px += -this->x * zoom_x;
    py += -this->y * zoom_y;

    double ret_x = (px * cos_r - py * sin_r);
    double ret_y = (py * cos_r + px * sin_r);

    ret_x += panPixelOffset().x;//pan_x * zoom_x;
    ret_y += panPixelOffset().y;//pan_y * zoom_y;

    ret_x += origin_ox;
    ret_y += origin_oy;

    return { ret_x, ret_y };
}

Vec2 Camera::toStage(double x, double y)
{
    return toStage({ x, y });
}

Vec2 Camera::toStageSize(const Vec2& size)
{
    return { size.x * zoom_x, size.y * zoom_y };
}

Vec2 Camera::toStageSize(double w, double h)
{
    return { w * zoom_x, h * zoom_y };
}

FRect Camera::toStageRect(double x0, double y0, double x1, double y1)
{
    Vec2 p1 = toStage({ x0, y0 });
    Vec2 p2 = toStage({ x1, y1 });
    return { p1.x, p1.y, p2.x, p2.y };
}

FRect Camera::toStageRect(const Vec2& pt1, const Vec2& pt2)
{
    return toStageRect(pt1.x, pt1.y, pt2.x, pt2.y);
}

void Camera::panBegin(int _x, int _y)
{
    pan_down_mx = _x;
    pan_down_my = _y;
    pan_beg_x = targ_pan_x;
    pan_beg_y = targ_pan_y;
    panning = true;
}

void Camera::panDrag(int _x, int _y)
{
    if (panning)
    {
        int dx = _x - pan_down_mx;
        int dy = _y - pan_down_my;
        targ_pan_x = pan_beg_x + (double)(dx / zoom_x) * pan_mult;
        targ_pan_y = pan_beg_y + (double)(dy / zoom_y) * pan_mult;
    }
}

void Camera::panProcess()
{
    double ease = 0.1;
    pan_x += (targ_pan_x - pan_x) * ease;
    pan_y += (targ_pan_y - pan_y) * ease;
    zoom_x += (targ_zoom_x - zoom_x) * ease;
    zoom_y += (targ_zoom_y - zoom_y) * ease;
}

void Camera::panEnd(int _x, int _y)
{
    panDrag(_x, _y);
    panning = false;
}
