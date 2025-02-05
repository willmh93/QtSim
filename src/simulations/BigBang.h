#pragma once
#include "SpaceEngine.h"

SIM_BEG(BigBang)
BASE_SIM(SpaceEngine)

struct BigBangInstance : public SpaceEngineInstance
{
    int particle_count;

    bool focus_rect_initialized = false;
    FRect focus_rect;

    void prepare();
    void start();
    void process(DrawingContext *ctx);
    void draw(DrawingContext* ctx);
};

struct BigBang : public Simulation<BigBangInstance>
{
    void projectAttributes() {}
    void prepare();
};

SIM_END