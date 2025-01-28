#pragma once
#include "SpaceEngine.h"

SIM_BEG(BigBang)

using namespace NS_SpaceEngine;
typedef NS_SpaceEngine::Sim SpaceEngine;

struct Sim : public NS_SpaceEngine::Sim
{
    int particle_count;

    void prepare();
    void start();
};

SIM_END