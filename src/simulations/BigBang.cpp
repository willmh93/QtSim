#include "BigBang.h"

SIM_DECLARE(BigBang, "Physics", "Space Engine", "Big Bang")

/*CHILD_SORT("Physics",
    "Space Engine",
    "Experimental"
)

CHILD_SORT("Physics", "Space Engine",
    "Big Bang",
    "Earth Moon"
)*/



void BigBang::projectPrepare()
{
    BigBang::makeScenes(1)->mountTo(newLayout());
}

/*void BigBangScene::projectPrepare()
{

    world_size_min = 100;
    world_size_max = 10000;
    steps_per_frame = 1;
    step_seconds = 0.001;
    particle_count = 50;// 5000;
    collision_substeps = 10;
    gravity_cell_near_ratio = 0.01;
    //start_particle_radius = 0.05;
    gravity = 0.03;

    start_world_size = 5000;

    step_seconds_step = step_seconds * 0.1;
    step_seconds_min = 0;// step_seconds * 0.5;
    step_seconds_max = step_seconds * 10;

    SpaceEngineScene::projectPrepare();

    

}*/

void BigBangScene::sceneStart()
{
    //world_size_min = 100;
    //world_size_max = 10000;
    steps_per_frame = 1;
    step_seconds = 0.001;
    particle_count = 10000;
    collision_substeps = 10;
    gravity_cell_near_ratio = 0.01;
    //start_particle_radius = 0.05;
    gravity = 0.03;

    //start_world_size = 5000;
    world_size = 5000;

    //step_seconds_step = step_seconds * 0.1;
    //step_seconds_min = 0;// step_seconds * 0.5;
    //step_seconds_max = step_seconds * 10;

    SpaceEngineScene::sceneStart();

    auto universe_particles = newPlanetFromParticleCount(0, 0, 50, world_size, particle_count);

    double explode_speed = 1000.0;
    double max_perp_speed_ratio = 1;
    double perp_bias = 1;

    double angle_randomness = ((M_PI * 2) / 360.0) * 45.0;

    for (Particle& p : universe_particles)
    {
        double angle = atan2(p.y, p.x) + random(-angle_randomness, angle_randomness);
        double perp_angle = angle + (M_PI / 2.0);
        double dist_ratio = sqrt(p.x * p.x + p.y * p.y) / 20.0;
        double speed = random(0, explode_speed) * (dist_ratio + 0.1);
        double max_perp_speed = speed;
        double perp_speed = random(-max_perp_speed, max_perp_speed) + (max_perp_speed * perp_bias);
        p.vx = cos(angle) * speed + cos(perp_angle) * perp_speed;
        p.vy = sin(angle) * speed + sin(perp_angle) * perp_speed;
    }

    addParticles(universe_particles);

    /*for (int i = 0; i < particle_count; i++)
    {
        double f = ((double)i) / ((double)particle_count);
        double a = f * M_PI * 2;

        double pos_angle = random(-M_PI, M_PI);
        double pos_radius = sqrt(random(0, 1)) * (world_size / 2);

        Particle n;
        n.x = cos(pos_angle) * pos_radius;
        n.y = sin(pos_angle) * pos_radius;
        n.vx = random(-start_particle_speed, start_particle_speed);
        n.vy = random(-start_particle_speed, start_particle_speed);

        double real_radius = start_particle_radius;// *0.01;// random(start_particle_radius, 10.0 * start_particle_radius);


        // Magnify simulated radius
        n.r = real_radius * particle_magnify;//

        // But use real radius to calculate mass
        n.calculateMass(real_radius, 100);
        //n.mass = 1;

        particles.emplace_back(n);
    }*/

    
}

void BigBangScene::sceneProcess()
{
    SpaceEngineScene::sceneProcess();

    double x1 = std::numeric_limits<double>::max();
    double x2 = std::numeric_limits<double>::lowest();
    double y1 = std::numeric_limits<double>::max();
    double y2 = std::numeric_limits<double>::lowest();

    for (const Particle &p: particles)
    {
        if (p.x < x1) x1 = p.x;
        if (p.x > x2) x2 = p.x;
        if (p.y < y1) y1 = p.y;
        if (p.y > y2) y2 = p.y;
    }

    /*if (!focus_rect_initialized)
    {
        focus_rect_initialized = true;
        focus_rect.set(x1, y1, x2, y2);
    }
    else
    {
        focus_rect = lerpRect(focus_rect, FRect(x1, y1, x2, y2), 0.1);
    }

    ctx->camera.focusWorldRect(focus_rect);*/
}

void BigBangScene::viewportDraw(Viewport* ctx)
{
    SpaceEngineScene::viewportDraw(ctx);
    /*FRect r = ctx->camera.toStageRect(-world_size / 2, -world_size / 2, world_size / 2, world_size / 2);

    ctx->setStrokeStyle(255,255,255);
    ctx->strokeRect(r);*/
}

SIM_END(BigBang)