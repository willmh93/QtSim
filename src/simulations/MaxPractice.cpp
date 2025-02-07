#include "MaxPractice.h"
SIM_DECLARE(MaxPractice, "Max Practice")

void MaxPractice::prepare()
{
    auto& layout = setLayout(panel_count);
    for (Panel* panel : layout)
        panel->construct<MaxPracticeInstance>();
}

void MaxPractice::start()
{
   
}

void MaxPracticeInstance::instanceAttributes()
{
    // Instance Settings
    options->realtime_slider("Num Particles", &particle_count, 1, 100, 1);
    options->realtime_slider("Max Start Speed", &max_speed, 0.0, 20.0, 0.1);
}

void MaxPracticeInstance::start()
{
    for (int i = 0; i < particle_count; ++i)
    {
        Ball ball;
        ball.x = 0;
        ball.y = 0;
        ball.rad = random(1,50);
        ball.vx = random(-max_speed, max_speed);
        ball.vy = random(-max_speed, max_speed);
        ball.r = random(0, 255);
        ball.g = random(0, 255);
        ball.b = random(0, 255);

        balls.push_back(ball);
    }
}

void MaxPracticeInstance::destroy()
{}

void MaxPracticeInstance::process(DrawingContext* ctx)
{
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

void MaxPracticeInstance::draw(DrawingContext* ctx)
{
    for (Ball &ball : balls)
    {
        ctx->setFillStyle(ball.r, ball.g, ball.b);
        ctx->beginPath();
        ctx->circle(ball.x, ball.y, ball.rad);
        ctx->fill();
    }

    ctx->setStrokeStyle(0, 255, 0, 100);
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
    }

    ctx->setStrokeStyle(255, 255, 255);
    ctx->setLineWidth(1);
    ctx->strokeRect(-world_size, -world_size, world_size, world_size);
}

SIM_END
