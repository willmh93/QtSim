#pragma once

#include "types.h"

struct WorldObject
{
    double x;
    double y;
};

struct Panel;
class Camera
{

public:

    Panel* panel = nullptr;

    //bool enabled = true;
    bool panning_enabled = true;
    bool zooming_enabled = true;

    double x = 0;
    double y = 0;
    double pan_x = 0;
    double pan_y = 0;
    double zoom_x = 1;
    double zoom_y = 1;
    double rotation = 0;

    bool transform_coordinates = true;
    bool scale_lines_text = true;
    bool rotate_text = true;

    void setTransformFilters(
        bool _transform_coordinates,
        bool _scale_line_txt, 
        bool _rotate_text)
    {
        transform_coordinates = _transform_coordinates;
        scale_lines_text = _scale_line_txt;
        rotate_text = _rotate_text;
    }

    void setTransformFilters(bool all)
    {
        transform_coordinates = all;
        scale_lines_text = all;
        rotate_text = all;
    }

    void worldTransform()
    {
        transform_coordinates = true;
        scale_lines_text = true;
        rotate_text = true;
    }

    void stageTransform()
    {
        transform_coordinates = false;
        scale_lines_text = false;
        rotate_text = false;
    }

    void labelTransform()
    {
        transform_coordinates = true;
        scale_lines_text = false;
        rotate_text = false;
    }

    double stage_ox = 0;
    double stage_oy = 0;
    void setStageOffset(double ox, double oy)
    {
        stage_ox = ox;
        stage_oy = oy;
    }

    //Vec2 worldPos(Vec2 stagePos)
    //{
    //    toWorld(stagePos.x, stagePos.y);
    //}

    void textTransform(bool world_position, bool scale_size, bool rotate)
    {
        transform_coordinates = world_position;
        scale_lines_text = scale_size;
        rotate_text = rotate;
    }

    double viewport_w;
    double viewport_h;

    double pan_mult = 1.0;
    int pan_down_x;
    int pan_down_y;
    double pan_beg_x;
    double pan_beg_y;
    bool panning = false;

    //void enable()
    //{
    //    enabled = true;
    //}

    //void scaleGraphics(bool b)
    //{
    //    scale_graphics = b;
    //}

    void setZoom(double zoom)
    {
        zoom_x = zoom;
        zoom_y = zoom;
    }

    /*void setCamera(const Camera& rhs)
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
        double camera_rotation = 0);*/

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

    void cameraToWorld(const FRect &r, bool stretch = false);

    void originToCenterViewport();

    inline Vec2 toStageOffset(const Vec2& world_offset)
    {
        if (this->transform_coordinates)
        {
            return world_offset;
        }
        else
        {
            double cos_r = cos(rotation);
            double sin_r = sin(rotation);

            return {
                (world_offset.x * cos_r - world_offset.y * sin_r) * zoom_x,
                (world_offset.y * cos_r + world_offset.x * sin_r) * zoom_y
            };
        }
    }
    inline Vec2 toWorldOffset(const Vec2& stage_offset)
    {
        if (this->transform_coordinates)
        {
            double cos_r = cos(rotation);
            double sin_r = sin(rotation);

            return {
                (stage_offset.x * cos_r + stage_offset.y * sin_r) / zoom_x,
                (stage_offset.y * cos_r - stage_offset.x * sin_r) / zoom_y
            };
        }
        else
        {
            return stage_offset;
        }
    }

    Vec2 toStageOffset(double world_ox, double world_oy)
    {
        return toStageOffset({ world_ox, world_oy });
    }
    Vec2 toWorldOffset(double stage_ox, double stage_oy)
    {
        return toWorldOffset({ stage_ox, stage_oy });
    }

    Vec2 toWorld(const Vec2& pt);
    Vec2 toWorld(double x, double y);
    FRect toWorldRect(const FRect& r);

    Vec2 toStage(const Vec2& pt);
    Vec2 toStage(double x, double y);
    Vec2 toStageSize(const Vec2& size);
    Vec2 toStageSize(double w, double h);
    FRect toStageRect(double x0, double y0, double x1, double y1);
    FRect toStageRect(const Vec2& pt1, const Vec2& pt2);


    void panBegin(int _x, int _y)
    {
        pan_down_x = _x;
        pan_down_y = _y;
        pan_beg_x = pan_x;
        pan_beg_y = pan_y;
        panning = true;
    }

    void panProcess(int _x, int _y)
    {
        if (panning)
        {
            int dx = _x - pan_down_x;
            int dy = _y - pan_down_y;
            pan_x = pan_beg_x + (double)dx * (pan_mult / zoom_x);
            pan_y = pan_beg_y + (double)dy * (pan_mult / zoom_y);
        }
    }

    void panEnd(int _x, int _y)
    {
        panProcess(_x, _y);
        panning = false;
    }
};

