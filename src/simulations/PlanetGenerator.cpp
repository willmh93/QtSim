#include "PlanetGenerator.h"
SIM_DECLARE(PlanetGenerator, "Physics", "Space Engine", "Planet Generator")

double particleRadiusForPlanet(double planet_radius, int subparticle_count)
{
    const double hex_lattice_density = 0.952;
    return (hex_lattice_density * (planet_radius / sqrt((double)subparticle_count)));
}


/// Project ///

void PlanetGenerator_Project::projectAttributes(Input* input)
{
    //options->realtime_slider("Panel Count", &panel_count, 1, 16, 1);
}

void PlanetGenerator_Project::projectPrepare()
{
    auto& layout = newLayout();

    /// Create multiple instance of Scene and add to separate viewports
    create<PlanetGenerator_Scene>(panel_count)->mountTo(layout);

    /// Or create individual instances of Scene and add them to Layout
    //for (int i = 0; i < panel_count; ++i)
    //    layout[i]->mountScene(create<PlanetGenerator_Scene>());

    /// Or create a single Scene instance and view on multiple Viewports
    //auto* scene = create<PlanetGenerator_Scene>();
    //for (int i = 0; i < panel_count; ++i)
    //    layout << scene;
}

/// Scene ///

void PlanetGenerator_Scene::sceneAttributes(Input* input)
{
    //--- Only updated on sceneStart ---//

    //options->starting_checkbox("Starting Flag", &var1);                
    //options->starting_slider("Starting Double", &var3, 0.0, 1.0, 0.1);

    //--- Updated in realtime ---//

    input->realtime_slider("Particle Count", &particle_count, 1, 1000);
    input->realtime_slider("Planet Radius", &planet_radius, 10, 1000.0);
    input->realtime_slider("Push Multiple", &push_mult, 0.1, 10.0);
    input->realtime_slider("Steps", &push_steps, 1, 10000);
    
}

void PlanetGenerator_Scene::sceneStart()
{
    /// Initialize Scene
}

void PlanetGenerator_Scene::sceneDestroy()
{
    /// Destroy Scene
}

void PlanetGenerator_Scene::sceneMounted(Viewport* viewport)
{
    /// Initialize viewport (after sceneStart)
    camera->setOriginViewportAnchor(Anchor::CENTER);
    //camera->focusWorldRect(0, 0, 300, 300);
}

void PlanetGenerator_Scene::sceneProcess()
{
    /// Process Scene update
    particles = randomDistributedPlanet<Particle>(0, 0, planet_radius, particle_count, 1.0, push_mult, push_steps);
}

void PlanetGenerator_Scene::viewportProcess(Viewport* ctx)
{
    /// Process Viewports running this Scene
}

void PlanetGenerator_Scene::viewportDraw(Viewport* ctx)
{
    /// Draw Scene to Viewport
    ctx->drawWorldAxis();

    int len = particles.size();
    for (int i = 0; i < len; i++)
    {
        Particle& n = particles[i];
        ctx->setFillStyle(255, 255, 255);
        ctx->beginPath();
        ctx->circle(n.x, n.y, n.r);
        ctx->fill();
    }
}

/// User Interaction

void PlanetGenerator_Scene::mouseDown() {}
void PlanetGenerator_Scene::mouseUp() {}
void PlanetGenerator_Scene::mouseMove() {}
void PlanetGenerator_Scene::mouseWheel() {}

SIM_END(PlanetGenerator)

