#pragma once
#include "SpaceEngine.h"

SIM_BEG(EarthMoon)
BASE_SIM(SpaceEngine)

struct EarthMoon : public SpaceEngine
{
    //const double zettagram_kg = 10.0e+18; // kg in a zettagram

    double solar_system_size = 5000 * gigameter;
    double moon_earth_orbit_radius = 0.4055 * gigameter;

    double moon_radius = 0.0017374 * gigameter;
    double moon_density = 3344;

    double earth_radius = 0.006371 * gigameter;
    double earth_density = 5515; // kg/m^3
    //double earth_mass = (5972 * zettagram_kg);// / 1000.0; // kg

    bool planet_subparticles = false;

    void prepare();
    void start();
};



SIM_END



