#include "CurvedSpace.h"
#include "Fluid.h"
SIM_DECLARE(CurvedSpace, "Physics", "Experimental", "Space Curvature")


void CurvedSpace_Project::projectAttributes()
{
    ImGui::SliderInt("Viewport Count", &viewport_count, 1, 36);
    ImGui::SliderDouble("Start Radius Mult", &shared_config->radius_mult, 0.1, 2.0);
}

void CurvedSpace_Project::projectPrepare()
{
    auto &viewports = newLayout();

    shared_config->big_arr.resize(10000, 5.0);


    //createScene(shared_config)->mountTo(viewports[0]);
    //createScene(shared_config)->mountTo(viewports[1]);

    viewports << create<CurvedSpace_Scene>(shared_config);
    viewports << create<CurvedSpace_Scene>(shared_config);
    viewports << create<CurvedSpace_Scene>(shared_config);
    viewports << create<CurvedSpace_Scene>(shared_config);
    viewports << create<CurvedSpace_Scene>(shared_config);

    shared_config->radius_mult = 0.3;

    //construct<SceneBase>(shared_config)->mountTo(viewports[0]);
    //construct<SceneBase>(shared_config)->mountTo(viewports[1]);

    //setLayout(viewport_count).constructAll<CurvedSpaceScene>(radius_mult, 10);
    //setLayout(viewport_count).

    //setLayout(2);
    //viewports[0]->construct<CurvedSpaceScene>(radius_mult, 10);
    //viewports[1]->construct<CurvedSpaceScene>(radius_mult, 20);
    //viewports[2]->construct<CurvedSpaceScene>(radius_mult, 30);
    //viewports[3]->construct<CurvedSpaceScene>(radius_mult, 40);

    //viewports[1]->construct<NS_Fluid::FluidScene>();
}



void CurvedSpace_Scene::sceneAttributes()
{
    ImGui::SliderDouble("Gravity", &gravity, 0.0, 1.0);
    ImGui::Checkbox("Custom Color", &custom_color);
}


/// Project SceneBase Logic

void CurvedSpace_Scene::sceneStart()
{
    //main->options->realtime_slider("Gravity", &gravity, 0.0, 1.0, 0.1);
    //radius = radius_mult * radius;
    particles = allocDelaunayTriangleMesh<Particle>(0, 0, 400, 400, 20);
    //qDebug() << "SceneBase Constructed: " << viewport->viewport_index;
}

void CurvedSpace_Scene::sceneDestroy()
{
    //qDebug() << "SceneBase Destroyed: " << viewport->viewport_index;
}

void CurvedSpace_Scene::sceneProcess()
{
    //scenes[ctx.viewport_index].process(ctx);
    //cam.setCamera(camera);
    //active_context->camera.x += 1.0;
    camera->rotation += (gravity / 180.0 * M_PI);
}

void CurvedSpace_Scene::viewportDraw(Viewport* ctx)
{
    srand(0);

    if (custom_color)
    {
        ctx->setFillStyle(255, 0, 0);
    }

    for (const auto& n : particles)
    {
        //bool scale_graphics = (i % 2) == 0;
        bool scale_graphics = (rand() % 2) == 0;

        //ctx.scaleGraphics(scale_graphics);
        //ctx.scaleGraphics(custom_scaling ? scale_graphics : true);

        ctx->beginPath();
        ctx->circle(n.x, n.y, radius_mult * radius);
        ctx->fill();
    }
}


void CurvedSpace_Scene::mouseDown(MouseInfo mouse)
{
    gravitateSpace(mouse.world_x, mouse.world_y, 1000000);
}

SIM_END(CurvedSpace)
