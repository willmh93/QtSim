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

    map_img.load(":/resources/race_hit_map.png");

    map = new QNanoImage(":/resources/race_map.png");
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
    car.present_state.x = 920;
    car.present_state.y = 500 + (viewport->viewportIndex() * 20);
    car.present_state.angle = -20 * M_PI / 180.0;

    cars.push_back(car);
}

void AnnoyingRacing_Scene::sceneProcess()
{
    // Process Scene update
}

void AnnoyingRacing_Scene::viewportProcess(Viewport* ctx)
{
    // Process Viewports running this Scene
    int i = ctx->viewportIndex();

    Car& car = cars[i];
    car.process(car.present_state, ctx, map_img, 1);

    camera->x = car.present_state.x;
    camera->y = car.present_state.y;

    //if (ctx->viewportIndex() == 0)
    //{
    //    // Human player
    //}
    //else
    //{
        // AI player
        

        //car.reset_ghost_to_present();
        //for (int i = 0; i < 500; i++)
        //{
        //    car.process_future(ctx, map_img);
        //}

    car.aiProcess(car.present_state, map_img);
    //}
}

void AnnoyingRacing_Scene::viewportDraw(Viewport* ctx)
{
    // Draw Scene to Viewport

    ctx->painter->drawImage(*map, 0, 0);
    ctx->drawWorldAxis();

    for (Car &car : cars)
    {
        car.draw(car.present_state, ctx, 255);

        //~ // Add bias

        car.reset_ghost_to_present();
        //car.ghost_state.turn_bias = -turn_speed / 10.0;

        car.ghost_state.sum_turned = 0;
        for (int i = 0; i < 100; i++)
        {
            car.process(car.ghost_state, ctx, map_img, 2);
            car.aiProcess(car.ghost_state, map_img);

            if (i % 20 == 0)
                car.draw(car.ghost_state, ctx, 80);
        }

        /*car.reset_ghost_to_present();
        car.ghost_state.turn_bias = turn_speed / 10.0;

        for (int i = 0; i < 100; i++)
        {
            car.process(car.ghost_state, ctx, map_img, 2);
            car.aiProcess(car.ghost_state, map_img);

            if (i % 20 == 0)
                car.draw(car.ghost_state, ctx, 80);
        }*/


        for (auto& ray : car.rays)
        {
            double angle = ray.angle * M_PI / 180.0;
            double d = ray.dist;

            ctx->beginPath();

            if (ray.longest)
                ctx->setStrokeStyle(0, 255, 0);
            else
                ctx->setStrokeStyle(255, 0, 0);

            ctx->moveTo(car.present_state.x, car.present_state.y);
            ctx->lineTo(
                car.present_state.x + cos(car.present_state.angle + angle) * d,
                car.present_state.y + sin(car.present_state.angle + angle) * d
            );
            ctx->stroke();
        }
    }

    
}

/// User Interaction

void AnnoyingRacing_Scene::mouseDown() {}
void AnnoyingRacing_Scene::mouseUp() {}
void AnnoyingRacing_Scene::mouseMove() {}
void AnnoyingRacing_Scene::mouseWheel() {}

void AnnoyingRacing_Scene::keyPressed(QKeyEvent* e)
{
    switch (e->key())
    {
        // Player 1
        case Qt::Key_Up:    cars[0].present_state.accelerating = true; break;
        case Qt::Key_Left:  cars[0].present_state.setTurn(-turn_speed); break;
        case Qt::Key_Right: cars[0].present_state.setTurn(turn_speed); break;
        case Qt::Key_Down: break;
    }

    if (cars.size() >= 2)
    {
        switch (e->key())
        {
            // Player 2
            case Qt::Key_W: cars[1].present_state.accelerating = true; break;
            case Qt::Key_A: cars[1].present_state.setTurn(-turn_speed); break;
            case Qt::Key_D: cars[1].present_state.setTurn(turn_speed);   break;
            case Qt::Key_S: break;
        }
    }
}

void AnnoyingRacing_Scene::keyReleased(QKeyEvent* e)
{
    switch (e->key())
    {
        // Player 1
        case Qt::Key_Up:    cars[0].present_state.accelerating = false; break;
        case Qt::Key_Left:  cars[0].present_state.setTurn(0); break;
        case Qt::Key_Right: cars[0].present_state.setTurn(0); break;
        case Qt::Key_Down: break;
    }
    if (cars.size() >= 2)
    {
        switch (e->key())
        {
            // Player 2
            case Qt::Key_W: cars[1].present_state.accelerating = false; break;
            case Qt::Key_A: cars[1].present_state.setTurn(0); break;
            case Qt::Key_D: cars[1].present_state.setTurn(0); break;
            case Qt::Key_S: break;
        }
    }
}

SIM_END(AnnoyingRacing_Project)