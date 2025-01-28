#pragma once

// Instructions:
// > To add a new simulation, include it below and define it in:
//   simulations.txt

#include "Particles.h"

#include "SpaceEngine.h"
    #include "EarthMoon.h"
    #include "BigBang.h"

#include "CurvedSpace.h"

///////////////////////////////////////////////////

/*#define BASE_SIM(id, ns, name)
#define SIM(id, ns, name) {SimulationType::##id, name},
const std::unordered_map<SimulationType, QString> simulationNames =
{
    #include "simulations.txt"
};
#undef SIM
#undef BASE_SIM

//std::vector<std::function<Simulation* (void)>> sim_factory;

Simulation* SimulationFactory(SimulationType type)
{
    Simulation* sim;

    #define BASE_SIM(id, ns, name)
    #define SIM(id, ns, name) case SimulationType::##id: sim = new NS_##ns::Sim(); break;
    switch (type)
    {
    #include "simulations.txt"
    default:
        return nullptr;
    }
    #undef SIM
    #undef BASE_SIM

    if (sim)
        sim->setName(simulationNames.find(type)->second);

    return sim;
}*/