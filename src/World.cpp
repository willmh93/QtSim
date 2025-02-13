#include "World.h"
#include "Simulation.h"
#include <cmath>

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

void Camera::cameraToWorld(double left, double top, double right, double bottom, bool stretch)
{
    double world_w = right - left;
    double world_h = bottom - top;
    rotation = 0;

    if (stretch)
    {
        x = left + (viewport_w / 2) / zoom_x;
        y = top + (viewport_h / 2) / zoom_y;
        zoom_x = (viewport_w / world_w);
        zoom_y = (viewport_h / world_h);
    }
    else
    {
        double aspect_view = viewport_w / viewport_h;
        double aspect_rect = world_w / world_h;

        if (aspect_rect > aspect_view)
        {
            // Shrink Height, gap top and bottom
            zoom_x = zoom_y = (viewport_w / world_w);
            pan_x = 0;
            pan_y = ((viewport_h - (world_h * zoom_y)) / 2.0) / zoom_y;
        }
        else
        {
            // Shrink Width, gap left and right
            zoom_x = zoom_y = (viewport_h / world_h);
            pan_x = ((viewport_w - (world_w * zoom_x)) / 2.0) / zoom_x;
            pan_y = 0;
        }

        x = (left + (viewport_w / 2) / zoom_x);
        y = (top + (viewport_h / 2) / zoom_y);
    }
}

void Camera::cameraToWorld(const FRect& r, bool stretch)
{
    cameraToWorld(r.x1, r.y1, r.x2, r.y2, stretch);
}

void Camera::originToCenterViewport()
{
    x = 0;
    y = 0;
}

Vec2 Camera::toWorld(const Vec2& pt)
{
    // World coordinate
    double px = pt.x;
    double py = pt.y;

    double cos_r = cos(rotation);
    double sin_r = sin(rotation);

    double viewport_cx = (panel->width / 2.0);
    double viewport_cy = (panel->height / 2.0);

    double origin_ox = (panel->width * (origin_ratio_x - 0.5) * zoom_x);
    double origin_oy = (panel->height * (origin_ratio_y - 0.5) * zoom_y);

    px -= viewport_cx + origin_ox;
    py -= viewport_cy + origin_oy;

    px -= (pan_x * zoom_x);
    py -= (pan_y * zoom_y);

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

Vec2 Camera::toStage(const Vec2& pt)
{
    // World coordinate
    double px = pt.x;
    double py = pt.y;

    double cos_r = cos(rotation);
    double sin_r = sin(rotation);

    double viewport_cx = (panel->width / 2.0);
    double viewport_cy = (panel->height / 2.0);
    double origin_ox   = (panel->width * (origin_ratio_x - 0.5) * zoom_x);
    double origin_oy   = (panel->height * (origin_ratio_y - 0.5) * zoom_y);

    /// Do transform

    px *= zoom_x;
    py *= zoom_y;

    px += -this->x * zoom_x;
    py += -this->y * zoom_y;

    double ret_x = (px * cos_r - py * sin_r);
    double ret_y = (py * cos_r + px * sin_r);

    ret_x += (pan_x * zoom_x);
    ret_y += (pan_y * zoom_y);

    ret_x += viewport_cx + origin_ox;
    ret_y += viewport_cy + origin_oy;

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
    pan_down_x = _x;
    pan_down_y = _y;
    pan_beg_x = targ_pan_x;
    pan_beg_y = targ_pan_y;
    panning = true;
}

void Camera::panDrag(int _x, int _y)
{
    if (panning)
    {
        int dx = _x - pan_down_x;
        int dy = _y - pan_down_y;
        targ_pan_x = pan_beg_x + (double)dx * (pan_mult / zoom_x);
        targ_pan_y = pan_beg_y + (double)dy * (pan_mult / zoom_y);
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
