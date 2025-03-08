#include "Test.h"
#include "CurvedSpace.h"
#include "SpaceEngine.h"
#include "EarthMoon.h"

//SIM_SORT_KEY(0)
SIM_DECLARE(Test, "Framework Tests", "Canvas Transforms")


///---------///
/// Project ///
///---------///

void Test_Project::projectAttributes(Input* options)
{
    //options->realtime_slider("Viewport Count", &viewport_count, 1, 36, 1);
}

void Test_Project::projectPrepare()
{
    auto& layout = newLayout();

    Test_Scene::Config config1;
    //auto config2 = make_shared<Test_Scene::Config>(Test_Scene::Config());

    create<Test_Scene>(config1)->mountTo(layout);
}

///-----------///
/// SceneBase ///
///-----------///

void Test_Scene::sceneAttributes(Input* options)
{
    // starting_checkbox,   realtime_checkbox
    // starting_combo,      realtime_combo
    // starting_spin_int    realtime_spin_int
    // starting_spin_double starting_spin_double

    //options->realtime_slider("SceneBase Var 1", &var1, 0.0, 1.0, 0.1); // updated in realtime
    //options->starting_slider("SceneBase Var 2", &var2, 0.0, 1.0, 0.1); // only updated on restart
    options->realtime_checkbox("Transform coordinates", &transform_coordinates); // updated in realtime
    options->realtime_checkbox("Scale Lines & Text", &scale_lines_text); // updated in realtime
    options->realtime_checkbox("Rotate Text", &rotate_text); // updated in realtime
    options->realtime_slider("Camera Rotatation", &camera_rotation, 0.0, M_PI*2.0, 0.0001); // updated in realtime
    options->realtime_slider("Camera X", &camera_x, -500.0, 500.0, 1.0); // updated in realtime
    options->realtime_slider("Camera Y", &camera_y, -500.0, 500.0, 1.0); // updated in realtime
  
}

void Test_Scene::sceneStart()
{
    // Initialize scene
    for (int i = 0; i < 50; i++)
    {
        particles.push_back({
            random(-200, 200),
            random(-200, 200),
            random(-2, 2),
            random(-2, 2)
        });
    }
}

void Test_Scene::sceneMounted(Viewport* viewport)
{
    // Initialize viewport
    //if (viewport->viewportIndex() == 0)
    //    //camera->setOriginViewportAnchor(Anchor::CENTER);
    //    camera->setOriginViewportAnchor(Anchor::TOP_LEFT);
    //else
        camera->setOriginViewportAnchor(Anchor::CENTER);

    //camera->focusWorldRect(0, 0, 300, 300);
}

void Test_Scene::sceneDestroy()
{
    // Destroy scene
}

void Test_Scene::sceneProcess()
{
    // Process scene update
    seed += 0.015;

    for (Particle& p : particles)
    {
        p.x += p.vx;
        p.y += p.vy;
        p.vx -= p.x * 0.0001;
        p.vy -= p.y * 0.0001;
    }
}

void Test_Scene::viewportProcess(Viewport* ctx)
{
    //if (ctx->viewportIndex() == 1)
    //{
    //    // Process viewport update
    //    camera->rotation = sin(seed) * M_PI_2;
    //    camera->setZoom(1.0 + sin(seed + M_PI_2) * 0.5);
    //    camera->x = cos(seed + M_PI * 0.75) * 300;
    //    camera->y = sin(seed + M_PI * 0.75) * 150;
    //
    //    //camera->rotation += 0.1 * M_PI / 180.0;
    //    camera->transform_coordinates = transform_coordinates;
    //    camera->scale_lines_text = scale_lines_text;
    //    camera->rotate_text = rotate_text;
    //}
    //else
    {
        camera->x = camera_x;
        camera->y = camera_y;
        camera->rotation = camera_rotation;
    }

    //ball_pos.x = mouse->world_x;
    //ball_pos.y = mouse->world_y;
}

void Test_Scene::viewportDraw(Viewport* ctx)
{
    //camera->x += 1;

    
    

    //ctx->beginPath();
    //ctx->circle(0, 0, 5);
    //ctx->fill();

    


    

    // Draw scene
    //ctx->scaleGraphics(scale_graphics);
    ctx->drawWorldAxis();
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

    ctx->setFillStyle(255, 0, 255);
    ctx->beginPath();
    for (Particle& p : particles)
    {
        ctx->circle(p.x, p.y, 0.1);
    }
    ctx->fill();

    ctx->setFillStyle(255, 255, 255);
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
    ctx->fillText("Fixed pixel offset", p);

    ctx->setStrokeStyle(0, 255, 0);
    ctx->beginPath();
    ctx->moveTo(100, 100);
    ctx->lineTo(p);
    ctx->stroke();
    //camera->setStageOffset(0, 0);
    //ctx->endTransform();

    ctx->setTextAlign(TextAlign::ALIGN_LEFT);
    ctx->setTextBaseline(TextBaseline::BASELINE_TOP);

    camera->setTransformFilters(
        true,
        false,
        false
    );

    // Camera
    ctx->setFillStyle(255, 0, 0);
    ctx->beginPath();
    ctx->circle(camera->x, camera->y, 5);
    ctx->fill();
    ctx->fillText("Camera", Vec2(camera->x, camera->y) + Offset(20, 20));

    ctx->setFillStyle(0, 255, 255);
    ctx->beginPath();
    ctx->circle(ball_pos, 10);
    ctx->fill();
}

/// User Interaction

void Test_Scene::mouseDown() 
{
    //ball_pos.x = mouse.world_x;
    //ball_pos.y = mouse.world_y;
}
void Test_Scene::mouseUp()
{
    //ball_pos.x = mouse.world_x;
    //ball_pos.y = mouse.world_y;
}
void Test_Scene::mouseMove()
{
    ball_pos.x = mouse->world_x;
    ball_pos.y = mouse->world_y;
}
void Test_Scene::mouseWheel() {}

SIM_END(Test)
