#include "AnnoyingRacing.h"
SIM_DECLARE(AnnoyingRacing_Project, "My Projects", "Annoying Racing")

/// Project ///

void AnnoyingRacing_Project::projectAttributes(Options* options)
{
    options->realtime_slider("Panel Count", &panel_count, 1, 16, 1);
}

void AnnoyingRacing_Project::projectPrepare()
{
    auto& layout = newLayout();

    // Create separate instances of our Scene and add them to Layout
    //for (int i = 0; i < panel_count; ++i)
    //    layout << AnnoyingRacing_Project::createScene();

    // Or create a single instance of our Scene and view on multiple Viewports
    auto* scene = AnnoyingRacing_Project::createScene();
    for (int i = 0; i < panel_count; ++i)
        layout << scene;
}

/// Scene ///

void AnnoyingRacing_Scene::sceneAttributes(Options* options)
{
    // Only updated on sceneStart()

    //options->starting_checkbox("Starting Flag", &var1);                
    //options->starting_slider("Starting Double", &var3, 0.0, 1.0, 0.1);

    // Updated in realtime

    //options->realtime_slider("Realtime Double", &var2, 0.0, 1.0, 0.1); 
    
}

void AnnoyingRacing_Scene::sceneStart()
{
    // Initialize Scene


}

void AnnoyingRacing_Scene::sceneDestroy()
{
    // Destroy Scene
}

void AnnoyingRacing_Scene::sceneMounted(Viewport* viewport)
{
    // Initialize viewport (after sceneStart)
    camera->setOriginViewportAnchor(Anchor::CENTER);
    //camera->focusWorldRect(0, 0, 300, 300);


    Car car;
    car.x = 0;
    car.y = viewport->viewportIndex() * 100;

    cars.push_back(car);
}

void AnnoyingRacing_Scene::sceneProcess()
{
    // Process Scene update
}

void AnnoyingRacing_Scene::viewportProcess(Viewport* ctx)
{
    // Process Viewports running this Scene
}

void AnnoyingRacing_Scene::viewportDraw(Viewport* ctx)
{
    // Draw Scene to Viewport
    ctx->drawWorldAxis();

    for (Car &car : cars)
    {
        car.draw(ctx);
    }
}

/// User Interaction

void AnnoyingRacing_Scene::mouseDown() {}
void AnnoyingRacing_Scene::mouseUp() {}
void AnnoyingRacing_Scene::mouseMove() {}
void AnnoyingRacing_Scene::mouseWheel() {}

SIM_END(AnnoyingRacing_Project)