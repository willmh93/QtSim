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

