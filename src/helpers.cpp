#include "helpers.h"


double min(double a, double b)
{
    if (a < b)
        return a;
    else
        return b;
}

double max(double a, double b)
{
    if (a > b)
        return a;
    else
        return b;
}

FRect lerpRect(const FRect& src, const FRect& targ, double factor)
{
    FRect ret = src;
    ret.x1 += (targ.x1 - src.x1) * factor;
    ret.y1 += (targ.y1 - src.y1) * factor;
    ret.x2 += (targ.x2 - src.x2) * factor;
    ret.y2 += (targ.y2 - src.y2) * factor;
    return ret;
}

QString normalizeSeconds(double seconds)
{
    if (seconds <= 60.0)
        return QString::number(seconds, 'f', 0) + " seconds";

    double minutes = seconds / 60.0;
    if (minutes <= 60.0)
        return QString::number(minutes, 'f', 0) + " minutes";

    double hours = minutes / 60.0;
    if (hours <= 24.0)
        return QString::number(hours, 'f', 1) + " hours";

    double days = hours / 24.0;
    if (days < 365.2422)
        return QString::number(days, 'f', 2) + " days";

    double years = days / 365.2422;
    if (years < 10)
        return QString::number(years, 'f', 2) + " years";

    double million_years = years / 1000000.0;
    if (years < 1000000)
        return QString::number(million_years, 'f', 3) + " million years";

    double billion_years = years / 1000000000.0;
    if (years < 1000000000)
        return QString::number(billion_years, 'f', 3) + " billion years";

    double trillion_years = years / 1000000000000.0;
    return QString::number(trillion_years, 'f', 3) + " trillion years";
}

// |error| < 0.005
float atan2_approximation2(double y, double x)
{
    if (x == 0.0f)
    {
        if (y > 0.0f) return PIBY2_FLOAT;
        if (y == 0.0f) return 0.0f;
        return -PIBY2_FLOAT;
    }
    double atan;
    double z = y / x;
    if (fabs(z) < 1.0f)
    {
        atan = z / (1.0f + 0.28f * z * z);
        if (x < 0.0f)
        {
            if (y < 0.0f) return atan - PI_FLOAT;
            return atan + PI_FLOAT;
        }
    }
    else
    {
        atan = PIBY2_FLOAT - z / (z * z + 0.28f);
        if (y < 0.0f) return atan - PI_FLOAT;
    }
    return atan;
}

double distToPoint(double cx, double cy, double _x, double _y)
{
    double dx = _x - cx;
    double dy = _y - cy;
    return sqrt(dx * dx + dy * dy);
}

Vec2 rotateVec(double cx, double cy, double angle, double x, double y)
{
    double dx = x - cx;
    double dy = y - cy;
    double _cos = cos(angle);
    double _sin = sin(angle);
    double dx2 = _cos * dx + _sin * dy;
    double dy2 = _cos * dy - _sin * dx;
    return { dx2, dy2 };
}

bool lineEqIntersect(Vec2* targ, Ray ray1, Ray ray2, bool bidirectional)
{
    double x1 = ray1.x;
    double y1 = ray1.y;
    double angle1 = ray1.angle;

    double x2 = ray1.x;
    double y2 = ray1.y;
    double angle2 = ray2.angle;

    constexpr double toRad = M_PI / 180.0;
    constexpr double toDeg = 180.0 / M_PI;
    angle1 *= toDeg;
    angle2 *= toDeg;

    if (fmod(fmod(angle1 - angle2, 180.0) + 180.0, 180.0) == 0.0)
    {
        // Parallel, no intersection
        return false;
    }

    bool vert_line = false;
    if (fmod(fmod(angle1, 180.0) + 180.0, 180.0) == 90.0)
    {
        // vertical line at x = x1
        *targ = { x1, tan(angle2 * toRad) * (x1 - x2) + y2 };
        vert_line = true;
    }
    else if (fmod(fmod(angle2, 180.0) + 180.0, 180.0) == 90)
    {
        // vertical line at x = x2
        *targ = { x2, tan(angle1 * toRad) * (x2 - x1) + y1 };
        vert_line = true;
    }

    if (!vert_line)
    {
        double m1 = tan(angle1 * toRad); // Line 1:  y = m1 (x - x1) + y1
        double m2 = tan(angle2 * toRad); // Line 2:  y = m2 (x - x2) + y2
        double x = ((m1 * x1 - m2 * x2) - (y1 - y2)) / (m1 - m2);
        *targ = { x, m1 * (x - x1) + y1 };
    }

    if (!bidirectional)
    {
        // Check that the intersection occurs in the direction of the line projection
        // If it doesn't, return null
        double angle = (angle1 - 90.0);
        Vec2 relDist = rotateVec(x1, y1, angle * toRad, targ->x, targ->y);

        if (relDist.y < 0)
            return false;
    }
    return true;
}

bool getRayRectIntersection(Vec2* targ, const FRect &r, const Vec2 &ray_cen, double ray_angle)
{
    /*constexpr double toRad = M_PI / 180.0;
    constexpr double toDeg = 180.0 / M_PI;

    //double ret = null;
    //ray_angle *= 180.0 / M_PI;
    double closest_intersection = std::numeric_limits<double>::max();

    _RayRectLine rect_lines[] = {
        { r.x1, r.y1, 0 } ,
        { r.x2, r.y1, 90 },
        { r.x2, r.y2, 180 },
        { r.x1, r.y2, 270 }
    };

    bool any_intersect = false;
    for (int i = 0; i < 4; i++)
    {
        _RayRectLine& side = rect_lines[i];

        switch (i)
        {
        case 0: if (ray_cen.y <= r.y1) continue; break; // Ignore top
        case 1: if (ray_cen.x >= r.x2) continue; break; // Ignore right
        case 2: if (ray_cen.y >= r.y2) continue; break; // Ignore bottom
        case 3: if (ray_cen.x <= r.x1) continue; break; // Ignore left
        }

        Vec2 intersection;
        bool intersects = lineEqIntersect(&intersection, 
            ray_cen.x, ray_cen.y, ray_angle, { side.x, side.y, side.angle_degrees * toRad });

        if (intersects)
        {
            double d = distToPoint(ray_cen.x, ray_cen.y, intersection.x, intersection.y);
            if (d < closest_intersection)
            {
                closest_intersection = d;
                *targ = { intersection.x, intersection.y };
                any_intersect = true;
            }
        }
    }

    return any_intersect;*/
    return true;
}
