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
    car.x = 920;
    car.y = 500 + (viewport->viewportIndex() * 20);
    car.angle = -20 * M_PI / 180.0;

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
    car.process(ctx, map_img);

    camera->x = car.x;
    camera->y = car.y;

    //if (ctx->viewportIndex() == 0)
    //{
    //    // Human player
    //}
    //else
    //{
        // AI player
        //car.accelerating = true;

        car.rays.clear();

        double longest = 0;
        double longest_angle;
        int longest_ray_index;

        for (double test_angle = -90; test_angle < 90; test_angle += 10)
        {
            double angle = test_angle * M_PI / 180.0;
            double ray_length = car.project_dist(angle, map_img);

            if (ray_length > longest)
            {
                // Found new longest ray
                longest = ray_length;
                longest_angle = test_angle;
                longest_ray_index = car.rays.size();
            }

            car.rays.push_back({ test_angle, ray_length, false });
        }

        car.rays[longest_ray_index].longest = true;

        /*if (longest_angle < 0)
        {
            // Turn left
            car.angle -= 0.1;
        }
        else
        {
            // Turn right
            car.angle += 0.1;
        }*/
    //}
}

void AnnoyingRacing_Scene::viewportDraw(Viewport* ctx)
{
    // Draw Scene to Viewport

    ctx->painter->drawImage(*map, 0, 0);
    ctx->drawWorldAxis();

    for (Car &car : cars)
    {
        car.draw(ctx);

        for (auto& ray : car.rays)
        {
            double angle = ray.angle * M_PI / 180.0;
            double d = ray.dist;

            ctx->beginPath();

            if (ray.longest)
                ctx->setStrokeStyle(0, 255, 0);
            else
                ctx->setStrokeStyle(255, 0, 0);

            ctx->moveTo(car.x, car.y);
            ctx->lineTo(
                car.x + cos(car.angle + angle) * d,
                car.y + sin(car.angle + angle) * d
            );
            ctx->stroke();
        }

        /*for (double test_angle = -90; test_angle < 90; test_angle += 10)
        {
            double angle = test_angle * M_PI / 180.0;
            double d = car.project_dist(angle, map_img);

            ctx->beginPath();
            ctx->setStrokeStyle(255, 255, 0);
            ctx->moveTo(car.x, car.y);
            ctx->lineTo(
                car.x + cos(car.angle + angle) * d,
                car.y + sin(car.angle + angle) * d
            );
            ctx->stroke();
        }*/
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
        case Qt::Key_Up:    cars[0].accelerating = true; break;
        case Qt::Key_Left:  cars[0].turning_left = true; break;
        case Qt::Key_Right: cars[0].turning_right = true; break;
        case Qt::Key_Down: break;


        // Player 2
        case Qt::Key_W: cars[1].accelerating = true; break;
        case Qt::Key_A: cars[1].turning_left = true; break;
        case Qt::Key_D: cars[1].turning_right = true; break;
        case Qt::Key_S: break;
    }
}

void AnnoyingRacing_Scene::keyReleased(QKeyEvent* e)
{
    switch (e->key())
    {
        // Player 1
        case Qt::Key_Up:    cars[0].accelerating = false; break;
        case Qt::Key_Left:  cars[0].turning_left = false; break;
        case Qt::Key_Right: cars[0].turning_right = false; break;
        case Qt::Key_Down: break;

        // Player 2
        case Qt::Key_W: cars[1].accelerating = false; break;
        case Qt::Key_A: cars[1].turning_left = false; break;
        case Qt::Key_D: cars[1].turning_right = false; break;
        case Qt::Key_S: break;
    }
}

SIM_END(AnnoyingRacing_Project)