#include "MaxPractice.h"
SIM_DECLARE(MaxPractice, "Max Practice")

void MaxPractice::prepare()
{
}

void MaxPractice::start()
{
    auto& layout = setLayout(panel_count);
    for (Panel* panel : layout)
        panel->construct<MaxPracticeInstance>();
}

void MaxPracticeInstance::start()
{
    for (int i = 0; i < 100000; ++i)
    {
        Ball ball;
        ball.x = 0;
        ball.y = 0;
        ball.r = 1;
        ball.vx = random(-1, 1);
        ball.vy = random(-1, 1);

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
    }
}

void MaxPracticeInstance::draw(DrawingContext* ctx)
{
    for (Ball &ball : balls)
    {
        ctx->setFillStyle(6, 193, 103);
        ctx->beginPath();
        ctx->circle(ball.x, ball.y, ball.r);
        ctx->fill();
    }

}

SIM_END
