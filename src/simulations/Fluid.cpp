#include "Fluid.h"
SIM_DECLARE(Fluid, "Physics", "Fabric")

// Shared variables (available to all scenes)


void Fluid_Project::projectAttributes(Input* options)
{
    // Project settings
    options->realtime_slider("Viewport Count", &viewport_count, 1, 36, 1);
}

void Fluid_Project::projectPrepare()
{
    auto& layout = newLayout();
    //Fluid::makeScenes(viewport_count)->mountTo(layout);
    create<Fluid_Scene>(viewport_count)->mountTo(layout);

    //auto& layout = setLayout(viewport_count);
    //for (Viewport* viewport : layout)
    //{
    //    viewport->construct<FluidScene>();
    //}
}

void Fluid_Scene::sceneAttributes(Input* options)
{
    // SceneBase Settings
    options->realtime_slider("Timestep", &timestep, 0.01, 0.1, 0.01);
    options->realtime_slider("Particle Count", &particle_count, 10, 1000, 10);
    options->realtime_slider("Spring Distance", &spring_dist, 10.0, 100.0, 10.0);
    options->realtime_slider("Spring Stiffness", &spring_stiffness, 0.0, 1.0, 0.01);
    options->realtime_slider("Spring Damping", &spring_damping, 0.0001, 0.1, 0.0001);
    options->realtime_slider("Viscosity Strength", &viscosity_strength, 0.0, 1.0, 0.01);
    options->realtime_slider("Viscosity Spring Dist-Ratio", &viscosity_spring_dist_ratio, 1.0, 100.0, 0.1);
}



void Fluid_Scene::sceneStart()
{
    // Initialize scene
    camera->setOriginViewportAnchor(Anchor::CENTER);

    particles.clear();
    for (int i = 0; i < particle_count; i++)
    {
        Particle* p = new Particle();
        p->x = random(-world_w, world_w);
        p->y = random(-world_h, world_h);
        p->vx = random(-0.3, 0.3);
        p->vy = random(-0.3, 0.3);
        particles.push_back(p);
    }
}

void Fluid_Scene::sceneDestroy()
{
    for (Particle *p : particles)
        delete p;
    particles.clear();
}

void Fluid_Scene::sceneProcess()
{
    delaunay.triangulate(particles, triangles);
    delaunay.extractLinks(triangles, links);

    for (const Link<Particle>& link : links)
    {
        spring(link.p1, link.p2, spring_dist, spring_stiffness, spring_damping, timestep);
    }

    for (const Link<Particle>& link : links)
    {
        double link_dist = link.dist();
        double viscosity_dist = spring_dist * viscosity_spring_dist_ratio;
        applyLinkViscosity(link.p1, link.p2, viscosity_dist, viscosity_strength, timestep);
    }

    //applyViscosity(viscosity_dist, viscosity_strength, timestep);

    for (Particle* n : particles)
    {
        n->x += n->vx;
        n->y += n->vy;
    }
}

void Fluid_Scene::viewportDraw(Viewport* ctx)
{
    ctx->drawWorldAxis();
    ctx->setLineCap(LineCap::CAP_ROUND);

    // Fill triangles
    ctx->setFillStyle(127, 0, 0 );
    for (auto& tri : triangles)
    {
        ctx->beginPath();
        ctx->moveTo(tri.a->x, tri.a->y);
        ctx->lineTo(tri.b->x, tri.b->y);
        ctx->lineTo(tri.c->x, tri.c->y);
        ctx->fill();
    }

    // Fill particles
    ctx->setFillStyle(255, 255, 255);
    int i = 0;
    for (Particle* n : particles)
    {
        camera->worldTransform();
        ctx->beginPath();
        ctx->circle(n->x, n->y, 2);
        ctx->fill();

        QString txt = QString("%1").arg(i);
        camera->labelTransform();
        ctx->save();
        ctx->fillText(txt, Vec2(n->x, n->y) + Offset(10, 10));
        ctx->restore();
        i++;
    }

    camera->worldTransform();

    // Draw links
    ctx->setLineWidth(1);
    ctx->setStrokeStyle(255,255,255);
    ctx->beginPath();
    for (auto& link : links)
    {
        ctx->moveTo(link.p1->x, link.p1->y);
        ctx->lineTo(link.p2->x, link.p2->y);
    }
    ctx->stroke();

    //ctx->restore();

    int ty = 5;
    int row_h = 18;

    ctx->setTextAlign(QNanoPainter::TextAlign::ALIGN_LEFT);
    ctx->setTextBaseline(QNanoPainter::TextBaseline::BASELINE_TOP);
    ctx->setFillStyle("#ffffff");

    camera->stageTransform();
    ctx->fillText(QString("Particles: %1").arg(particles.size()), 5, ty);

    ty += row_h;
    //p->fillText(QString("Frame dt: %1").arg(dt_projectProcess), 5, ty);
}

void Fluid_Scene::applyLinkViscosity(Particle* a, Particle* b, double r, double strength, double dt)
{
    const double radiusSq = r * r;

    // Calculate distance between particles
    double dx = b->x - a->x;
    double dy = b->y - a->y;
    double distSq = dx * dx + dy * dy;

    if (distSq > radiusSq || distSq == 0) return;

    double distance = sqrt(distSq);
    double invDist = 1.0 / distance;

    // Velocity difference between particles
    double dvx = b->vx - a->vx;
    double dvy = b->vy - a->vy;

    // Smoothing kernel (quartic falloff)
    double kernel = 1.0 - (distance / r);
    kernel *= kernel * kernel * kernel;  // Quartic kernel

    // Viscosity impulse calculation
    double impulse = strength * kernel * dt;

    // Apply damping to both particles
    a->vx += dvx * impulse;
    a->vy += dvy * impulse;
    b->vx -= dvx * impulse;
    b->vy -= dvy * impulse;
}

void Fluid_Scene::applyViscosityAll(double r, double strength, double dt)
{
    for (Particle* a : particles)
    {
        for (Particle *b : particles) 
        {
            if (a == b) continue;
            applyLinkViscosity(a, b, r, strength, dt);
        }
    }
}

void Fluid_Scene::spring(Particle* a, Particle* b, double restLength, double k, double damping, double deltaTime)
{
    // Direction vector between particles
    double dx = b->x - a->x;
    double dy = b->y - a->y;
    double distance = sqrt(dx * dx + dy * dy);
    if (distance == 0) return; // Avoid division by zero

    // Normalized direction
    double nx = dx / distance;
    double ny = dy / distance;

    // Relative velocity along the spring direction
    double relativeVel = (b->vx - a->vx) * nx + (b->vy - a->vy) * ny;

    // Hooke's Law: Spring force (F = -k * displacement)
    double springForce = k * (distance - restLength);

    // Damping force (opposes relative motion)
    double dampingForce = -damping * relativeVel;

    // Total force and impulse (force * deltaTime)
    double totalImpulse = (springForce + dampingForce) * deltaTime;

    // Apply impulses to particles
    a->vx += totalImpulse * nx;
    a->vy += totalImpulse * ny;
    b->vx -= totalImpulse * nx;
    b->vy -= totalImpulse * ny;
}

SIM_END(Fluid)
