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
