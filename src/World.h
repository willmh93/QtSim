#pragma once

#include "types.h"

struct WorldObject
{
    double x;
    double y;
};

class Viewport;
class Camera
{

public:

    Viewport* viewport = nullptr;

    //bool enabled = true;
    bool panning_enabled = true;
    bool zooming_enabled = true;

    double x = 0;
    double y = 0;
    double zoom_x = 1;
    double zoom_y = 1;
    double rotation = 0;
    double targ_zoom_x = 1;
    double targ_zoom_y = 1;

    double pan_x = 0;
    double pan_y = 0;
    double targ_pan_x = 0;
    double targ_pan_y = 0;

    bool transform_coordinates = true;
    bool scale_lines_text = true;
    bool rotate_text = true;

    double viewport_w = 0;
    double viewport_h = 0;

    double focal_anchor_x = 0.0;
    double focal_anchor_y = 0.0;

    double pan_mult = 1.0;
    int pan_down_mx = 0;
    int pan_down_my = 0;
    double pan_beg_x = 0;
    double pan_beg_y = 0;
    bool panning = false;

    bool saved_transform_coordinates = transform_coordinates;
    bool saved_scale_lines_text = scale_lines_text;
    bool saved_rotate_text = rotate_text;

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

    void saveCameraTransform()
    {
        saved_transform_coordinates = transform_coordinates;
        saved_scale_lines_text = scale_lines_text;
        saved_rotate_text = rotate_text;
    }

    void restoreCameraTransform()
    {
        transform_coordinates = saved_transform_coordinates;
        scale_lines_text = saved_scale_lines_text;
        rotate_text = saved_rotate_text;
    }

    double stage_ox = 0;
    double stage_oy = 0;
    void setStageOffset(double ox, double oy)
    {
        stage_ox = ox;
        stage_oy = oy;
    }

    void textTransform(bool world_position, bool scale_size, bool rotate)
    {
        transform_coordinates = world_position;
        scale_lines_text = scale_size;
        rotate_text = rotate;
    }

    void setOriginViewportAnchor(double ax, double ay)
    {
        focal_anchor_x = ax;
        focal_anchor_y = ay;
    }

    void setOriginViewportAnchor(Anchor anchor)
    {
        switch (anchor)
        {
        case Anchor::TOP_LEFT:
            focal_anchor_x = 0;
            focal_anchor_y = 0;
            break;
        case Anchor::CENTER:
            focal_anchor_x = 0.5;
            focal_anchor_y = 0.5;
            break;
        }
    }

    void setStagePanX(int px)
    {
        pan_x = px / zoom_x;
    }

    void setStagePanY(int py)
    {
        pan_y = py / zoom_x;
    }

    double getStagePanX()
    {
        return pan_x * zoom_x;
    }

    double getStagePanY()
    {
        return pan_x * zoom_x;
    }

    void setZoom(double zoom)
    {
        zoom_x = zoom;
        zoom_y = zoom;
    }

    void cameraToViewport(double left, double top, double right, double bottom);
    void focusWorldRect(double left, double top, double right, double bottom, bool stretch=false);
    void focusWorldRect(const FRect &r, bool stretch = false);

    void originToCenterViewport();

    Vec2 toStageOffset(const Vec2& world_offset)
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
    Vec2 toWorldOffset(const Vec2& stage_offset)
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

    Vec2 originPixelOffset();
    Vec2 originWorldOffset();
    Vec2 panPixelOffset();

    Vec2 toWorld(const Vec2& pt);
    Vec2 toWorld(double x, double y);
    FRect toWorldRect(const FRect& r);

    Vec2 toStage(const Vec2& pt);
    Vec2 toStage(double x, double y);
    Vec2 toStageSize(const Vec2& size);
    Vec2 toStageSize(double w, double h);
    FRect toStageRect(double x0, double y0, double x1, double y1);
    FRect toStageRect(const Vec2& pt1, const Vec2& pt2);

    void panBegin(int _x, int _y);
    void panDrag(int _x, int _y);
    void panEnd(int _x, int _y);
    void panProcess();
};

