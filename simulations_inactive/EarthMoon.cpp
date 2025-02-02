#include "EarthMoon.h"

SIM_DECLARE(EarthMoon, "Earth Moon")

void EarthMoon::prepare()
{
    SpaceEngine::prepare();
    //sim_factory.push_back([]() {
    //    return new Simulation();
    //    });

    
    steps_per_frame = 30;
    step_seconds = 10;// 1000000000000.0;
    //timestep = 1 / step_seconds;

    // Starting values
    gravity = G;
    start_world_size = moon_earth_orbit_radius * 2;// moon_earth_orbit_radius * 3;// solar_system_size;
    start_particle_radius = particleRadiusForPlanet(earth_radius, 200);// / 10.0;// *magnify_particles;
    //start_particle_mass = earth_mass / 100.0;
    start_particle_speed = 0;// 29780;

    //collision_cell_size = start_particle_radius * 10;
    //collision_cell_size_min = collision_cell_size * 0.1;
    //collision_cell_size_max = collision_cell_size * 2;
    //collision_cell_size_step = collision_cell_size * 0.1;

    collision_substeps = 5;
    //particle_count = 2;

    // Set ranges
    step_seconds_step = step_seconds * 0.1;
    step_seconds_min = 0;// step_seconds * 0.5;
    step_seconds_max = step_seconds * 10;

    //particle_mass_step = 0.001 * start_particle_mass;
    //particle_mass_min = 0.1 * start_particle_mass;
    //particle_mass_max = 10 * start_particle_mass;

    particle_radius_step = earth_radius * particle_magnify;
    particle_radius_min = earth_radius * 0.5 * particle_magnify;
    particle_radius_max = earth_radius * 2 * particle_magnify;

    world_size_step = solar_system_size * 0.01;
    world_size_min = solar_system_size * 0.1;
    world_size_max = solar_system_size * 10;

    options->slider("Particle Speed (m/s^2)", &start_particle_speed, 0.0, 100000000000.0, 100000.0);
    //options->slider("Gravity", &G, 0.000000000001, 100, 0.001);
    options->slider("Particle Radius (gm)", &start_particle_radius, particle_radius_min, particle_radius_max, particle_radius_step);
    //options->slider("Particle Mass (zg)", &start_particle_mass, &particle_mass_min, &particle_mass_max, &particle_mass_step);
}

void EarthMoon::start()
{
    SpaceEngine::start();

    //double distance_to_neptune = 4495.1 * gigemeter;

    
    //auto earth = newParticle(0, 0, earth_radius, earth_density);
    //auto earth2 = newParticle(moon_earth_orbit_radius, 0, earth_radius, earth_density);
    //auto moon = newParticle(moon_earth_orbit_radius, 0, moon_radius, moon_density);

    auto earth_particles = newPlanetFromParticleSize(0, 0, earth_radius, earth_density, start_particle_radius);
    auto moon_particles = newPlanetFromParticleSize(moon_earth_orbit_radius, 0, moon_radius, moon_density, start_particle_radius);

    //double earth_mass_error = earth_sum_mass / earth.mass;
    //double moon_mass_error = moon_sum_mass / moon.mass;

    //addParticles(earth);
    //addParticles(earth2);
    //addParticles(moon);

    addParticles(earth_particles);
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

SIM_END