#include "Fluid.h"
SIM_DECLARE(Fluid, "Fluid")

void Fluid::prepare()
{
    main_cam.enable(); // Main scene camera (affects all drawing)
    //attachCameraControls(&cam); // Custom camera (for transforms)

    options->slider("Timestep", &timestep, 0.01, 0.1, 0.01);
    options->slider("Particle Count", &particle_count, 10, 1000, 10);

    options->slider("Spring Distance", &spring_dist, 10.0, 100.0, 10.0);
    options->slider("Spring Stiffness", &spring_stiffness, 0.0, 1.0, 0.01);
    options->slider("Spring Damping", &spring_damping, 0.0001, 0.1, 0.0001);

    options->slider("Viscosity Strength", &viscosity_strength, 0.0, 1.0, 0.01);
    options->slider("Viscosity Distance", &viscosity_dist, 10, 1000.0, 10);
}

void Fluid::start()
{
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

void Fluid::destroy()
{}

void Fluid::process()
{
    delaunay.triangulate(particles, triangles);
    delaunay.extractLinks(triangles, links);

    for (const Link<Particle>& link : links)
    {
        spring(link.p1, link.p2, spring_dist, spring_stiffness, spring_damping, timestep);
    }

    applyViscosity(viscosity_dist, viscosity_strength, timestep);

    for (Particle* n : particles)
    {
        n->x += n->vx;
        n->y += n->vy;
    }
}

void Fluid::draw(QNanoPainter* p)
{
    p->setFillStyle({255, 255, 255});
    for (Particle *n : particles)
    {
        p->beginPath();
        p->circle(n->x, n->y, 2);
        p->fill();
    }

    
    p->setLineWidth(1);
    p->setStrokeStyle({ 255,255,255 });
    p->beginPath();
    for (auto& tri : triangles)
    {
        p->moveTo(tri.a->x, tri.a->y);
        p->lineTo(tri.b->x, tri.b->y);
        p->lineTo(tri.c->x, tri.c->y);
    }
    p->stroke();
}

void Fluid::applyViscosity(double r, double strength, double dt)
{
    const double radiusSq = r * r;

    for (Particle* a : particles)
    {
        for (Particle *b : particles) 
        {
            if (a == b) continue;  // Skip self-comparison

            // Calculate distance between particles
            double dx = b->x - a->x;
            double dy = b->y - a->y;
            double distSq = dx * dx + dy * dy;

            if (distSq > radiusSq || distSq == 0) continue;

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
    }
}

void Fluid::spring(Particle* a, Particle* b, double restLength, double k, double damping, double deltaTime)
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


void Fluid::mouseDown(int x, int y, Qt::MouseButton btn)
{
    Simulation::mouseDown(x, y, btn);
}

void Fluid::mouseUp(int x, int y, Qt::MouseButton btn)
{
    Simulation::mouseUp(x, y, btn);
}

void Fluid::mouseMove(int x, int y)
{
    Simulation::mouseMove(x, y);
}

void Fluid::mouseWheel(int delta)
{
    Simulation::mouseWheel(delta);
}

SIM_END
