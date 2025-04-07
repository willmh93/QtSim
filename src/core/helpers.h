#pragma once
#include <QString>

#include <unordered_set>
#include <type_traits>
//#include <array>
#include <tuple>

#include "types.h"

inline Vec2 rotateOffset(double dx, double dy, double rotation)
{
    double _cos = cos(rotation);
    double _sin = sin(rotation);
    return {
        (dx * _cos - dy * _sin),
        (dy * _cos + dx * _sin)
    };
}
inline Vec2 rotateOffset(double dx, double dy, double _cos, double _sin)
{
    return {
        (dx * _cos - dy * _sin),
        (dy * _cos + dx * _sin)
    };
}
inline Vec2 rotateOffset(const Vec2& offset, double rotation)
{
    double _cos = cos(rotation);
    double _sin = sin(rotation);
    return {
        (offset.x * _cos - offset.y * _sin),
        (offset.y * _cos + offset.x * _sin)
    };
}
inline Vec2 rotateOffset(const Vec2& offset, double _cos, double _sin)
{
    return {
        (offset.x * _cos - offset.y * _sin),
        (offset.y * _cos + offset.x * _sin)
    };
}

inline Vec2 reverseRotateOffset(double dx, double dy, double rotation)
{
    double _cos = cos(rotation);
    double _sin = sin(rotation);
    return {
        (dx * _cos + dy * _sin),
        (dy * _cos - dx * _sin)
    };
}
inline Vec2 reverseRotateOffset(double dx, double dy, double _cos, double _sin)
{
    return {
        (dx * _cos + dy * _sin),
        (dy * _cos - dx * _sin)
    };
}
inline Vec2 reverseRotateOffset(const Vec2& offset, double rotation)
{
    double _cos = cos(rotation);
    double _sin = sin(rotation);
    return {
        (offset.x * _cos + offset.y * _sin),
        (offset.y * _cos - offset.x * _sin)
    };
}
inline Vec2 reverseRotateOffset(const Vec2& offset, double _cos, double _sin)
{
    return {
        (offset.x * _cos + offset.y * _sin),
        (offset.y * _cos - offset.x * _sin)
    };
}

struct CanvasObject
{
    union {
        Vec2 pos = { 0, 0 };
        struct { double x, y; };
    };

    union {
        Vec2 size = { 0, 0 };
        struct { double w, h; };
    };

    union {
        Vec2 align = { -1, -1 };
        struct { double align_x, align_y; };
    };

    double rotation = 0.0;
    CoordinateType coordinate_type = CoordinateType::WORLD;
    
    ~CanvasObject() {}

    void setCoordinateType(CoordinateType type)
    {
        coordinate_type = type;
    }

    void setAlign(int ax, int ay)
    {
        align_x = ax;
        align_y = ay;
    }

    void setAlign(const Vec2& _align)
    {
        align = _align;
    }

    void setStageRect(double _x, double _y, double _w, double _h)
    {
        coordinate_type = CoordinateType::STAGE;
        x = _x;
        y = _y;
        w = _w;
        h = _h;
    }

    void setWorldRect(double _x, double _y, double _w, double _h)
    {
        coordinate_type = CoordinateType::STAGE;
        x = _x;
        y = _y;
        w = _w;
        h = _h;
    }

    Vec2 topLeft()
    {
        Vec2 offset = { -(align_x + 1) * 0.5 * w , -(align_y + 1) * 0.5 * h };
        return rotateOffset(offset, rotation) + pos;
    }

    FQuad getQuad()
    {
        Vec2 pivot = { (align_x + 1) * 0.5 * w , (align_y + 1) * 0.5 * h };
        FQuad quad = { { 0.0, 0.0 }, { w, 0.0 }, { w, h }, { 0.0, h } };

        // Precompute cos and sin of rotation
        double _cos = cos(rotation);
        double _sin = sin(rotation);

        // Shift corner by negative pivot & Rotate around (0,0)
        quad.a = rotateOffset(quad.a - pivot, _cos, _sin) + pos;
        quad.b = rotateOffset(quad.b - pivot, _cos, _sin) + pos;
        quad.c = rotateOffset(quad.c - pivot, _cos, _sin) + pos;
        quad.d = rotateOffset(quad.d - pivot, _cos, _sin) + pos;

        return quad;
    }

    double localAlignOffsetX()
    {
        return -(align_x + 1) * 0.5 * w;
    }

    double localAlignOffsetY()
    {
        return -(align_y + 1) * 0.5 * h;
    }
};

int countDecimals(double num);

inline float closestAngleDifference(double angle, double target_angle)
{
    constexpr double two_pi = 2.0 * M_PI;
    double diff = std::fmod((target_angle - angle) + M_PI, two_pi);
    if (diff < 0)
        diff += two_pi;
    return diff - M_PI;
}

inline double wrapRadians(double angle)
{
    angle = std::fmod(angle, 2.0 * M_PI);
    if (angle > M_PI) angle -= 2.0 * M_PI;
    else if (angle <= -M_PI) angle += 2.0 * M_PI;
    return angle;
}

inline double wrapRadians2PI(double angle)
{
    angle = std::fmod(angle, 2.0 * M_PI);
    if (angle < 0.0)
        angle += 2.0 * M_PI;
    return angle;
}

//inline double lerp(double a, double b, double x)
//{
//    return (a + (b - a) * x);
//}

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

inline std::vector<std::pair<size_t, size_t>> splitRanges(size_t totalSize, size_t numParts)
{
    std::vector<std::pair<size_t, size_t>> ranges;

    size_t partSize = totalSize / numParts;
    size_t remainder = totalSize % numParts;

    size_t start = 0;
    for (size_t i = 0; i < numParts; ++i)
    {
        size_t thisPartSize = partSize + (i < remainder ? 1 : 0);
        ranges.emplace_back(start, start + thisPartSize);
        start += thisPartSize;
    }

    return ranges;
}

template <typename Callback>
void forEachPixelPos(int iw, int ih, int thread_count, Callback&& callback)
{
    int pixel_count = iw * ih;
    auto pixel_ranges = splitRanges(pixel_count, thread_count);
    std::vector<QFuture<void>> futures(thread_count);

    for (int ti = 0; ti < thread_count; ++ti)
    {
        auto& pixel_range = pixel_ranges[ti];

        futures[ti] = QtConcurrent::run([&, pixel_range]() {
            for (int i = pixel_range.first; i < pixel_range.second; ++i)
            {
                int x = i % iw;
                int y = i / iw;
                std::forward<Callback>(callback)(x, y);
            }
        });
    }

    for (auto& future : futures)
        future.waitForFinished();
}




template <size_t I, size_t N>
constexpr auto makeBoolsTuple()
{
    // We expand over [0..N-1] and produce a tuple of integral_constant<bool, …>
    return[]<size_t... Bs>(std::index_sequence<Bs...>) {
        return std::tuple{
            std::integral_constant<bool, ((I >> (N - 1 - Bs)) & 1)>{}...
        };
    }(std::make_index_sequence<N>{});
}

// 2) Build the dispatch table by enumerating all bitmask combinations
//    of the booleans, storing one function for each pattern.
template <typename F, size_t... Is>
void dispatchBooleansImpl(const std::array<bool, sizeof...(Is)>& flags,
    F&& f,
    std::index_sequence<Is...>)
{
    // Compute the flat index from the actual runtime flags
    int index = 0;
    const size_t shift = sizeof...(Is) - 1;
    ((index |= (flags[Is] ? 1 : 0) << (shift - Is)), ...);

    using FnPtr = void(*)(F&&);

    static constexpr std::array<FnPtr, 1 << sizeof...(Is)> dispatch_table = [] {
        std::array<FnPtr, 1 << sizeof...(Is)> table{};

        [&] <size_t... Idxs>(std::index_sequence<Idxs...>) {
            ((table[Idxs] = +[](F&& f_inner) constexpr {
                constexpr auto bools = makeBoolsTuple<Idxs, sizeof...(Is)>();
                std::apply([&](auto... Bools) {
                    f_inner(Bools...);
                }, bools);
            }), ...);
        }(std::make_index_sequence<1 << sizeof...(Is)>{});

        return table;
    }();


    // Finally, call the correct entry with the user functor.
    dispatch_table[index](std::forward<F>(f));
}

// Helpers that creates a function lookup table for each boolean combination
// usage:  dispatchBooleans( bools_template(FUNC_NAME, [CAPTURE], FUNC_ARGS), BOOL_ARGS )
// note:   The function call itself is not likely to get inlined
template <typename... Bools, typename F>
void dispatchBooleans(F&& f, Bools... flags)
{
    static_assert((std::is_same_v<Bools, bool> && ...), "All flags must be bool");
    dispatchBooleansImpl({ flags... },
        std::forward<F>(f),
        std::make_index_sequence<sizeof...(Bools)>{});
}

#define boolsTemplate(func, capture, ...) capture<typename... Bools>(Bools... passed) { func<Bools::value...>(__VA_ARGS__); }

/* dispatchBooleans([&]<typename... Bools>(Bools... passed) { func<Bools::value...>(__VA_ARGS__); }, true, false, true, false);
*/
///////////////////////////////


/////////////////////////////////


// Number of entries in the lookup table
constexpr std::size_t TABLE_SIZE = 360;

// Pi constant
constexpr double PI = 3.14159265358979323846;

// Structure to hold a pair of cosine and sine values
struct TrigPair {
    double cosValue;
    double sinValue;
};

// Generate the lookup table at compile time
constexpr std::array<TrigPair, TABLE_SIZE> generateTrigLookupTable() {
    std::array<TrigPair, TABLE_SIZE> table{};
    for (std::size_t i = 0; i < TABLE_SIZE; ++i) {
        double angle = (2.0 * PI * i) / static_cast<double>(TABLE_SIZE);
        table[i] = { std::cos(angle), std::sin(angle) };
    }
    return table;
}

// The global constexpr lookup table, computed at compile time
static auto trigLookupTable = generateTrigLookupTable();

inline double fastCos(double radians) {
    // Scale from radians to table index
    constexpr double factor = static_cast<double>(TABLE_SIZE) / (2.0 * PI);

    double idx = radians * factor;
    idx = std::fmod(idx, static_cast<double>(TABLE_SIZE));
    if (idx < 0.0) {
        idx += static_cast<double>(TABLE_SIZE);
    }

    // Round to nearest integer index
    std::size_t i = static_cast<std::size_t>(idx + 0.5);
    if (i >= TABLE_SIZE) {
        i = 0; // handle wrapping edge case
    }
    return trigLookupTable[i].cosValue;
}

inline double fastSin(double radians) {
    constexpr double factor = static_cast<double>(TABLE_SIZE) / (2.0 * PI);

    double idx = radians * factor;
    idx = std::fmod(idx, static_cast<double>(TABLE_SIZE));
    if (idx < 0.0) {
        idx += static_cast<double>(TABLE_SIZE);
    }

    std::size_t i = static_cast<std::size_t>(idx + 0.5);
    if (i >= TABLE_SIZE) {
        i = 0;
    }
    return trigLookupTable[i].sinValue;
}

struct ScanLineSegment {
    int y;
    int xStart;
    int xEnd;
};

static inline void FillFlatSideTriangle(
    const Vec2& top, 
    const Vec2& leftFlat, 
    const Vec2& rightFlat,
    std::vector<ScanLineSegment>& outSegments)
{
    float invSlope1 = 0.0f;
    float invSlope2 = 0.0f;

    float dy1 = (leftFlat.y - top.y);
    float dy2 = (rightFlat.y - top.y);

    if (std::fabs(dy1) > 1e-5f) {
        invSlope1 = (leftFlat.x - top.x) / dy1;
    }
    if (std::fabs(dy2) > 1e-5f) {
        invSlope2 = (rightFlat.x - top.x) / dy2;
    }

    int yStart = static_cast<int>(std::ceil(std::min(top.y, leftFlat.y)));
    int yEnd = static_cast<int>(std::ceil(std::max(top.y, leftFlat.y)));

    float curx1 = top.x;
    float curx2 = top.x;

    for (int scanY = yStart; scanY < yEnd; ++scanY)
    {
        int leftX = static_cast<int>(std::floor(std::min(curx1, curx2)));
        int rightX = static_cast<int>(std::ceil(std::max(curx1, curx2)));

        if (leftX < rightX) 
        {
            ScanLineSegment seg;
            seg.y = scanY;
            seg.xStart = leftX;
            seg.xEnd = rightX;
            outSegments.push_back(seg);
        }

        curx1 += invSlope1;
        curx2 += invSlope2;
    }
}

static inline void FillTriangleSegments(
    const Vec2& v1, 
    const Vec2& v2, 
    const Vec2& v3,
    std::vector<ScanLineSegment>& outSegments)
{
    Vec2 A = v1, B = v2, C = v3;
    if (A.y > B.y) std::swap(A, B);
    if (B.y > C.y) std::swap(B, C);
    if (A.y > B.y) std::swap(A, B);

    const float EPS = 1e-5f;

    if (std::fabs(A.y - B.y) < EPS)
        FillFlatSideTriangle(C, A, B, outSegments);
    else if (std::fabs(B.y - C.y) < EPS)
        FillFlatSideTriangle(A, B, C, outSegments);

    else
    {
        float alpha = (B.y - A.y) / (C.y - A.y + EPS);
        Vec2 D;
        D.x = A.x + (C.x - A.x) * alpha;
        D.y = B.y;

        FillFlatSideTriangle(A, B, D, outSegments);

        FillFlatSideTriangle(C, B, D, outSegments);
    }
}

static inline std::vector<ScanLineSegment> FillConvexQuadSegments(
    const Vec2& A, const Vec2& B, const Vec2& C, const Vec2& D)
{
    std::vector<ScanLineSegment> segments;
    FillTriangleSegments(A, B, C, segments);
    FillTriangleSegments(A, C, D, segments);
    return segments;
}

static inline std::vector<ScanLineSegment> MergeSegments(
    const std::vector<ScanLineSegment>& inSegments)
{
    std::map<int, std::pair<int, int>> rowToSpan;

    for (const auto& seg : inSegments) {
        int y = seg.y;
        int xStart = seg.xStart;
        int xEnd = seg.xEnd;

        auto it = rowToSpan.find(y);
        if (it == rowToSpan.end()) {
            rowToSpan[y] = { xStart, xEnd };
        }
        else {
            auto& span = it->second;
            if (xStart < span.first)  span.first = xStart;
            if (xEnd > span.second) span.second = xEnd;
        }
    }

    std::vector<ScanLineSegment> out;
    out.reserve(rowToSpan.size());
    for (auto& kv : rowToSpan) {
        ScanLineSegment seg;
        seg.y = kv.first;
        seg.xStart = kv.second.first;
        seg.xEnd = kv.second.second;
        out.push_back(seg);
    }
    return out;
}

static inline std::vector<ScanLineSegment> FillConvexQuadSegmentsMerged(
    const Vec2& A, const Vec2& B, const Vec2& C, const Vec2& D)
{
    std::vector<ScanLineSegment> allSegments;
    allSegments.reserve(1000);
    FillTriangleSegments(A, B, C, allSegments);
    FillTriangleSegments(A, C, D, allSegments);
    std::vector<ScanLineSegment> merged = MergeSegments(allSegments);

    return merged;
}


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
