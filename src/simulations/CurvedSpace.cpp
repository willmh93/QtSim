#include "CurvedSpace.h"
#include "Fluid.h"
SIM_DECLARE(CurvedSpace, "Curved Space")


void CurvedSpace::projectAttributes()
{
    options->realtime_slider("Panel Count", &panel_count, 1, 36, 1);
    options->realtime_slider("Start Radius Mult", &radius_mult, 0.1, 2.0, 0.1); // Never add same pointer twice
}

void CurvedSpace::prepare()
{
    setLayout(panel_count).constructAll<CurvedSpaceInstance>(radius_mult, 10);


    //setLayout(2);
    //panels[0]->construct<CurvedSpaceInstance>(radius_mult, 10);
    //panels[1]->construct<CurvedSpaceInstance>(radius_mult, 20);
    //panels[2]->construct<CurvedSpaceInstance>(radius_mult, 30);
    //panels[3]->construct<CurvedSpaceInstance>(radius_mult, 40);

    //panels[1]->construct<NS_Fluid::FluidInstance>();
}



void CurvedSpaceInstance::instanceAttributes()
{
    options->realtime_slider("Gravity", &gravity, 0.0, 1.0, 0.1);
    options->starting_checkbox("Custom Color", &custom_color);
}


/// Simulation Instance Logic

void CurvedSpaceInstance::start()
{
    //main->options->realtime_slider("Gravity", &gravity, 0.0, 1.0, 0.1);
    radius = radius_mult * radius;
    particles = allocDelaunayTriangleMesh<Particle>(0, 0, width, height, 20);
    //qDebug() << "Instance Constructed: " << panel->panel_index;
}

void CurvedSpaceInstance::destroy()
{
    //qDebug() << "Instance Destroyed: " << panel->panel_index;
}

void CurvedSpaceInstance::process(DrawingContext* ctx)
{
    //instances[ctx.panel_index].process(ctx);
    //cam.setCamera(camera);
    //active_context->camera.x += 1.0;
    camera->rotation += (gravity / 180.0 * M_PI);
}

void CurvedSpaceInstance::draw(DrawingContext* ctx)
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
        ctx->circle(n->x, n->y, radius);
        ctx->fill();
    }
}


void CurvedSpaceInstance::mouseDown(MouseInfo mouse)
{
    gravitateSpace(mouse.world_x, mouse.world_y, 1000000);
}

SIM_END
