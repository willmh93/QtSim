#pragma once
#include "SpaceEngine.h"

SIM_BEG(BigBang)
BASE_SIM(SpaceEngine)

struct BigBangInstance : public SpaceEngineInstance
{
    int particle_count;

    bool focus_rect_initialized = false;
    FRect focus_rect;

    void start() override;
    void processScene() override;
    void draw(Panel* ctx) override;
};

struct BigBang : public SpaceEngine
{
    void prepare() override;
};

SIM_END