#include "{CLASS_NAME}.h"
SIM_DECLARE({CLASS_NAME}, "{SIM_NAME}")

// Shared variables (available to all instances)

void {CLASS_NAME}::prepare()
{{
}}

void {CLASS_NAME}::start()
{{
    auto& layout = setLayout(panel_count);
    for (Panel* panel : layout)
        panel->create<{CLASS_NAME}_Instance>();
}}

void {CLASS_NAME}_Instance::prepare()
{{}}

void {CLASS_NAME}_Instance::destroy()
{{}}

void {CLASS_NAME}_Instance::process(DrawingContext* ctx)
{{}}

void {CLASS_NAME}_Instance::draw(DrawingContext* ctx)
{{}}

/// User Interaction

//void {CLASS_NAME}_Instance::mouseDown(int x, int y, Qt::MouseButton btn) {{}}
//void {CLASS_NAME}_Instance::mouseUp(int x, int y, Qt::MouseButton btn) {{}}
//void {CLASS_NAME}_Instance::mouseMove(int x, int y) {{}}
//void {CLASS_NAME}_Instance::mouseWheel(int delta) {{}}

SIM_END
