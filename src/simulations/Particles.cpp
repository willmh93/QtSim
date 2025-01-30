#include "Particles.h"

SIM_BEG(Particles)

void Particles::prepare()
{
    main_cam.enable();

    particle_count = 100;
    gravity = 0;

    options->slider("Particles", &particle_count, 10, 100000);
    options->slider("Gravity", &gravity, -1.0, 1.0, 0.1);
}

void Particles::start()
{
    for (int i = 0; i < particle_count; i++)
    {
        Particle particle;
        particle.x = random(-1000, 1000);
        particle.y = random(-1000, 1000);
        particle.vx = random(-1, 1);
        particle.vy = random(-1, 1);
        particles.push_back(particle);
    }
}

void Particles::destroy()
{
    particles.clear();
}

void Particles::process()
{
    size_t len = particles.size();
    for (size_t i = 0; i < len; i++)
    {
        Particle& particle = particles[i];
        particle.x += particle.vx;
        particle.y += particle.vy;
        particle.vy += gravity;
    }
}

void Particles::draw(QNanoPainter* p)
{
    p->setLineCap(QNanoPainter::CAP_ROUND);
    p->setFillStyle({ 255,255,255 });
    p->setStrokeStyle({ 255,0,0 });

    main_cam.pan_x = mouse_x - (width()/2);
    main_cam.pan_y = mouse_y - (height()/2);
    main_cam.rotation += 0.003;

    size_t len = particles.size();
    for (size_t i = 0; i < len; i++)
    {
        Particle& particle = particles[i];
        //p->setLineWidth(10);
        p->beginPath();
        p->circle(particle.x, particle.y, 1);
        //p->arc(particle.x, particle.y, 50, 0, M_PI * 2);
        p->fill();
        //p->stroke();
    }
}

void Particles::mouseWheel(int delta)
{
    main_cam.zoom_x += ((double)delta) / 1000.0;
    main_cam.zoom_y += ((double)delta) / 1000.0;
}

SIM_END
