#include "World.h"
#include <cmath>

void Camera::setCamera(
    double camera_x,
    double camera_y,
    double camera_zoom_x,
    double camera_zoom_y,
    double camera_rotation)
{
    enabled = true;
    x = camera_x;
    y = camera_y;
    zoom_x = camera_zoom_x;
    zoom_y = camera_zoom_y;
    rotation = camera_rotation;
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

    enabled = true;
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
    //pan_x = 0;
    //pan_y = 0;

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

        double ox = 0;
        double oy = 0;

        if (aspect_rect > aspect_view)
        {
            // Rect too wide, Grow View Height to fit, gap left and right
            zoom_x = (viewport_w / world_w); // (viewport_w / (world_w / zoom_x));
            zoom_y = zoom_x;
            
            pan_x = 0;
            pan_y = ((viewport_h - (world_h * zoom_y)) / 2.0) / zoom_y;
        }
        else
        {
            // Shrink Width, gap left and right
            zoom_y = (viewport_h / world_h); // (viewport_h / (world_h / zoom_y));
            zoom_x = zoom_y;
           
            pan_x = ((viewport_w - (world_w * zoom_x)) / 2.0) / zoom_x;
            pan_y = 0;
        }

        x = (left + (viewport_w / 2) / zoom_x) + ox;
        y = (top + (viewport_h / 2) / zoom_y) + oy;
    }
    
    rotation = 0;
}

Vec2 Camera::toStage(const Vec2& pt)
{
    if (!enabled)
        return pt;

    // Precompute trigonometric values
    double cos_r = cos(rotation);
    double sin_r = sin(rotation);

    // Compute translation offsets
    double tx = viewport_w / 2 + (pan_x * zoom_x) - (x * zoom_x);
    double ty = viewport_h / 2 + (pan_y * zoom_y) - (y * zoom_y);

    // Apply transformations
    double x_new = zoom_x * (cos_r * pt.x - sin_r * pt.y) + tx;
    double y_new = zoom_y * (sin_r * pt.x + cos_r * pt.y) + ty;

    return { x_new, y_new };
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
