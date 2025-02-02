#include "CurvedSpace.h"
SIM_DECLARE(CurvedSpace, "Curved Space")

void CurvedSpace::prepare()
{
    //options->checkbox("Manual Transform", &custom_scaling);
    options->slider("Gravity", &gravity, 0.0, 1.0, 0.1);
}

void CurvedSpace::start()
{
    auto& layout = setLayout(2, 2);
    for (Panel* panel : layout)
    {
        panel->create<CurvedSpaceInstance>(gravity);
    }
}

void CurvedSpaceInstance::prepare()
{
    particles = allocDelaunayTriangleMesh<Particle>(0, 0, width, height, 20);
}

void CurvedSpaceInstance::destroy()
{
}

void CurvedSpaceInstance::process(DrawingContext& ctx)
{
    //instances[ctx.panel_index].process(ctx);
    //cam.setCamera(main_cam);
    //active_context->main_cam.x += 1.0;
    camera->rotation += (gravity / 180.0 * M_PI);
}

void CurvedSpaceInstance::draw(DrawingContext& ctx)
{
    srand(0);

    for (const auto& n : particles)
    {
        //bool scale_graphics = (i % 2) == 0;
        bool scale_graphics = (rand() % 2) == 0;

        //ctx.scaleGraphics(scale_graphics);
        //ctx.scaleGraphics(custom_scaling ? scale_graphics : true);

        ctx.beginPath();
        ctx.circle(n->x, n->y, 20);
        ctx.fill();
    }
}


void CurvedSpaceInstance::mouseDown(MouseInfo mouse)
{
    gravitateSpace(mouse.world_x, mouse.world_y, 1000000);
}

SIM_END
