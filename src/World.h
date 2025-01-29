#pragma once

#include "types.h"

struct WorldObject
{
    double x;
    double y;
};

struct Camera
{
    bool enabled = true;
    bool panning_enabled = true;
    bool zooming_enabled = true;

    double x = 0;
    double y = 0;
    double pan_x = 0;
    double pan_y = 0;
    double zoom_x = 1;
    double zoom_y = 1;
    double rotation = 0;

    double viewport_w;
    double viewport_h;

    double pan_mult = 2.0;
    int pan_down_x;
    int pan_down_y;
    double pan_beg_x;
    double pan_beg_y;
    bool panning = false;

    void enable()
    {
        enabled = true;
    }

    void setZoom(double zoom)
    {
        zoom_x = zoom;
        zoom_y = zoom;
    }

    void setCamera(const Camera& rhs)
    {
        x = rhs.x;
        y = rhs.y;
        pan_x = rhs.pan_x;
        pan_y = rhs.pan_y;
        zoom_x = rhs.zoom_x;
        zoom_y = rhs.zoom_y;
        rotation = rhs.rotation;
        viewport_w = rhs.viewport_w;
        viewport_h = rhs.viewport_h;
    }

    void setCamera(
        double camera_x,
        double camera_y,
        double camera_zoom_x,
        double camera_zoom_y,
        double camera_rotation = 0);

    void cameraToViewport(
        double left,
        double top,
        double right,
        double bottom);

    void cameraToWorld(
        double left,
        double top,
        double right,
        double bottom,
        bool stretch=false
    );

    Vec2 toStage(const Vec2& pt);
    Vec2 toStage(double x, double y);
    Vec2 toStageSize(const Vec2& size);
    Vec2 toStageSize(double w, double h);
    FRect toStageRect(double x0, double y0, double x1, double y1);
    FRect toStageRect(const Vec2& pt1, const Vec2& pt2);


    void panBegin(int x, int y)
    {
        pan_down_x = x;
        pan_down_y = y;
        pan_beg_x = pan_x;
        pan_beg_y = pan_y;
        panning = true;
    }

    void panProcess(int x, int y)
    {
        if (panning)
        {
            int dx = x - pan_down_x;
            int dy = y - pan_down_y;
            pan_x = pan_beg_x + (double)dx * (pan_mult / zoom_x);
            pan_y = pan_beg_y + (double)dy * (pan_mult / zoom_y);
        }
    }

    void panEnd(int x, int y)
    {
        panProcess(x, y);
        panning = false;
    }
};

