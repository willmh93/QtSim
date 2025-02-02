#include "{CLASS_NAME}.h"
SIM_DECLARE({CLASS_NAME}, "{SIM_NAME}")

void {CLASS_NAME}::prepare()
{{
    //main_cam.originToCenterViewport(); // Global origin in center of viewport
    //setFocusedCamera(&cam);            // If using custom camera for manual transforms
}}

void {CLASS_NAME}::start()
{{}}

void {CLASS_NAME}::destroy()
{{}}

void {CLASS_NAME}::process()
{{}}

void {CLASS_NAME}::draw(QNanoPainter* p)
{{}}

/// User Interaction

void {CLASS_NAME}::mouseDown(int x, int y, Qt::MouseButton btn)
{{}}

void {CLASS_NAME}::mouseUp(int x, int y, Qt::MouseButton btn)
{{}}

void {CLASS_NAME}::mouseMove(int x, int y)
{{}}

void {CLASS_NAME}::mouseWheel(int delta)
{{}}

SIM_END
