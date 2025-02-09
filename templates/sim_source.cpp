#include "{CLASS_NAME}.h"
SIM_DECLARE({CLASS_NAME}, "{SIM_NAME}")

// Shared variables (available to all instances)

void {CLASS_NAME}::projectAttributes()
{{
    options->realtime_slider("Panel Count", &panel_count, 1, 36, 1);
}}

void {CLASS_NAME}::prepare()
{{
    setLayout(panel_count).constructAll<{CLASS_NAME}_Instance>(
        /// Constructor args for all instances
    );

    /// Or alternatively, set up each instance manually
    //auto& panels = setLayout(2);
    //panels[0]->construct<CurvedSpaceInstance>(config_A);
    //panels[1]->construct<CurvedSpaceInstance>(config_B);

}}

/// Instance ///

void {CLASS_NAME}_Instance::instanceAttributes()
{{
    // starting_checkbox,   realtime_checkbox
    // starting_combo,      realtime_combo
    // starting_spin_int    realtime_spin_int
    // starting_spin_double starting_spin_double

    //options->realtime_slider("Instance Var 1", &var1, 0.0, 1.0, 0.1); // updated in realtime
    //options->starting_slider("Instance Var 2", &var2, 0.0, 1.0, 0.1); // only updated on restart
}}

void {CLASS_NAME}_Instance::start()
{{
    // Initialize instance
}}

void {CLASS_NAME}_Instance::destroy()
{{
    // Destroy instance
}}

void {CLASS_NAME}_Instance::process(DrawingContext* ctx)
{{
    // Process update
}}

void {CLASS_NAME}_Instance::draw(DrawingContext* ctx)
{{
    // Draw instance
    ctx->strokeRect(-200, -200, 200, 200);
}}

/// User Interaction

//void {CLASS_NAME}_Instance::mouseDown(MouseInfo mouse) {{}}
//void {CLASS_NAME}_Instance::mouseUp(MouseInfo mouse) {{}}
//void {CLASS_NAME}_Instance::mouseMove(MouseInfo mouse) {{}}
//void {CLASS_NAME}_Instance::mouseWheel(MouseInfo mouse) {{}}

SIM_END
