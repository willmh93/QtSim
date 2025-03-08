#pragma once
#include <QString>
#include <qmath.h>
#include <unordered_set>
#include <random>
#include "types.h"


double min(double a, double b);
double max(double a, double b);

FRect lerpRect(const FRect& src, const FRect& targ, double factor);

template<typename T>
FRect boundaries(const std::vector<T>& points)
{
    double x1 = std::numeric_limits<double>::max();
    double x2 = std::numeric_limits<double>::lowest();
    double y1 = std::numeric_limits<double>::max();
    double y2 = std::numeric_limits<double>::lowest();

    size_t count = points.size();
    for (size_t i = 0; i < count; ++i)
    {
        const T& p = points[i];
        if (p.x < x1) x1 = p.x;
        if (p.x > x2) x2 = p.x;
        if (p.y < y1) y1 = p.y;
        if (p.y > y2) y2 = p.y;
    }

    return FRect(x1, y1, x2, y2);
}

template<typename T>
std::vector<T> allocDelaunayTriangleMesh(
    double x0,
    double y0,
    double x1,
    double y1,
    double r,
    bool center=true)
{
    double w = x1 - x0;
    double h = y1 - y0;
    double dx = 2.0 * r;        // horizontal spacing
    double dy = sqrt(3.0) * r;  // vertical spacing

    //double max_ix = (w - r) / dx;
    //double max_iy = (h - r) / dy;
    double max_x2 = floor(w / dx) * dx;
    double max_y2 = (r * 2) + floor((h - (r * 2)) / (dy)) * dy;
    double shift_x = (w - max_x2) / 2.0;
    double shift_y = (h - max_y2) / 2.0;

    int maxRow = (h - (r*2)) / dy;// static_cast<int>(std::ceil((r * 2.0) / dx)) + 2;

    std::vector<T> points;

    for (int row = 0; row <= maxRow; ++row)
    {
        double y = r + row * dy + shift_y;

        // Shift in x for every other row (to achieve the "hex" staggering)
        double xOffset = r + ((row % 2 == 0) ? 0.0 : r) + shift_x;
        int maxCol = (w - xOffset - r) / dx;// static_cast<int>(std::ceil((r * 2.0) / dx)) + 2;

        //int maxCol = static_cast<int>(std::ceil((r * 2.0) / dx)) + 2;

        for (int col = 0; col <= maxCol; ++col)
        {
            double x = xOffset + col * dx;

            // Accept this position
            /*T* p = new T(
                x0 + x,
                y0 + y
            );*/

            T p;
            p.x = x0 + x;
            p.y = y0 + y;
            points.push_back(p);
        }
    }

    return points;
}



template <typename VecT>
struct Link {
    VecT* p1;
    VecT* p2;

    Link(VecT* a, VecT* b)
    {
        /*if (a < b)
        {
            p1 = a;
            p2 = b;
        }
        else
        {
            p1 = b;
            p2 = a;
        }*/
        if ((a->x < b->x) || ((a->x == b->x) && (a->y < b->y))) {
            p1 = a;
            p2 = b;
        }
        else {
            p1 = b;
            p2 = a;
        }
    }

    double dist() const
    {
        double dx = p2->x - p1->x;
        double dy = p2->y - p1->y;
        return sqrt(dx * dx + dy * dy);
    }

    bool operator==(const Link<VecT>& other) const
    {
        return (p1 == other.p1 && p2 == other.p2 /* || p2 == other.p1 && p1 == other.p2*/);
    }
};

template <typename VecT>
struct LinkHash
{
    size_t operator()(const Link<VecT>& link) const noexcept
    {
        // Sort the three pointers using branchless min/max logic
        VecT* p1 = link.p1;
        VecT* p2 = link.p2;

        VecT* minP = std::min({ p1, p2 });
        VecT* maxP = std::max({ p1, p2 });

        // Hash the sorted pointers directly without extra function calls
        size_t h1 = reinterpret_cast<size_t>(minP);
        size_t h2 = reinterpret_cast<size_t>(maxP);

        // Murmur-inspired hash mixing for stronger hash distribution
        h1 ^= (h2 + 0x9e3779b97f4a7c15ULL + (h1 << 6) + (h1 >> 2));

        return h1;
    }
};

template <typename VecT>
struct LinkEqual
{
    bool operator()(const Link<VecT>& lhs, const Link<VecT>& rhs) const
    {
        return lhs == rhs;
    }
};

template <typename VecT>
class DelaunayTriangulation
{
public:
    static std::vector<Link<VecT>> getTriangleLinks(const Triangle<VecT>& t) {
        return {
            Link<VecT>(t.a, t.b),
            Link<VecT>(t.b, t.c),
            Link<VecT>(t.c, t.a)
        };
    }


    bool isSharedLink(const Link<VecT>& link,
        const std::unordered_set<Triangle<VecT>, TriangleHash<VecT>, TriangleEqual<VecT>>& triSet)
    {
        int count = 0;
        for (const auto& tri : triSet) {
            auto triLinks = getTriangleLinks(tri);
            for (const auto& l : triLinks) {
                if (l == link) {
                    count++;
                    if (count > 1) return true;
                }
            }
        }
        return false;
    }

    void triangulate(
        const std::vector<VecT*>& points, 
        std::vector<Triangle<VecT>>& out_tris) 
    {
        out_tris.clear();
        if (points.size() < 3)
            return;

        // Compute bounding "super-triangle"
        double minX = std::numeric_limits<double>::max();
        double minY = minX;
        double maxX = std::numeric_limits<double>::lowest();
        double maxY = maxX;

        for (auto* p : points) {
            if (p->x < minX) minX = p->x;
            if (p->x > maxX) maxX = p->x;
            if (p->y < minY) minY = p->y;
            if (p->y > maxY) maxY = p->y;
        }

        double dx = maxX - minX;
        double dy = maxY - minY;
        double deltaMax = std::max(dx, dy) * 2.0;

        // Create three new Vec2 objects for the super-triangle
        // (on the stack here, but that's OK as we only need them within this function)
        VecT st1(minX - deltaMax, minY - deltaMax);
        VecT st2(maxX + deltaMax, minY - deltaMax);
        VecT st3((minX + maxX) / 2.0, maxY + deltaMax);

        //std::vector<Triangle<VecT>> tris_a;
        std::vector<Link<VecT>> polygon;
        //std::vector<Triangle<VecT>> tris_b;

        //std::vector<Triangle<VecT>>* active_tris = &out_tris;
        std::vector<Triangle<VecT>> new_tris;

        std::unordered_set<Triangle<VecT>, TriangleHash<VecT>, TriangleEqual<VecT>> bad_set;

        out_tris.push_back(Triangle<VecT>(&st1, &st2, &st3));

        // Insert each point into the triangulation
        //for (auto* currentPt : points)
        int len = (int)points.size();
        for (int i=0; i< len; i++)
        {
            VecT* currentPt = points[i];
            bool is_last = (i == len - 1);

            //std::vector<Triangle<VecT>> badTriangles;
            bad_set.clear();

            // Find all triangles that have this point inside their circumcircle
            for (auto& tri : out_tris)
            {
                if (tri.isPointInCircumcircle(*currentPt))
                    bad_set.insert(tri);
            }

            // Build a polygon (set of Links) from the "bad" triangles' edges
            polygon.clear();
            for (auto& bt : bad_set) 
            {
                auto btLinks = getTriangleLinks(bt);
                for (auto& link : btLinks) 
                {
                    if (!isSharedLink(link, bad_set))
                        polygon.emplace_back(link);
                }
            }

            new_tris.clear();
            //new_tris.reserve(out_tris.size());

            // If we don't find this triangle in the bad set, add to polygon
            if (is_last)
            {
                for (Triangle<VecT>& tri : out_tris)
                {
                    if (bad_set.find(tri) == bad_set.end() &&
                        !(tri.containsVertex(&st1) || tri.containsVertex(&st2) || tri.containsVertex(&st3)))
                    {
                        new_tris.emplace_back(tri);
                    }
                }
            }
            else
            {
                for (Triangle<VecT>& tri : out_tris)
                {
                    if (bad_set.find(tri) == bad_set.end())
                        new_tris.emplace_back(tri);
                }
            }

            for (auto& link : polygon)
                new_tris.emplace_back(link.p1, link.p2, currentPt);

            // Flip active triangle buffer
            out_tris = std::move(new_tris);
            //std::vector<Triangle<VecT>>* old_active_tris = active_tris;
            //active_tris = new_tris;
            //new_tris = old_active_tris;
        }
        
        //for (Triangle<VecT>& tri : *active_tris)
        //{
        //    if ((tri.containsVertex(&st1) || tri.containsVertex(&st2) || tri.containsVertex(&st3)))
        //    {
        //        int a = 5;
        //    }
        //}
    }

    void extractLinks(
        const std::vector<Triangle<VecT>>& inTris, 
        std::vector<Link<VecT>>& outLinks) 
    {
        outLinks.clear();
        //std::unordered_set<Link<VecT>, LinkHash<VecT>, LinkEqual<VecT>> unique_links;

        for (const auto& tri : inTris)
        {
            // Each triangle has 3 sides (Links)
            std::vector<Link<VecT>> triLinks = getTriangleLinks(tri);
            //for (auto& l : triLinks) 
            //{
            //    if (unique_links.find(l) == unique_links.end())
            //        unique_links.insert(l);
            //}

            // Add each to 'outLinks' if not already present
            for (auto& l : triLinks) {
                // Check for duplicates
                bool found = false;
                for (auto& existing : outLinks) {
                    if (existing == l) {
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    outLinks.push_back(l);
                }
            }
        }

        //outLinks.assign(unique_links.begin(), unique_links.end());
    }
};

//

QString normalizeSeconds(double seconds);

#define PI_FLOAT     3.14159265
#define PIBY2_FLOAT  1.5707963

// |error| < 0.005
inline float atan2_approximation2(double y, double x);

double distToPoint(double cx, double cy, double _x, double _y);

Vec2 rotateVec(double cx, double cy, double angle, double x, double y);

bool lineEqIntersect(Vec2 *targ, const Ray& ray1, const Ray& ray2, bool bidirectional=false);
bool getRayRectIntersection(Vec2* back_intersect, Vec2* foward_intersect, const FRect& r, const Ray& ray);

template<typename T>
std::vector<std::vector<T>> splitVector(const std::vector<T>& objects, size_t numParts)
{
    std::vector<std::vector<T>> parts(numParts);

    size_t partSize = objects.size() / numParts;
    size_t remainder = objects.size() % numParts;

    size_t start = 0;
    for (size_t i = 0; i < numParts; ++i)
    {
        size_t thisPartSize = partSize + (i < remainder ? 1 : 0);  // Spread remainder evenly
        parts[i] = std::vector<T>(objects.begin() + start, objects.begin() + start + thisPartSize);
        start += thisPartSize;
    }

    return parts;
}

std::vector<std::pair<size_t, size_t>> splitRanges(size_t totalSize, size_t numParts);

namespace MovingAverage
{
    class MA
    {
        int ma_count;
        double sum;
        std::vector<double> samples;

    public:
        MA(int ma_count) : ma_count(ma_count)
        {}

        double push(double v)
        {
            sum += v;
            samples.push_back(v);

            if (samples.size() > ma_count)
            {
                sum -= samples[0];
                samples.erase(samples.begin());
            }

            return average();
        }

        double average()
        {
            return (sum / static_cast<double>(samples.size()));
        }
    };
}

//bool getRayRectIntersection(Vec2* targ, const FRect& r, const Ray& ray);
