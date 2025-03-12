#include "MaxPractice.h"

SIM_DECLARE(MaxPractice, "Lessons", "Max Practice 1")

void MaxPractice_Project::projectPrepare()
{
    auto& layout = newLayout();
    create<MaxPractice_Scene>(viewport_count)->mountTo(layout);
}

void MaxPractice_Project::projectStart()
{
   
}

void MaxPractice_Scene::sceneAttributes(Input* options)
{
    // SceneBase Settings
    options->realtime_slider("Num Particles", &particle_count, 1, 1000000, 1);
    options->realtime_slider("Max Start Speed", &max_speed, 0.0, 20.0, 0.1);
}

void MaxPractice_Scene::sceneStart()
{
    for (int i = 0; i < particle_count; ++i)
    {
        Ball ball;
        ball.x = 0;
        ball.y = 0;
        ball.rad = random(1,3);
        ball.vx = random(-max_speed, max_speed);
        ball.vy = random(-max_speed, max_speed);
        ball.r = random(0, 255);
        ball.g = random(0, 255);
        ball.b = random(0, 255);

        balls.push_back(ball);
    }
}

void MaxPractice_Scene::sceneDestroy()
{}

void MaxPractice_Scene::sceneProcess()
{
    //uint8_t arr[] = {1,2,3,4,5};
    //std::vector<uint8_t> arr;// = { 1,2,3,4,5 };
    //cache->apply(arr);

    //if (missing_cache())

    if (cache->missing())
    {
        for (int i = 0; i < particle_count; i++)
        {
            Ball& a = balls[i];
            double sum_vx = 0;
            double sum_vy = 0;
            for (int j = i; j < particle_count; j++)
            {
                if (i == j) continue;
                Ball& b = balls[j];
                double f = 0.00000001;
                double dx = b.x - a.x;
                double dy = b.y - a.y;
                a.vx += dx * f;
                a.vy += dy * f;
                b.vx -= dx * f;
                b.vy -= dy * f;
            }
        }
    }

    cache->apply(balls);

    for (Ball &ball : balls)
    {
        ball.x += ball.vx;
        ball.y += ball.vy;

        //ball.vy += 0.1;

        if (ball.x < -world_size + ball.rad)
        {
            ball.x = -world_size + ball.rad;
            ball.vx = -ball.vx;
        }
        if (ball.x > world_size - ball.rad)
        {
            ball.x = world_size - ball.rad;
            ball.vx = -ball.vx;
        }
        if (ball.y < -world_size + ball.rad)
        {
            ball.y = -world_size + ball.rad;
            ball.vy = -ball.vy;
        }
        if (ball.y > world_size - ball.rad)
        {
            ball.y = world_size - ball.rad;
            ball.vy = -ball.vy;
        }
    }
}

void MaxPractice_Scene::viewportDraw(Viewport* ctx)
{
    for (Ball &ball : balls)
    {
        ctx->setFillStyle(ball.r, ball.g, ball.b);
        ctx->beginPath();
        ctx->circle(ball.x, ball.y, ball.rad);
        ctx->fill();
    }

    /*ctx->setStrokeStyle(0, 255, 0, 100);
    ctx->setLineWidth(1);
    for (int i = 0; i < particle_count; i++)
    {
        Ball& a = balls[i];
        for (int j = i+1; j < particle_count; j++)
        {
            Ball& b = balls[j];

            ctx->beginPath();
            ctx->moveTo(a.x, a.y);
            ctx->lineTo(b.x, b.y);
            ctx->stroke();
        }
    }*/

    ctx->setStrokeStyle(255, 255, 255);
    ctx->setLineWidth(1);
    ctx->strokeRect(-world_size, -world_size, world_size*2, world_size*2);
}

SIM_END(MaxPractice)
