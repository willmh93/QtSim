#pragma once
#include "Simulation.h"

SIM_BEG(CurvedSpace)

// Structure to represent a 2D vector or point
struct Vector2D {
    double x;
    double y;

    Vector2D(double x_ = 0, double y_ = 0) : x(x_), y(y_) {}

    // Overload the multiplication operator for scalar multiplication
    Vector2D operator*(double scalar) const {
        return Vector2D(x * scalar, y * scalar);
    }

    // Overload the addition operator for vector addition
    Vector2D operator+(const Vector2D& other) const {
        return Vector2D(x + other.x, y + other.y);
    }
};

class CoordinateSystem {
public:
    Vector2D origin; // Origin point A
    Vector2D axis1;  // First axis (A->B)
    Vector2D axis2;  // Second axis (A->C)

    // Constructor to initialize the coordinate system
    CoordinateSystem(const Vector2D& origin_, const Vector2D& axis1_, const Vector2D& axis2_)
        : origin(origin_), axis1(axis1_), axis2(axis2_) {}

    // Method to get the Cartesian coordinates of a point given its (x, y) in the custom system
    Vector2D getPoint(double x, double y) const {
        return origin + (axis1 * x) + (axis2 * y);
    }
};

struct CoordinateNode : public Vec2
{
    QString node_id;

    CoordinateNode(double _x, double _y)
        : Vec2(_x, _y)
    {
    }
};

struct Side
{
    CoordinateNode* a;
    CoordinateNode* b;
    CoordinateNode* cw; // (Positive Side, Visually Clockwise)
    CoordinateNode* ccw; // (Negative Side, Visually Counter-Clockwise)
    //Side* clockwise_side;

    Side(
        CoordinateNode* _a,
        CoordinateNode* _b,
        CoordinateNode* _cw,
        CoordinateNode* _ccw)
    {
        a = _a;
        b = _b;
        cw = _cw;
        ccw = _ccw;
    }

    double size()
    {
        double dx = b->x - a->x;
        double dy = b->y - a->y;
        return sqrt(dx * dx + dy * dy);
    }

    double angle()
    {
        return atan2(
            b->y - a->y, 
            b->x - a->x
        );
    }

    //Side* clockwiseSide()
    //{
    //    return nullptr;
    //}

    // Equalized triangle side adjacent
    Vec2 projected_normal()
    {
        double cx = (a->x + b->x) * 0.5;
        double cy = (a->y + b->y) * 0.5;
        double rot = angle() + M_PI / 2.0;
        double d = sqrt(3) * (size() / 2.0);
        return Vec2(
            cx + cos(rot) * d,
            cy + sin(rot) * d
        );
    }

    void draw_projected_adjacent(QNanoPainter* p, Camera &cam)
    {
        Vec2 pa = projected_normal();
        Vec2 a_pos = cam.toStage(a->x, a->y);
        Vec2 b_pos = cam.toStage(b->x, b->y);
        Vec2 pa_pos = cam.toStage(pa.x, pa.y);

        p->beginPath();
        p->moveTo(a_pos);
        p->lineTo(pa_pos);
        p->lineTo(b_pos);
        p->stroke();
    }
};

struct Particle
{
    Side* side_a;
    Side* side_b;
    //double position;
    //double direction; // (0 -> PI) relative to (a -> b)

    double slide_a;
    double slide_b;

    Particle(double slideA, double slideB)
    {
        //side = s;
        //position = p;
        slide_a = slideA;
        slide_b = slideB;
        //weight_b = wB;
        //weight_c = wC;
    }

    Vec2 world_pos()
    {
        Vec2 vec_a = Vec2(
            side_a->b->x - side_a->a->x,
            side_a->b->y - side_a->a->y
        );

        Vec2 vec_b = Vec2(
            side_b->b->x - side_b->a->x,
            side_b->b->y - side_b->a->y
        );

        /*double denominator = (vec_a.x * vec_b.x) - (vec_a.y * vec_b.x);
        double x_val = (slide_a * vec_b.y - slide_b * vec_b.x) / denominator;
        double y_val = (vec_a.x * slide_b - vec_a.y * slide_a) / denominator;*/

        double a_len = side_a->size();
        double b_len = side_b->size();

        CoordinateSystem coordSys(
            Vector2D(0,0),
            Vector2D(vec_a.x/a_len, vec_a.y/a_len),
            Vector2D(vec_b.x/b_len, vec_b.y/b_len));
        
        Vector2D p = coordSys.getPoint(slide_a, slide_b);

        return Vec2(
            side_a->a->x + p.x * a_len, 
            side_a->a->y + p.y * b_len
        );

        /*double a_angle = side_a->angle();
        double b_angle = side_b->angle();
        double a_dist = side_a->angle();
        double b_dist = side_b->angle();

        return Vec2(
            side_a->a->x + cos(a_angle) * a_dist,
            side_a->a->y + sin(b_angle) * a_dist
        )*/
    }

    /*double worldX()
    {
        CoordinateNode* a = side->a;
        CoordinateNode* b = side->b;
        return (a->x + (b->x - a->x) * position);
    }

    double worldY()
    {
        CoordinateNode* a = side->a;
        CoordinateNode* b = side->b;
        return (a->y + (b->y - a->y) * position);
    }*/

    /*void getNextPos(Side* next_side, double next_position)
    {
        Vec2 normal_normal = side->projected_normal();
        
        // Always side bc or ca
        Side* bc = side->clockwise_side;
        Side* ca = bc->clockwise_side;

    }*/
};

struct CurvedSpace : public Simulation
{
    Camera cam;

    std::vector<CoordinateNode*> coordinate_nodes;
    std::vector<Side*> sides;
    std::vector<Particle*> particles;

    CoordinateNode* a;
    CoordinateNode* b;
    CoordinateNode* c;
    CoordinateNode* d;
    Side* s1;
    Side* s2;
    Side* s3;
    Side* s4;
    Side* s5;
    Particle* p1;

    double world_size;

    void prepare();
    void start();
    void destroy();

    void process();
    void draw(QNanoPainter* p);

    std::vector<CoordinateNode*> createDelaunayMesh(
        double x0,
        double y0,
        double x1,
        double y1,
        double r)
    {
        double dx = 2.0 * r;        // horizontal spacing
        double dy = sqrt(3.0) * r;  // vertical spacing

        int maxRow = static_cast<int>(std::ceil((r * 2.0) / dx)) + 2;

        for (int row = -maxRow; row <= maxRow; ++row)
        {
            double y = row * dy;

            // Shift in x for every other row (to achieve the "hex" staggering)
            double xOffset = (row % 2 == 0) ? 0.0 : r;

            int maxCol = static_cast<int>(std::ceil((r * 2.0) / dx)) + 2;

            for (int col = -maxCol; col <= maxCol; ++col)
            {
                double x = xOffset + col * dx;

                // Accept this position
                CoordinateNode *p = new CoordinateNode(
                    x0 + x,
                    y0 + y
                );

                coordinate_nodes.push_back(p);
            }
        }

        return coordinate_nodes;
    }

    void mouseWheel(int delta);
};

SIM_END