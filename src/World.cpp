#include "World.h"
#include <cmath>

/*void Camera::setCamera(
    double camera_x,
    double camera_y,
    double camera_zoom_x,
    double camera_zoom_y,
    double camera_rotation)
{
    //enabled = true;
    x = camera_x;
    y = camera_y;
    zoom_x = camera_zoom_x;
    zoom_y = camera_zoom_y;
    rotation = camera_rotation;
}*/

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
    /*double cos_r = cos(rotation);
    double sin_r = sin(rotation);

    // Rotate
    double x_new = (cos_r * pt.x - sin_r * pt.y);
    double y_new = (sin_r * pt.x + cos_r * pt.y);

    // Zoom
    x_new *= zoom_x;
    y_new *= zoom_y;

    // Translate
    x_new += viewport_w / 2 + (pan_x * zoom_x) - (x * zoom_x);
    y_new += viewport_h / 2 + (pan_y * zoom_y) - (y * zoom_y);

    return { x_new, y_new };*/

    double cos_r = cos(rotation);
    double sin_r = sin(rotation);

    double px = pt.x;
    double py = pt.y;

    /// Translations

    // Point
    px -= (x * zoom_x);
    py -= (y * zoom_y);

    // Pan
    px += (pan_x * zoom_x);
    py += (pan_y * zoom_y);

    // Center viewport
    px += (viewport_w / 2);
    py += (viewport_h / 2);

    // Zoom
    px /= zoom_x;
    py /= zoom_y;
    
    // Apply inverse rotation
    double x_orig = px * cos_r - py * sin_r;
    double y_orig = py * cos_r + px * sin_r;

    return { x_orig, y_orig };

    /// Untranslate

    // Point
    //px -= (x / zoom_x);
    //py -= (y / zoom_y);
    //
    //// Unpan
    //px -= (pan_x / zoom_x);
    //py -= (pan_y / zoom_y);

    //// Uncenter viewport
    //px -= (viewport_w / 2);
    //py -= (viewport_h / 2);

    //// Unzoom
    //px /= zoom_x;
    //py /= zoom_y;

    //// Unrotate
    //double x_orig = px * cos_r - py * sin_r;
    //double y_orig = py * cos_r + px * sin_r;

    return { x_orig, y_orig };
    /*// Precompute trigonometric values
    double cos_r = cos(rotation);
    double sin_r = sin(rotation);

    // Compute translation offsets
    double tx = viewport_w / 2 + (pan_x * zoom_x) - (x * zoom_x);
    double ty = viewport_h / 2 + (pan_y * zoom_y) - (y * zoom_y);

    // Reverse transformations
    double x_unzoomed = (pt.x - tx) / zoom_x;
    double y_unzoomed = (pt.y - ty) / zoom_y;

    // Apply inverse rotation
    double x_orig = cos_r * x_unzoomed + sin_r * y_unzoomed;
    double y_orig = -sin_r * x_unzoomed + cos_r * y_unzoomed;

    return { x_orig, y_orig };*/
}

Vec2 Camera::toWorld(double x, double y)
{
    return toWorld({ x, y });
}

Vec2 Camera::toStage(const Vec2& pt)
{
    // Precompute trigonometric values
    double cos_r = cos(rotation);
    double sin_r = sin(rotation);

    double px = pt.x;
    double py = pt.y;


    // p->rotate(main_cam.rotation);
    px = zoom_x * (cos_r * px - sin_r * py);
    py = zoom_y * (sin_r * px + cos_r * py);
    
    // p->translate( (main_cam.pan_x * main_cam.zoom_x),
    //               (main_cam.pan_y * main_cam.zoom_y) );
    px += (pan_x * zoom_x);
    py += (pan_y * zoom_y);

    // p->translate( -main_cam.x * main_cam.zoom_x,
    //               -main_cam.y * main_cam.zoom_y);
    px += (this->x * zoom_x);
    py += (this->y * zoom_y);

    //p->scale(main_cam.zoom_x, main_cam.zoom_y);
    //px *= zoom_x;
    //py *= zoom_y;

    return { px, py };

    ////////

    /*// Compute translation offsets
    double tx = viewport_w / 2 + (pan_x * zoom_x) - (x * zoom_x);
    double ty = viewport_h / 2 + (pan_y * zoom_y) - (y * zoom_y);

    // Apply transformations
    double x_new = zoom_x * (cos_r * pt.x - sin_r * pt.y) + tx;
    double y_new = zoom_y * (sin_r * pt.x + cos_r * pt.y) + ty;

    return { x_new, y_new };*/
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
