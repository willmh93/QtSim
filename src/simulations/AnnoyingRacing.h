#pragma once
#include "Project.h"

SIM_BEG(AnnoyingRacing_Project)

struct Ray
{
    double angle;
    double dist;
    bool longest;
};

class Car : public Vec2
{
public:

    const double car_w = 20;
    const double car_h = 10;
    const double acceleration = 0.05;

    bool accelerating = false;
    bool turning_left = false;
    bool turning_right = false;
    
    double vx = 0;
    double vy = 0;
    double angle = 0;
    double speed = 0;

    bool on_grass = false;

    std::vector<Ray> rays;

    bool point_on_grass(QImage& map_img, double x, double y)
    {
        QColor pixel = map_img.pixelColor(x, y);
        if (pixel.green() > 50 && pixel.red() < 10 && pixel.blue() < 10)
            return true;
        return false;
    }

    double project_dist(double angle_offset, QImage& map_img)
    {
        double step = 10.0;
        double test_dist = step;
        double project_angle = angle + angle_offset;

        // If car on grass, skip ahead until we find road
        if (on_grass)
        {
            for (int i = 0; i < 50; i++)
            {
                double test_x = x + cos(project_angle) * test_dist;
                double test_y = y + sin(project_angle) * test_dist;

                // Have we found the road?
                if (!point_on_grass(map_img, test_x, test_y))
                    break;

                test_dist += step;
            }
        }

        // Check for grass (i.e. walls)
        for (int i=0; i<300; i++)
        {
            double test_x = x + cos(project_angle) * test_dist;
            double test_y = y + sin(project_angle) * test_dist;

            if (point_on_grass(map_img, test_x, test_y))
                break;

            test_dist += step;
        }

        return test_dist;
    }

    std::vector<Vec2> predict_ray(double angle, QImage& map_img)
    {
        double current_angle = angle;
        double current_x = x;
        double current_y = y;

        for (int i = 0; i < 300; i++)
        {
            // For this future point, test possible angles

            for (double test_angle = -90; test_angle < 90; test_angle += 10)
            {
                double angle = test_angle * M_PI / 180.0;
                double ray_length = project_dist(angle, map_img);

            }
        }
    }

    void process(Viewport* ctx, QImage &map_img)
    {
        on_grass = point_on_grass(map_img, x, y);

        if (turning_left)
            angle -= speed * 0.03;
        if (turning_right)
            angle += speed * 0.03;

        if (accelerating)
        {
            speed += acceleration;
        }

        // Air resistance
        speed *= 0.985;

        if (on_grass)
        {
            speed *= 0.9;
        }

        vx = cos(angle) * speed;
        vy = sin(angle) * speed;

        x += vx;
        y += vy;
    }

    void draw(Viewport* ctx)
    {
        ctx->setFillStyle(255, 0, 255);

        ctx->save();
        ctx->translate(x, y);
        ctx->rotate(angle);
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
    int panel_count = 2;

    void projectAttributes(Options* options) override;
    void projectPrepare() override;
    //void projectStart() override;
    //void projectStop() override;
    //void projectDestroy() override;

};

SIM_END(AnnoyingRacing_Project)