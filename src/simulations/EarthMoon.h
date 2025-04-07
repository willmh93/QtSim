#pragma once
#include "SpaceEngine.h"

SIM_BEG(EarthMoon)

using namespace SpaceEngine;

struct EarthMoon_Scene : public SpaceEngine_Scene
{
    struct Config 
    {
        bool show_moon;
        Config(bool show_moon = false) :
            show_moon(show_moon)
        {}
    };

    EarthMoon_Scene(Config &config) :
        show_moon(config.show_moon)
    {}

    //const double zettagram_kg = 10.0e+18; // kg in a zettagram

    double solar_system_size = 5000 * gigameter;
    double moon_earth_orbit_radius = 0.4055 * gigameter;

    double moon_radius = 0.0017374 * gigameter;
    double moon_density = 3344;

    double earth_radius = 0.006371 * gigameter;
    double earth_density = 5515; // kg/m^3
    //double earth_mass = (5972 * zettagram_kg);// / 1000.0; // kg

    //double particle_radius;

    int earth_particle_count = 100;
    double particle_speed = 0;

    bool show_moon = false;

    FRect focus_rect;

    bool planet_subparticles = false;

    void sceneAttributes();
    void sceneStart() override;
    void sceneMounted(Viewport* ctx);
    void viewportProcess(Viewport* ctx);
    void viewportDraw(Viewport* ctx) override;
};

struct EarthMoon_Project : public SpaceEngine_Project
{
    void projectPrepare() override;
};

SIM_END(EarthMoon)

using EarthMoon::EarthMoon_Scene;

