#include "EarthMoon.h"
#include "PlanetGenerator.h"

SIM_DECLARE(EarthMoon, "Physics", "Space Engine", "Earth Moon")

void EarthMoon_Project::projectPrepare()
{
    auto& layout = newLayout();

    auto scene = create<EarthMoon_Scene>(EarthMoon_Scene::Config(true));

    for (int i=0; i<1; i++)
        layout << scene;
}

void EarthMoon_Scene::sceneAttributes()
{

    steps_per_frame = 100;
    step_seconds = 1;// 1000000000000.0;
    collision_substeps = 1;// 4;

    //timestep = 1 / step_seconds;

    // Starting values
    gravity = G;
    world_size = moon_earth_orbit_radius * 2;// moon_earth_orbit_radius * 3;// solar_system_size;
    
    particle_bounce = 0.27;
    // / 10.0;// *magnify_particles;
    //start_particle_mass = earth_mass / 100.0;

    //double particle_radius_step = particle_radius * 0.1;// particle_magnify;
    //double particle_radius_min = particle_radius * 0.1 * particle_magnify;
    //double particle_radius_max = particle_radius * 10 * particle_magnify;

    //collision_cell_size = start_particle_radius * 10;
    //collision_cell_size_min = collision_cell_size * 0.1;
    //collision_cell_size_max = collision_cell_size * 2;
    //collision_cell_size_step = collision_cell_size * 0.1;

    
    //particle_count = 2;

    // Set ranges
    /*step_seconds_step = step_seconds * 0.1;
    step_seconds_min = 0;// step_seconds * 0.5;
    step_seconds_max = step_seconds * 10;

    //particle_mass_step = 0.001 * start_particle_mass;
    //particle_mass_min = 0.1 * start_particle_mass;
    //particle_mass_max = 10 * start_particle_mass;



    world_size_step = solar_system_size * 0.01;
    world_size_min = solar_system_size * 0.1;
    world_size_max = solar_system_size * 10;*/


    ImGui::Checkbox("Show Moon", &show_moon);
    ImGui::SliderInt("Earth Particles", &earth_particle_count, 10, 2000);
    ImGui::SliderDouble("Bounce Coefficient", &particle_bounce, 0.0, 1.0);
    ImGui::SliderDouble("Particle Speed (m/s^2)", &particle_speed, 0.0, 1e11);
    
    //options->starting_checkbox("Show Moon", &show_moon);
    //options->starting_slider("Particle Radius (gm)", &particle_radius, particle_radius_min, particle_radius_max, particle_radius_step);
    //options->slider("Particle Mass (zg)", &start_particle_mass, &particle_mass_min, &particle_mass_max, &particle_mass_step);

    SpaceEngine_Scene::sceneAttributes();
}


void EarthMoon_Scene::sceneStart()
{
    SpaceEngine_Scene::sceneStart();

    //double distance_to_neptune = 4495.1 * gigemeter;

    double particle_radius = PlanetGenerator::particleRadiusForPlanet(earth_radius, earth_particle_count);
    
    //auto earth = newParticle(0, 0, earth_radius, earth_density);
    //auto earth2 = newParticle(moon_earth_orbit_radius, 0, earth_radius, earth_density);
    //auto moon = newParticle(moon_earth_orbit_radius, 0, moon_radius, moon_density);

    auto earth_particles = PlanetGenerator::planetFromParticleSize<Particle>(0, 0, earth_radius, earth_density, particle_radius);
    auto moon_particles = PlanetGenerator::planetFromParticleSize<Particle>(moon_earth_orbit_radius, 0, moon_radius, moon_density, particle_radius);

    //double earth_mass_error = earth_sum_mass / earth.mass;
    //double moon_mass_error = moon_sum_mass / moon.mass;

    //addParticles(earth);
    //addParticles(earth2);
    //addParticles(moon);

    addParticles(earth_particles);
    
    if (show_moon)
        addParticles(moon_particles);
    //double earth_planet_mass = sumMass(earth_particles);
    //double moon_planet_mass = sumMass(moon_particles);
    /*int pc_2 = particle_count / 2;
    for (int i=0; i<particle_count; i++)
    {
        double f = ((double)i) / ((double)particle_count);
        double a = f * M_PI * 2;

        double pos_angle = random(-M_PI, M_PI);
        double pos_radius = sqrt(random(0,1)) * (world_size / 2);

        Particle n;
        //n.x = random(-world_size / 2, world_size / 2);
        //n.y = random(-world_size / 2, world_size / 2);
        n.x = cos(pos_angle) * pos_radius;
        n.y = sin(pos_angle) * pos_radius;
        n.vx = random(-start_particle_speed, start_particle_speed);
        n.vy = random(-start_particle_speed, start_particle_speed);

        double real_radius = start_particle_radius;// *0.01;// random(start_particle_radius, 10.0 * start_particle_radius);

        // Magnify simulated radius
        n.r = real_radius * magnify_particles;//

        // But use real radius to calculate mass
        n.calculateMass(real_radius, earth_density);

        //n.mass = start_particle_mass;

        //n.angle = 0;// random(-M_PI, M_PI);
        //n.spin = (a - M_PI) * 0.001;// random(-M_PI / 200.0, M_PI / 200.0);
        particles.emplace_back(n);
    }*/

    
}

void EarthMoon_Scene::sceneMounted(Viewport* ctx)
{
    //camera->focusWorldRect(-world_size, -world_size, world_size, world_size);
    camera->focusWorldRect(boundaries(particles).scaled(1.3), false);
    focus_rect.set(boundaries(particles).scaled(1.3));
}

void EarthMoon_Scene::viewportProcess(Viewport* ctx)
{
    focus_rect = lerpRect(focus_rect, boundaries(particles).scaled(1.3), 0.02);
    camera->focusWorldRect(focus_rect, false);
}

void EarthMoon_Scene::viewportDraw(Viewport* ctx)
{
    SpaceEngine_Scene::viewportDraw(ctx);
}

SIM_END(EarthMoon)