#ifndef {HEADER_GUARD}
#define {HEADER_GUARD}
#include "Simulation.h"

SIM_BEG({CLASS_NAME})

struct {CLASS_NAME}_Instance : public SimulationInstance
{{
    {CLASS_NAME}_Instance()
    {{
        // Args passed from {CLASS_NAME}.
        // Use initializer list for reference variables
    }}

    void instanceAttributes() override;

    void start();
    void destroy();
    void process(DrawingContext* ctx);
    void draw(DrawingContext* ctx);

    //void mouseDown(MouseInfo mouse);
    //void mouseUp(MouseInfo mouse);
    //void mouseMove(MouseInfo mouse);
    //void mouseWheel(MouseInfo mouse);
}};

struct {CLASS_NAME} : public Simulation<{CLASS_NAME}_Instance>
{{
    int panel_count = 1;

    void projectAttributes() override;
    void prepare();
}};

SIM_END
#endif