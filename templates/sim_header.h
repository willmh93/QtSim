#ifndef {HEADER_GUARD}
#define {HEADER_GUARD}
#include "Simulation.h"

SIM_BEG({CLASS_NAME})

struct Sim : public Simulation
{{
    //Camera cam; // Optional camera for custom transforms

    void prepare();
    void start();
    void destroy();
    void process();
    void draw(QNanoPainter* p);

    void mouseDown(int x, int y, Qt::MouseButton btn);
    void mouseUp(int x, int y, Qt::MouseButton btn);
    void mouseMove(int x, int y);
    void mouseWheel(int delta);
}};

SIM_END
#endif