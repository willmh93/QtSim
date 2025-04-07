#pragma once
#include "Project.h"

SIM_BEG(PlanetGenerator)


template<typename Particle>
std::vector<Particle> planetFromParticleSize(
    double cx,
    double cy,
    double planet_radius,
    double density,
    double particle_radius)
{
    
    double dx = 2.0 * particle_radius;            // horizontal spacing
    double dy = sqrt(3.0) * particle_radius;      // vertical spacing

    double maxAllowedRadius = planet_radius;// -particle_radius;

    int maxRow = static_cast<int>(std::ceil((planet_radius * 2.0) / dx)) + 2;

    std::vector<Particle> planet_particles;

    for (int row = -maxRow; row <= maxRow; ++row)
    {
        double y = row * dy;

        // Shift in x for every other row (to achieve the "hex" staggering)
        double xOffset = (row % 2 == 0) ? 0.0 : particle_radius;

        int maxCol = static_cast<int>(std::ceil((planet_radius * 2.0) / dx)) + 2;

        for (int col = -maxCol; col <= maxCol; ++col)
        {
            double x = xOffset + col * dx;

            // Check if (x, y) is inside the circle of radius maxAllowedRadius
            double dist = std::sqrt(x * x + y * y);
            if (dist <= maxAllowedRadius)
            {
                // Accept this position
                Particle p;
                p.x = cx + x;
                p.y = cy + y;
                p.vx = 0;
                p.vy = 0;
                p.r = particle_radius;
                planet_particles.push_back(p);
            }
        }
    }
    double particle_count = planet_particles.size();

    double planet_volume = (4.0 / 3.0) * M_PI * pow(planet_radius, 3);

    double planet_target_mass = density * planet_volume;
    double particle_mass = planet_target_mass / particle_count;

    for (Particle& p : planet_particles)
    {
        p.mass = particle_mass;
    }

    return planet_particles;
}

double particleRadiusForPlanet(double planet_radius, int subparticle_count);

template<typename Particle>
std::vector<Particle> planetFromParticleCount(
    double cx,
    double cy,
    double planet_radius,
    double density,
    int subparticle_count)
{
    //density *= hex_lattice_density_scalar;

    double planet_volume = (4.0 / 3.0) * M_PI * pow(planet_radius, 3);
    double planet_target_mass = density * planet_volume;

    double particle_radius = particleRadiusForPlanet(planet_radius, subparticle_count);
    return planetFromParticleSize<Particle>(cx, cy, planet_radius, density, particle_radius);
}

template<typename Particle>
std::vector<Particle> randomDistributedPlanet(
    double cx,
    double cy,
    double planet_radius,
    double particle_count,
    double density,
    double push_mult = 1.0,
    int push_steps = 1000)
{
    double planet_volume = (4.0 / 3.0) * M_PI * pow(planet_radius, 3);
    double planet_target_mass = density * planet_volume;

    std::vector<Particle> planet_particles;

    std::uniform_real_distribution<> random(0, 1);
    std::mt19937 gen;

    // Make a completely random planet to start with, including particle overlaps
    for (int i = 0; i < particle_count; i++)
    {
        double position_angle = random(gen) * 2.0 * M_PI;
        double position_dist = sqrt(random(gen)) * planet_radius;

        Particle p;
        p.x = cos(position_angle) * position_dist;
        p.y = sin(position_angle) * position_dist;
        planet_particles.push_back(p);
    }

    // Continuously adjust the closest particles to push them away from eachother
    for (int i = 0; i < push_steps; i++)
    {
        double nearest_dist = std::numeric_limits<double>::max();
        std::pair<int, int> closest_indexes;
        for (int i = 0; i < particle_count; i++)
        {
            for (int j = i + 1; j < particle_count; j++)
            {
                Particle& a = planet_particles[i];
                Particle& b = planet_particles[j];
                double dx = b.x - a.x;
                double dy = b.y - a.y;
                double d = sqrt(dx * dx + dy * dy);
                if (d < nearest_dist)
                {
                    nearest_dist = d;
                    closest_indexes.first = i;
                    closest_indexes.second = j;
                }
            }
        }

        // Push away closest particles
        Particle& a = planet_particles[closest_indexes.first];
        Particle& b = planet_particles[closest_indexes.second];
        double dx = b.x - a.x;
        double dy = b.y - a.y;
        double d = sqrt(dx * dx + dy * dy);
        double nx = dx / d;
        double ny = dy / d;
        a.x -= nx * planet_radius * 0.01 * push_mult;
        a.y -= ny * planet_radius * 0.01 * push_mult;
        b.x += nx * planet_radius * 0.01 * push_mult;
        b.y += ny * planet_radius * 0.01 * push_mult;

        // Ensure we stay inside planet radius
        d = sqrt(a.x * a.x + a.y * a.y);
        if (d > planet_radius)
        {
            double angle = atan2(a.y, a.x);
            a.x = cos(angle) * planet_radius;
            a.y = sin(angle) * planet_radius;
        }

        d = sqrt(b.x * b.x + b.y * b.y);
        if (d > planet_radius)
        {
            double angle = atan2(b.y, b.x);
            b.x = cos(angle) * planet_radius;
            b.y = sin(angle) * planet_radius;
        }
    }

    // Get final closest distance
    double nearest_dist = std::numeric_limits<double>::max();
    for (int i = 0; i < particle_count; i++)
    {
        for (int j = i + 1; j < particle_count; j++)
        {
            Particle& a = planet_particles[i];
            Particle& b = planet_particles[j];
            double dx = b.x - a.x;
            double dy = b.y - a.y;
            double d = sqrt(dx * dx + dy * dy);
            if (d < nearest_dist)
            {
                nearest_dist = d;
            }
        }
    }

    double particle_mass = planet_target_mass / particle_count;
    for (Particle& p : planet_particles)
    {
        p.r = nearest_dist / 2;
        p.vx = 0;
        p.vy = 0;
        p.mass = particle_mass;
    }

    return planet_particles;
}

struct Particle : public MassForceParticle
{

};

struct PlanetGenerator_Scene : public Scene
{
/*  // --- Custom Launch Config Example ---
       
    struct Config
    {
        double speed = 10.0;
    };

    PlanetGenerator_Scene(Config& info) :
        speed(info.speed)
    {}

    double speed;
*/

    /// --- Sim Variables ---
    double planet_radius = 100.0;
    double push_mult = 4;
    int particle_count = 100;
    int push_steps = 1000;
    std::vector<Particle> particles;

    // --- Scene management ---
    void sceneAttributes() override;
    void sceneStart() override;
    //void sceneStop() override;
    void sceneDestroy() override;
    void sceneMounted(Viewport* viewport) override;

    // --- Sim Logic Here ---
    // void processParticles();

    void sceneProcess() override;

    // --- Viewport handling ---
    void viewportProcess(Viewport* ctx) override;
    void viewportDraw(Viewport* ctx) override;

    // --- Input ---
    void mouseDown() override;
    void mouseUp() override;
    void mouseMove() override;
    void mouseWheel() override;
};

struct PlanetGenerator_Project : public Project
{
    int panel_count = 1;

    void projectAttributes() override;
    void projectPrepare() override;
    //void projectStart() override;
    //void projectStop() override;
    //void projectDestroy() override;

};

SIM_END(PlanetGenerator)