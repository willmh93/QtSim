#include "CurvedSpace.h"
#include "Fluid.h"
SIM_DECLARE(CurvedSpace, "Physics", "Experimental", "Space Curvature")


void CurvedSpace::projectAttributes(Options* options)
{
    options->realtime_slider("Panel Count", &panel_count, 1, 36, 1);
    options->realtime_slider("Start Radius Mult", &shared_config->radius_mult, 0.1, 2.0, 0.1); // Never add same pointer twice
}

void CurvedSpace::prepare()
{
    auto &panels = newLayout();

    shared_config->big_arr.resize(10000, 5.0);


    //makeInstance(shared_config)->mountTo(panels[0]);
    //makeInstance(shared_config)->mountTo(panels[1]);

    panels << makeInstance(shared_config);
    panels << makeInstance(shared_config);
    panels << makeInstance(shared_config);
    panels << makeInstance(shared_config);
    panels << makeInstance(shared_config);

    shared_config->radius_mult = 0.3;

    //construct<Instance>(shared_config)->mountTo(panels[0]);
    //construct<Instance>(shared_config)->mountTo(panels[1]);

    //setLayout(panel_count).constructAll<CurvedSpaceInstance>(radius_mult, 10);
    //setLayout(panel_count).

    //setLayout(2);
    //panels[0]->construct<CurvedSpaceInstance>(radius_mult, 10);
    //panels[1]->construct<CurvedSpaceInstance>(radius_mult, 20);
    //panels[2]->construct<CurvedSpaceInstance>(radius_mult, 30);
    //panels[3]->construct<CurvedSpaceInstance>(radius_mult, 40);

    //panels[1]->construct<NS_Fluid::FluidInstance>();
}



void CurvedSpaceInstance::instanceAttributes(Options* options)
{
    options->realtime_slider("Gravity", &gravity, 0.0, 1.0, 0.1);
    options->starting_checkbox("Custom Color", &custom_color);
}


/// Simulation Instance Logic

void CurvedSpaceInstance::start()
{
    //main->options->realtime_slider("Gravity", &gravity, 0.0, 1.0, 0.1);
    //radius = radius_mult * radius;
    particles = allocDelaunayTriangleMesh<Particle>(0, 0, 400, 400, 20);
    //qDebug() << "Instance Constructed: " << panel->panel_index;
}

void CurvedSpaceInstance::destroy()
{
    //qDebug() << "Instance Destroyed: " << panel->panel_index;
}

void CurvedSpaceInstance::processScene()
{
    //instances[ctx.panel_index].process(ctx);
    //cam.setCamera(camera);
    //active_context->camera.x += 1.0;
    camera->rotation += (gravity / 180.0 * M_PI);
}

void CurvedSpaceInstance::draw(Panel* ctx)
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
        ctx->circle(n->x, n->y, radius_mult * radius);
        ctx->fill();
    }
}


void CurvedSpaceInstance::mouseDown(MouseInfo mouse)
{
    gravitateSpace(mouse.world_x, mouse.world_y, 1000000);
}

SIM_END(CurvedSpace)
