#include "helpers.h"
#include <sstream>



int countDecimals(double num)
{
    std::ostringstream out;
    out << std::fixed << std::setprecision(10) << num;
    std::string str = out.str();

    size_t pos = str.find('.');
    if (pos == std::string::npos) return 0; 
    return str.size() - pos - 1;
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
        if (y > 0.0f) return (float)PIBY2_FLOAT;
        if (y == 0.0f) return 0.0f;
        return (float)-PIBY2_FLOAT;
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

bool lineEqIntersect(Vec2* targ, const Ray& ray1, const Ray& ray2, bool bidirectional)
{
    // Compute unit direction vectors for each ray.
    double d1x = cos(ray1.angle), d1y = sin(ray1.angle);
    double d2x = cos(ray2.angle), d2y = sin(ray2.angle);

    // Origins:
    double x1 = ray1.x, y1 = ray1.y;
    double x2 = ray2.x, y2 = ray2.y;

    // Denom of the 2x2 system:
    double denom = d1x * d2y - d1y * d2x;
    if (fabs(denom) < 1e-9)
    {
        // Rays are parallel (or nearly so)
        return false;
    }

    // Compute the parameters t and u such that:
    //   ray1.origin + t*d1 = ray2.origin + u*d2
    double dx = x2 - x1;
    double dy = y2 - y1;
    double t = (dx * d2y - dy * d2x) / denom;
    double u = (dx * d1y - dy * d1x) / denom;

    // If we're restricting to the forward direction, both t and u must be nonnegative.
    if (!bidirectional)
    {
        if (t < 0 || u < 0)
            return false;
    }

    // Compute intersection point.
    *targ = Vec2(x1 + t * d1x, y1 + t * d1y);
    return true;
}

bool getRayRectIntersection(Vec2* back_intersect, Vec2* foward_intersect, const FRect& r, const Ray& ray)
{
    // Normalize rect boundaries
    double minX = std::min(r.x1, r.x2);
    double maxX = std::max(r.x1, r.x2);
    double minY = std::min(r.y1, r.y2);
    double maxY = std::max(r.y1, r.y2);

    // Ray origin and direction
    double rx = ray.x;
    double ry = ray.y;
    double dx = cos(ray.angle);
    double dy = sin(ray.angle);

    // Aaccumulate candidate intersections as (t, point) pairs
    struct IntersectionCandidate
    {
        double t;
        Vec2 pt;
    };

    std::vector<IntersectionCandidate> candidates;
    const double eps = 1e-9;

    // --- Check intersection with vertical edges ---

    // Left edge: x = minX
    if (std::fabs(dx) > eps) {
        double t = (minX - rx) / dx;
        double y_int = ry + t * dy;
        if (y_int >= minY - eps && y_int <= maxY + eps)
            candidates.push_back({ t, Vec2(minX, y_int) });
    }

    // Right edge: x = maxX
    if (std::fabs(dx) > eps) {
        double t = (maxX - rx) / dx;
        double y_int = ry + t * dy;
        if (y_int >= minY - eps && y_int <= maxY + eps)
            candidates.push_back({ t, Vec2(maxX, y_int) });
    }

    // --- Check intersection with horizontal edges ---

    // Bottom edge: y = minY
    if (std::fabs(dy) > eps) {
        double t = (minY - ry) / dy;
        double x_int = rx + t * dx;
        if (x_int >= minX - eps && x_int <= maxX + eps) {
            candidates.push_back({ t, Vec2(x_int, minY) });
        }
    }

    // Top edge: y = maxY
    if (std::fabs(dy) > eps)
    {
        double t = (maxY - ry) / dy;
        double x_int = rx + t * dx;
        if (x_int >= minX - eps && x_int <= maxX + eps)
            candidates.push_back({ t, Vec2(x_int, maxY) });
    }

    // We expect exactly 2 intersection points
    if (candidates.size() < 2)
        return false;

    // Sort candidates by their t-value.
    std::sort(candidates.begin(), candidates.end(), [](
        const IntersectionCandidate& a,
        const IntersectionCandidate& b)
        {
            return a.t < b.t;
        });

    // Remove duplicates: if two candidate intersections have nearly the same t, keep only one.
    std::vector<IntersectionCandidate> unique;
    unique.push_back(candidates.front());
    for (size_t i = 1; i < candidates.size(); ++i)
    {
        if (std::fabs(candidates[i].t - unique.back().t) > eps)
            unique.push_back(candidates[i]);
    }

    if (unique.size() != 2)
    {
        // In degenerate cases (e.g. ray exactly tangent to the rectangle)
        return false;
    }

    // Order the two intersections:
    // The one with the smaller t is the "back" intersection (may be behind the ray’s origin)
    // and the one with the larger t is the "foward" intersection.
    *back_intersect = unique[0].pt;
    *foward_intersect = unique[1].pt;
    return true;
}


/*bool getRayRectIntersection(Vec2* targ, const FRect &r, const Ray &ray)
{
    constexpr double toRad = M_PI / 180.0;
    constexpr double toDeg = 180.0 / M_PI;

    //double ret = null;
    //ray_angle *= 180.0 / M_PI;
    double closest_intersection = std::numeric_limits<double>::max();

    Ray rect_lines[] = {
        { r.x1, r.y1, 0 * toRad} ,
        { r.x2, r.y1, 90 * toRad},
        { r.x2, r.y2, 180 * toRad},
        { r.x1, r.y2, 270 * toRad}
    };

    bool any_intersect = false;
    for (int i = 0; i < 4; i++)
    {
        Ray& side = rect_lines[i];

        switch (i)
        {
        case 0: if (ray.y <= r.y1) continue; break; // Ignore top
        case 1: if (ray.x >= r.x2) continue; break; // Ignore right
        case 2: if (ray.y >= r.y2) continue; break; // Ignore bottom
        case 3: if (ray.x <= r.x1) continue; break; // Ignore left
        }

        Vec2 intersection;
        bool intersects = lineEqIntersect(&intersection, ray, side);

        if (intersects)
        {
            double d = distToPoint(ray.x, ray.y, intersection.x, intersection.y);
            if (d < closest_intersection)
            {
                closest_intersection = d;
                *targ = { intersection.x, intersection.y };
                any_intersect = true;
            }
        }
    }

    return any_intersect;
}
*/