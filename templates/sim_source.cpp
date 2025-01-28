#include "{CLASS_NAME}.h"
SIM_DECLARE({CLASS_NAME}, "{SIM_NAME}")

void Sim::prepare()
{{
    //camera.enable(); // Main scene camera (affects all drawing)
    //attachCameraControls(&cam); // Custom camera (for transforms)
}}

void Sim::start()
{{}}

void Sim::destroy()
{{}}

void Sim::process()
{{}}

void Sim::draw(QNanoPainter* p)
{{}}


void Sim::mouseDown(int x, int y, Qt::MouseButton btn)
{{
    Simulation::mouseDown(x, y, btn);
}}

void Sim::mouseUp(int x, int y, Qt::MouseButton btn)
{{
    Simulation::mouseUp(x, y, btn);
}}

void Sim::mouseMove(int x, int y)
{{
    Simulation::mouseMove(x, y);
}}

void Sim::mouseWheel(int delta)
{{
    Simulation::mouseWheel(delta);
}}

SIM_END
