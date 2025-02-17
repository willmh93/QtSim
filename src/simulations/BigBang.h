#pragma once
#include "SpaceEngine.h"

SIM_BEG(BigBang)
BASE_SIM(SpaceEngine)

struct BigBangScene : public SpaceEngineScene
{
    int particle_count;

    bool focus_rect_initialized = false;
    FRect focus_rect;

    void sceneStart() override;
    void sceneProcess() override;
    void viewportDraw(Viewport* ctx) override;
};

struct BigBang : public SpaceEngineTemplate<BigBangScene>
{
    void projectPrepare() override;
};

SIM_END(BigBang)