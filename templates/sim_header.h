#ifndef {HEADER_GUARD}
#define {HEADER_GUARD}
#include "Simulation.h"

SIM_BEG({CLASS_NAME})

struct {CLASS_NAME}_Instance : public SimulationInstance
{{
    //Camera cam; // Optional camera for custom transforms

    void prepare();
    void destroy();
    void process(DrawingContext* ctx);
    void draw(DrawingContext* ctx);

    //void mouseDown(int x, int y, Qt::MouseButton btn);
    //void mouseUp(int x, int y, Qt::MouseButton btn);
    //void mouseMove(int x, int y);
    //void mouseWheel(int delta);
}};

struct Fluid : public Simulation
{
    int panel_count = 1;

    void prepare();
    void start();
};

SIM_END
#endif