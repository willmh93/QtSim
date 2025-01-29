#pragma once
#include <QPoint>

struct Vec2
{
    double x;
    double y;

    Vec2(double _x, double _y)
    {
        x = _x;
        y = _y;
    }

    operator QPointF()
    {
        return QPointF(x, y);
    }
};

struct FRect
{
    double x1;
    double y1;
    double x2;
    double y2;

    FRect(double _x1, double _y1, double _x2, double _y2)
    {
        x1 = _x1;
        y1 = _y1;
        x2 = _x2;
        y2 = _y2;
    }
};

struct Size
{
    int x;
    int y;

    Size(int _x, int _y)
    {
        x = _x;
        y = _y;
    }
};

