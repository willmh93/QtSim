#pragma once
#include "SpaceEngine.h"

SIM_BEG(BigBang)
BASE_SIM(SpaceEngine)

struct BigBang : public SpaceEngine
{
    int particle_count;

    bool focus_rect_initialized = false;
    FRect focus_rect;

    void prepare();
    void start();
    void process();
    void draw(QNanoPainter* p);
};

SIM_END