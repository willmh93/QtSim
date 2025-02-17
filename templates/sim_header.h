#pragma once
#include "Project.h"

SIM_BEG({CLASS_NAME}_Project)

struct {CLASS_NAME}_Scene : public Scene
{{
/*
    // Custom Launch Config Example

    struct LaunchConfig
    {{
        double particle_speed = 10.0;
    }};
    
    {CLASS_NAME}_Scene(LaunchConfig& info) : 
        particle_speed(info.particle_speed)
    {{}}
    
    double particle_speed;
*/

    void sceneAttributes(Options* options) override;

    void sceneStart() override;
    //void sceneStop() override;
    void sceneDestroy() override;
    void sceneMounted(Viewport* viewport) override;
    void sceneProcess() override;

    void viewportProcess(Viewport* ctx) override;
    void viewportDraw(Viewport* ctx) override;

    void mouseDown() override;
    void mouseUp() override;
    void mouseMove() override;
    void mouseWheel() override;
}};

struct {CLASS_NAME}_Project : public Project<{CLASS_NAME}_Scene>
{{
    int panel_count = 1;

    void projectAttributes(Options* options) override;
    void projectPrepare() override;
    //void projectStart() override;
    //void projectStop() override;
    //void projectDestroy() override;

}};

SIM_END({CLASS_NAME}_Project)