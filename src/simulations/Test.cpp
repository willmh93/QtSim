#include "Test.h"
SIM_DECLARE(Test, "Sim Test")

///---------///
/// Project ///
///---------///

void Test::projectAttributes()
{
    options->realtime_slider("Panel Count", &panel_count, 1, 36, 1);
}

void Test::prepare()
{
    setLayout(panel_count).constructAll<Test_Instance>(
        /// Constructor args for all instances
    );

    /// Or alternatively, set up each instance manually
    //auto& panels = setLayout(2);
    //panels[0]->construct<CurvedSpaceInstance>(config_A);
    //panels[1]->construct<CurvedSpaceInstance>(config_B);

}

///----------///
/// Instance ///
///----------///

void Test_Instance::instanceAttributes()
{
    // starting_checkbox,   realtime_checkbox
    // starting_combo,      realtime_combo
    // starting_spin_int    realtime_spin_int
    // starting_spin_double starting_spin_double

    //options->realtime_slider("Instance Var 1", &var1, 0.0, 1.0, 0.1); // updated in realtime
    //options->starting_slider("Instance Var 2", &var2, 0.0, 1.0, 0.1); // only updated on restart
    options->realtime_checkbox("Transform coordinates", &transform_coordinates); // updated in realtime
    options->realtime_checkbox("Scale Lines & Text", &scale_lines_text); // updated in realtime
    options->realtime_checkbox("Rotate Text", &rotate_text); // updated in realtime
    options->realtime_slider("Camera Rotatation", &camera->rotation, 0, M_PI*2.0, 0.0001); // updated in realtime
    options->realtime_slider("Camera X", &camera->x, -500, 500, 1); // updated in realtime
    options->realtime_slider("Camera Y", &camera->y, -500, 500, 1); // updated in realtime
}

void Test_Instance::start()
{
    // Initialize instance
    panel->setOriginViewportAnchor(Anchor::CENTER);
}

void Test_Instance::destroy()
{
    // Destroy instance
}

void Test_Instance::process(DrawingContext* ctx)
{
    // Process update
}

void Test_Instance::draw(DrawingContext* ctx)
{
    //camera->x += 1;



    //camera->rotation += 0.1 * M_PI / 180.0;
    camera->transform_coordinates = transform_coordinates;
    camera->scale_lines_text = scale_lines_text;
    camera->rotate_text = rotate_text;

    ctx->beginPath();
    ctx->circle(0, 0, 5);
    ctx->fill();

    ctx->setFillStyle(255, 0, 0);
    ctx->beginPath();
    ctx->circle(camera->x, camera->y, 5);
    ctx->fill();

    ctx->setFillStyle(255,255,255);

    // Draw instance
    //ctx->scaleGraphics(scale_graphics);
    ctx->drawGraphGrid();
    /*ctx->setLineWidth(10);
    ctx->beginPath();
    ctx->moveTo(0, 0);
    ctx->lineTo(100, 0);
    ctx->lineTo(100, 100);
    ctx->stroke();
    ctx->strokeRect(0, 0, 200, 200);

    ctx->fillText("Hello!", 20, 20);

    ctx->beginStageTransform();
    ctx->fillText("HUD Text", 20, 20);
    ctx->endTransform();

    camera->worldTransform();*/

    //ctx->beginWorldTransform();

    camera->setTransformFilters(
        transform_coordinates,
        scale_lines_text,
        rotate_text
    );

    ctx->strokeRect(-100, -100, 200, 200);
    //ctx->beginPath();
    //ctx->circle(100, 100, 5);
    //ctx->fill();

    //camera->labelTransform();
    //camera->setStageOffset(50, 0);
    ctx->beginPath();
    Vec2 p = Vec2(100, 100) + Offset(50,50);
    ctx->circle(p, 2);
    ctx->fill();

    ctx->setTextAlign(TextAlign::ALIGN_LEFT);
    ctx->setTextBaseline(TextBaseline::BASELINE_TOP);
    ctx->fillText("In-place with pixel offset", p);

    ctx->setStrokeStyle(0, 255, 0);
    ctx->beginPath();
    ctx->moveTo(100, 100);
    ctx->lineTo(p);
    ctx->stroke();
    //camera->setStageOffset(0, 0);
    //ctx->endTransform();
}

/// User Interaction

//void Test_Instance::mouseDown(MouseInfo mouse) {}
//void Test_Instance::mouseUp(MouseInfo mouse) {}
//void Test_Instance::mouseMove(MouseInfo mouse) {}
//void Test_Instance::mouseWheel(MouseInfo mouse) {}

SIM_END
