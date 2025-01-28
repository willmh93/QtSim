#include "CurvedSpace.h"

SIM_DECLARE(CurvedSpace, "Curved Space")

void Sim::prepare()
{
    attachCameraControls(&cam);

    world_size = 100;
    //makeTriangleGrid(0, 0, world_size, world_size, 5);

    a = new CoordinateNode(30, 20);
    b = new CoordinateNode(60, 30);
    c = new CoordinateNode(45, 50);
    a->node_id = "a";
    b->node_id = "b";
    c->node_id = "c";
    coordinate_nodes.push_back(a);
    coordinate_nodes.push_back(b);
    coordinate_nodes.push_back(c);

    s1 = new Side(a, b);
    s2 = new Side(b, c);
    s3 = new Side(a, c);
    //s1->clockwise_side = s2;
    //s2->clockwise_side = s3;
    //s3->clockwise_side = s1;

    sides.push_back(s1);
    sides.push_back(s2);
    sides.push_back(s3);

    //Particle* p1 = new Particle(s1, 0.5);
    //
    //p1->direction = M_PI * 0.3;
    p1 = new Particle(20, 20);
    p1->side_a = s1;
    p1->side_b = s3;
    particles.push_back(p1);
}

void Sim::start()
{
    cam.setZoom(((double)height()) / world_size);
}

void Sim::destroy()
{
}

void Sim::process()
{
    p1->slide_a = mouse_x;
    p1->slide_b = mouse_y;
}

void Sim::draw(QNanoPainter* p)
{
    p->setLineWidth(1);
    p->setStrokeStyle("#ff0000");
    p->beginPath();
    for (Side* s : sides)
    {
        p->moveTo(cam.toStage(s->a->x, s->a->y));
        p->lineTo(cam.toStage(s->b->x, s->b->y));
    }
    p->stroke();

    p->setFillStyle("#00ff00");
    p->beginPath();
    for (CoordinateNode* n : coordinate_nodes)
    {
        Vec2 pos = cam.toStage(n->x, n->y);
        p->circle(pos.x, pos.y, 2);
        p->fillText(n->node_id, pos.x + 5, pos.y + 5);
    }
    p->fill();

    p->setFillStyle("#008fff");
    p->setStrokeStyle("#004fff");
    for (Particle* pt : particles)
    {
        Vec2 world_pos = pt->world_pos();
        Vec2 particle_pos = cam.toStage(world_pos);
        p->beginPath();
        p->circle(particle_pos.x, particle_pos.y, 5);
        p->fill();

        /*double arrow_angle = pt->side->angle() + pt->direction;
        double arrow_length = pt->side->size() * 0.2;

        Vec2 arrow_tip(
             + cos(arrow_angle) * arrow_length,
            pt->worldY() + sin(arrow_angle) * arrow_length
        );

        Vec2 tip_pos = cam.toStage(arrow_tip);


        p->beginPath();
        p->moveTo(particle_pos);
        //p->lineTo(
        //    particle_pos.x + cos(arrow_angle) * arrow_length,
        //    particle_pos.y + sin(arrow_angle) * arrow_length);
        p->lineTo(tip_pos);
        p->stroke();*/
    }

    s1->draw_projected_adjacent(p, cam);
}

void Sim::mouseWheel(int delta)
{
}

SIM_END