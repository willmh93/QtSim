#pragma once
#include "Project.h"

SIM_BEG(AnnoyingRacing_Project)

struct Ray
{
    double angle;
    double dist;
    bool longest;
};

const double car_w = 20;
const double car_h = 10;
const double acceleration = 0.05;
const double turn_speed = 0.02;

struct CarState
{
    double x;
    double y;
    double vx = 0;
    double vy = 0;
    double angle = 0;
    double speed = 0;

    double turningSpeed = 0;
    bool accelerating = false;

    double turn_bias = 0;

    double sum_turned = 0;

    //bool on_grass = false;

    void setTurn(double _turn_speed)
    {
        turningSpeed = _turn_speed;
    }
};

class Car// : public Vec2
{
public:



    CarState present_state;
    CarState ghost_state;

    std::vector<Ray> rays;

    bool is_grass(const QColor& pixel)
    {
        if (pixel.green() > 50)
            return true;
        return false;
    }
    bool is_outside_map(const QColor& pixel)
    {
        if (pixel.alpha() < 10)
            return true;
        return false;
    }

    /*bool point_on_grass(QImage& map_img, double x, double y)
    {
        QColor pixel = map_img.pixelColor(x, y);
        if (pixel.green() > 50 && pixel.red() < 10 && pixel.blue() < 10)
            return true;
        return false;
    }

    bool point_outside_map(QImage& map_img, double x, double y)
    {
        QColor pixel = map_img.pixelColor(x, y);
        if (pixel.alpha() < 10)
            return true;
        return false;
    }*/

    double project_dist(CarState &state, double angle_offset, QImage& map_img)
    {
        double step = 10.0;
        double test_dist = step;
        double project_angle = state.angle + angle_offset;

        // If car on grass, skip ahead until we find road
        QColor pixel = map_img.pixelColor(state.x, state.y);
        bool on_grass = is_grass(pixel);

        int w = map_img.width();
        int h = map_img.height();

        if (on_grass)
        {
            for (int i = 0; i < 50; i++)
            {
                double test_x = state.x + cos(project_angle) * test_dist;
                double test_y = state.y + sin(project_angle) * test_dist;
                if (test_x < 0 || test_y < 0 || test_x >= w || test_y >= h)
                    return test_dist;

                QColor pixel = map_img.pixelColor(test_x, test_y);

                if (is_outside_map(pixel))
                    break;

                // Have we found the road?
                if (!is_grass(pixel))
                    break;

                test_dist += step;
            }
        }

        // Check for grass (i.e. walls)
        for (int i=0; i<200; i++)
        {
            double test_x = state.x + cos(project_angle) * test_dist;
            double test_y = state.y + sin(project_angle) * test_dist;
            if (test_x < 0 || test_y < 0 || test_x >= w || test_y >= h)
                return test_dist;

            QColor pixel = map_img.pixelColor(test_x, test_y);
            if (is_outside_map(pixel))
                break;

            if (is_grass(pixel))
                break;

            test_dist += step;
        }

        return test_dist;
    }

    void process(CarState &state, Viewport* ctx, QImage &map_img, double dt)
    {
        QColor pixel = map_img.pixelColor(state.x, state.y);
        bool on_grass = is_grass(pixel);

        state.angle += state.turningSpeed * state.speed * dt;

        if (state.accelerating)
        {
            state.speed += acceleration;
        }

        // Air resistance
        state.speed *= 0.985;

        if (on_grass)
        {
            state.speed *= 0.9;
        }

        state.vx = cos(state.angle) * state.speed;
        state.vy = sin(state.angle) * state.speed;

        state.x += state.vx * dt;
        state.y += state.vy * dt;
    }

    /*void process_future(CarState& state, Viewport* ctx, QImage& map_img)
    {
        bool ghost_on_grass = point_on_grass(map_img, state.x, state.y);

        state.angle += state.turningSpeed * state.speed;


        // Assume always accelerating
        state.speed += acceleration;

        // Air resistance
        state.speed *= 0.985;

        if (ghost_on_grass)
        {
            state.speed *= 0.9;
        }

        state.vx = cos(state.angle) * state.speed;
        state.vy = sin(state.angle) * state.speed;

        state.x += state.vx;
        state.y += state.vy;

        aiProcess(state, map_img);
    }*/


    void aiProcess(CarState &state, QImage& map_img)
    {
        state.accelerating = true;
        rays.clear();

        double longest = 0;
        double longest_angle;
        int longest_ray_index;

        for (double test_angle = -90; test_angle < 90; test_angle += 1)
        {
            double angle = test_angle * M_PI / 180.0;
            double ray_length = project_dist(state, angle, map_img);

            if (ray_length > longest)
            {
                // Found new longest ray
                longest = ray_length;
                longest_angle = test_angle;
                longest_ray_index = rays.size();
            }

            rays.push_back({ test_angle, ray_length, false });
        }

        rays[longest_ray_index].longest = true;

        if (longest_angle < 0)
        {
            // Turn left
            state.setTurn(-turn_speed);
            state.sum_turned -= turn_speed;
        }
        else
        {
            // Turn right
            state.setTurn(turn_speed);
            state.sum_turned += turn_speed;
        }
    }

    void reset_ghost_to_present()
    {
        ghost_state.x = present_state.x;
        ghost_state.y = present_state.y;
        ghost_state.angle = present_state.angle;
        ghost_state.vx = present_state.vx;
        ghost_state.vy = present_state.vy;
        ghost_state.speed = present_state.speed;
    }


    void draw(CarState &state, Viewport* ctx, double alpha)
    {
        ctx->setFillStyle(255, 0, 255, alpha);

        ctx->save();
        ctx->translate(state.x, state.y);
        ctx->rotate(state.angle);
        ctx->fillRect(-car_w / 2 , -car_h / 2, car_w, car_h);
        ctx->restore();
    }
};

struct AnnoyingRacing_Scene : public Scene
{
/*
    // Custom Launch Config Example

    struct LaunchConfig
    {
        double particle_speed = 10.0;
    };
    
    AnnoyingRacing_Scene(LaunchConfig& info) : 
        particle_speed(info.particle_speed)
    {}
    
    double particle_speed;
*/

    vector<Car> cars;

    QImage map_img;
    QNanoImage *map;

    void sceneAttributes(Options* options) override;

    void sceneStart() override;
    //void sceneStop() override;
    void sceneDestroy() override;
    void sceneMounted(Viewport* viewport) override;
    void sceneProcess() override;

    void viewportProcess(Viewport* ctx) override;
    void viewportDraw(Viewport* ctx) override;

    void mouseDown() override;
    void mouseUp() override;
    void mouseMove() override;
    void mouseWheel() override;

    void keyPressed(QKeyEvent* e) override;
    void keyReleased(QKeyEvent* e) override;
};

struct AnnoyingRacing_Project : public Project<AnnoyingRacing_Scene>
{
    int panel_count = 1;

    void projectAttributes(Options* options) override;
    void projectPrepare() override;
    //void projectStart() override;
    //void projectStop() override;
    //void projectDestroy() override;

};

SIM_END(AnnoyingRacing_Project)