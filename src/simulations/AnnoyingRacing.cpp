#include "AnnoyingRacing.h"
SIM_DECLARE(AnnoyingRacing, "Lessons", "Annoying Racing")

/// Project ///

void AnnoyingRacing_Project::projectAttributes(Input* options)
{
    options->realtime_slider("Panel Count", &panel_count, 1, 16, 1);
}

void AnnoyingRacing_Project::projectPrepare()
{
    auto& layout = newLayout();
    auto scene = create<AnnoyingRacing_Scene>();
    scene->mountTo(layout);
    scene->mountTo(layout);
}

/// Scene ///

void AnnoyingRacing_Scene::sceneAttributes(Input* input)
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

    //car.load(":/resources/car.png");
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


    auto car = make_shared<Car>();
    car->car_img = new QNanoImage(":/resources/car.png");
    car->present_state.x = 920;
    car->present_state.y = 500 + (viewport->viewportIndex() * 20);
    car->present_state.angle = -20 * M_PI / 180.0;

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

    auto car = cars[i];
    car->process(car->present_state, ctx, map_img, 1);

    camera->x = car->present_state.x;
    camera->y = car->present_state.y;

    if (ctx->viewportIndex() == 0)
    {
        // Human player
    }
    else // AI
    {
        // Calculate multiple AI scenarios until crash (at maximum throttle)
        car->reset_ghost_to_present();

        //car->ghost_state.sum_turned = 0;
        car->ghost_state.max_accelerate_frames = 0;
        car->ghost_state.throttle = 1;

        // X frames in the future
        for (int i = 0; i < 100; i++)
        {
            if (car->process(car->ghost_state, ctx, map_img, 1.5))
                break;
            else
                car->ghost_state.max_accelerate_frames++;

            car->determineBestAngle(car->ghost_state, map_img);
            car->applyAIControls(car->ghost_state);
        }
        

        // React to possible throttle scenarios
        {
            QColor pixel = car->getPixel(map_img, car->present_state.x, car->present_state.y);
            if (car->is_road(pixel))
            {
                if (car->ghost_state.max_accelerate_frames >= 99)
                    car->present_state.throttle = 1.5;
                else if (car->ghost_state.max_accelerate_frames >= 80)
                {
                    // Collision far away
                    car->present_state.throttle = 1;
                }
                else if (car->ghost_state.max_accelerate_frames >= 40)
                {
                    // Cruise
                    car->present_state.throttle = 0.9;
                }
                else if (car->ghost_state.max_accelerate_frames >= 20)
                {
                    // Collision near-ish
                    if (car->present_state.speed > 1)
                        car->present_state.throttle = -1;
                    else
                        car->present_state.throttle = 0.7;
                }
                else
                {
                    // Collision very near
                    if (car->present_state.speed > 2)
                        car->present_state.throttle = -1;
                    else
                        car->present_state.throttle = 0.4;
                }
            }
            else
            {
                car->present_state.throttle = 1;
            }
        }

        car->determineBestAngle(car->present_state, map_img, true);
        car->applyAIControls(car->present_state);
   }
}

void AnnoyingRacing_Scene::viewportDraw(Viewport* ctx)
{
    // Draw Scene to Viewport

    ctx->painter->drawImage(*map, 0, 0);
    ctx->drawWorldAxis();


    for (auto car : cars)
    {
        car->draw(ctx, car->present_state, car->car_img);

        /*car.reset_ghost_to_present();
        car.ghost_state.turn_bias = max_turn / 10.0;

        for (int i = 0; i < 100; i++)
        {
            bool collided = car.process(car.ghost_state, ctx, map_img, 2);
            car.determineBestAngle(car.ghost_state, map_img);


            if (car.ghost_state.best_ray_angle < 0)
            {
                // Turn left
                car.ghost_state.setTurn(-max_turn);
                car.ghost_state.sum_turned -= max_turn;
            }
            else
            {
                // Turn right
                car.ghost_state.setTurn(max_turn);
                car.ghost_state.sum_turned += max_turn;
            }

            if (i % 20 == 0)
                car.draw(ctx, car.ghost_state, car_img);
        }

        */
        /*for (auto& ray : car->rays)
        {
            double angle = ray.ray_angle;
            double d = ray.dist;

            ctx->beginPath();

            if (ray.longest || ray.shortest)
            {
                ctx->setStrokeStyle(0, 255, 0);

                ctx->moveTo(car->present_state.x, car->present_state.y);
                ctx->lineTo(
                    car->present_state.x + cos(angle) * d,
                    car->present_state.y + sin(angle) * d
                );
            }
            else
            {
                ctx->setStrokeStyle(255, 0, 0, 50);

                ctx->moveTo(car->present_state.x, car->present_state.y);
                ctx->lineTo(
                    car->present_state.x + cos(angle) * d,
                    car->present_state.y + sin(angle) * d
                );
            }


            ctx->stroke();
        }

        ctx->setStrokeStyle(255, 255, 0);
        ctx->beginPath();
        ctx->moveTo(car->present_state.x, car->present_state.y);
        double best_angle = car->present_state.ai_target_angle;
        ctx->lineTo(
            car->present_state.x + cos(best_angle) * 200,
            car->present_state.y + sin(best_angle) * 200
        );
        ctx->stroke();

        ctx->print() << car->ghost_state.max_accelerate_frames << "\n";
        ctx->print() << car->present_state.speed;*/

        /*if (car.ghost_state.sum_turned < 0)
            ctx->print() << "LEFT";
        else
            ctx->print() << "RIGHT";
        */
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
        case Qt::Key_Up:    cars[0]->present_state.setThrottle(1.5); break;
        case Qt::Key_Left:  cars[0]->present_state.setTurn(-max_turn); break;
        case Qt::Key_Right: cars[0]->present_state.setTurn(max_turn); break;
        case Qt::Key_Down:  cars[0]->present_state.setThrottle(-1); break;
    }

    if (cars.size() >= 2)
    {
        switch (e->key())
        {
            // Player 2
            case Qt::Key_W: cars[1]->present_state.setThrottle(1.5); break;
            case Qt::Key_A: cars[1]->present_state.setTurn(-max_turn); break;
            case Qt::Key_D: cars[1]->present_state.setTurn(max_turn);   break;
            case Qt::Key_S: cars[1]->present_state.setThrottle(-1); break;
        }
    }
}

void AnnoyingRacing_Scene::keyReleased(QKeyEvent* e)
{
    switch (e->key())
    {
        // Player 1
        case Qt::Key_Up:    cars[0]->present_state.setThrottle(0); break;
        case Qt::Key_Left:  cars[0]->present_state.setTurn(0); break;
        case Qt::Key_Right: cars[0]->present_state.setTurn(0); break;
        case Qt::Key_Down:  cars[1]->present_state.setThrottle(0); break;
    }
    if (cars.size() >= 2)
    {
        switch (e->key())
        {
            // Player 2
            case Qt::Key_W: cars[1]->present_state.setThrottle(0); break;
            case Qt::Key_A: cars[1]->present_state.setTurn(0); break;
            case Qt::Key_D: cars[1]->present_state.setTurn(0); break;
            case Qt::Key_S: cars[1]->present_state.setThrottle(0); break;
        }
    }
}

SIM_END(AnnoyingRacing)