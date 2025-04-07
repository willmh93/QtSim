#pragma once
#include "Project.h"

SIM_BEG(AnnoyingRacing)

struct Ray
{
    double ray_angle;
    double dist;
    bool shortest;
    bool longest;
};

const double car_w = 20;
const double car_h = 10;
const double acceleration = 0.05;
const double max_turn = 0.01;

struct CarState
{
    double x;
    double y;
    double vx = 0;
    double vy = 0;
    double angle = 0;
    double speed = 0;

    double turningSpeed = 0;
    double throttle = 0.0; // 0 -> 1
    //bool accelerating = false;

    double turn_bias = 0;

    //double sum_turned = 0;
    double best_ray_angle;
    double ai_target_angle = 0;
    double best_ray = 0;

    int max_accelerate_frames = 0;

    //bool on_grass = false;

    void setThrottle(double _throttle)
    {
        throttle = _throttle;
    }
    void setTurn(double _turn_speed)
    {
        if (_turn_speed > max_turn)
            _turn_speed = max_turn;
        else if (_turn_speed < -max_turn)
            _turn_speed = -max_turn;

        //turningSpeed = _turn_speed;
        turningSpeed += (_turn_speed - turningSpeed) * 0.5;
    }
};

class Car// : public Vec2
{
public:

    //QNanoImage nano_img;

    CarState present_state;
    CarState ghost_state;

    std::vector<Ray> rays;
    QNanoImage* car_img;

    bool is_road(const QColor& pixel)
    {
        if (pixel.red() > 50)
            return true;
        return false;
    }
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

    inline QColor getPixel(QImage& map_img, double x, double y)
    {
        if (x < 0 || y < 0 || x >= map_img.width() || y >= map_img.height())
            return QColor(0, 0, 0, 0);

        return map_img.pixelColor(x, y);
    }

    double project_dist_to_grass(CarState &state, double angle_offset, double ray_width, QImage& map_img)
    {
        double step = 2;
        double test_dist = step;
        double project_angle = state.angle + angle_offset;

        // If car on grass, skip ahead until we find road
        QColor pixel = getPixel(map_img, state.x, state.y);
        bool on_grass = is_grass(pixel);

        int w = map_img.width();
        int h = map_img.height();

        double cosf = cos(project_angle);
        double sinf = sin(project_angle);

        double perp_angle = project_angle + M_PI_2;
        double ray_hit_offX = cos(perp_angle) * ray_width;
        double ray_hit_offY = sin(perp_angle) * ray_width;

        if (on_grass)
        {
            for (int i = 0; i < 50; i++)
            {
                double test_x = state.x + cosf * test_dist;
                double test_y = state.y + sinf * test_dist;
                if (test_x < 0 || test_y < 0 || test_x >= w || test_y >= h)
                    return test_dist;

                QColor pixel = getPixel(map_img, test_x, test_y);

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
            double test_x = state.x + cosf * test_dist;
            double test_y = state.y + sinf * test_dist;
            if (test_x < ray_width || test_y < ray_width || test_x >= w- ray_width || test_y >= h- ray_width)
                return test_dist;

            QColor pixel_a = getPixel(map_img, test_x - ray_hit_offX, test_y - ray_hit_offY);
            QColor pixel_b = getPixel(map_img, test_x + ray_hit_offX, test_y + ray_hit_offY);
            if (is_outside_map(pixel_a) || is_grass(pixel_a)) break;
            if (is_outside_map(pixel_b) || is_grass(pixel_b)) break;

            test_dist += step;
        }

        return test_dist;
    }

    double project_dist_to_road(CarState& state, double angle_offset, QImage& map_img)
    {
        double step = 10.0;
        double test_dist = step;
        double project_angle = state.angle + angle_offset;

        // If car on grass, skip ahead until we find road
        QColor pixel = getPixel(map_img, state.x, state.y);
        if (is_road(pixel))
            return 0;

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

                QColor pixel = getPixel(map_img, test_x, test_y);

                if (is_outside_map(pixel))
                    return 10000.0;

                // Have we found the road?
                if (is_road(pixel))
                    return test_dist;

                test_dist += step;
            }
        }

        return test_dist;
    }

    bool process(CarState &state, Viewport* ctx, QImage &map_img, double dt)
    {
        QColor pixel = getPixel(map_img, state.x, state.y);
        bool on_grass = is_grass(pixel);

        state.angle += state.turningSpeed * pow(state.speed * 10, 0.2) * 2 * dt;
        state.speed += state.throttle * acceleration;
        

        // Air resistance
        state.speed *= 0.993;

        if (on_grass)
        {
            state.speed *= 0.9;
        }

        state.vx = cos(state.angle) * state.speed;
        state.vy = sin(state.angle) * state.speed;

        state.x += state.vx * dt;// *0.2;
        state.y += state.vy * dt;// *0.2;

        return on_grass;
    }

    /*void process_future(CarState& state, Viewport* ctx, QImage& map_img)
    {
        bool ghost_on_grass = point_on_grass(map_img, state.x, state.y);

        state.ray_angle += state.turningSpeed * state.speed;


        // Assume always accelerating
        state.speed += acceleration;

        // Air resistance
        state.speed *= 0.985;

        if (ghost_on_grass)
        {
            state.speed *= 0.9;
        }

        state.vx = cos(state.ray_angle) * state.speed;
        state.vy = sin(state.ray_angle) * state.speed;

        state.x += state.vx;
        state.y += state.vy;

        aiProcess(state, map_img);
    }*/


    void determineBestAngle(CarState &state, QImage& map_img, bool save_rays=false)
    {
        QColor pixel = getPixel(map_img, state.x, state.y);
        bool on_grass = is_grass(pixel);

        if (save_rays)
            rays.clear();

        if (on_grass)
        {
            // Get off the grass by taking shortest route
            double shortest = std::numeric_limits<double>::max();
            double shortest_ray_angle;
            int shortest_ray_index;

            for (double test_angle_offset_deg = -90; test_angle_offset_deg < 90; test_angle_offset_deg += 2.5)
            {
                double test_angle_offset_rad = test_angle_offset_deg * M_PI / 180.0;

                double ray_angle = state.angle + (test_angle_offset_deg * M_PI / 180.0);
                double ray_length = project_dist_to_grass(state, test_angle_offset_rad, 10, map_img);

                if (ray_length < shortest)
                {
                    // Found new longest ray
                    shortest = ray_length;
                    shortest_ray_angle = ray_angle;
                    shortest_ray_index = rays.size();
                }

                if (save_rays)
                    rays.push_back({ ray_angle, ray_length, false, false });
            }

            if (save_rays)
                rays[shortest].shortest = true;

            state.best_ray_angle = shortest_ray_angle;
            state.best_ray = shortest;
        }
        else
        {

            double longest = 0;
            double longest_ray_angle;
            int longest_ray_index;

            for (double test_angle_offset_deg = -72.5; test_angle_offset_deg < 72.5; test_angle_offset_deg += 2.5)
            {
                double test_angle_offset_rad = test_angle_offset_deg * M_PI / 180.0;

                double ray_angle = state.angle + (test_angle_offset_deg * M_PI / 180.0);
                double ray_length = project_dist_to_grass(state, test_angle_offset_rad, 10, map_img);

                if (ray_length > longest)
                {
                    // Found new longest ray
                    longest = ray_length;
                    longest_ray_angle = ray_angle;
                    longest_ray_index = rays.size();
                }

                if (save_rays)
                {
                    rays.push_back({ ray_angle, ray_length, false, false });
                }
            }

            if (save_rays)
                rays[longest_ray_index].longest = true;

            state.best_ray_angle = longest_ray_angle;
            state.best_ray = longest;
        }
        //state.ai_target_angle = state.best_ray_angle;

        state.ai_target_angle += 0.1 * // (state.best_ray_angle - state.ai_target_angle);
            closestAngleDifference(state.ai_target_angle, state.best_ray_angle);
    }

    void applyAIControls(CarState& state)
    {
        state.setTurn(closestAngleDifference(state.angle, state.ai_target_angle));
        /*if (closestAngleDifference(state.angle, state.ai_target_angle) < 0)
        {
            // Turn left
            state.setTurn(-max_turn);
        }
        else
        {
            // Turn right
            state.setTurn(max_turn);
        }*/
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


    void draw(Viewport* ctx, CarState &state, QNanoImage* car_img)
    {
        const float car_scale = 0.04f;
        double car_world_x = state.x;
        double car_world_y = state.y;

        ctx->painter->save();
        ctx->painter->translate(car_world_x, car_world_y);
        ctx->painter->rotate(state.angle);
        ctx->painter->scale(car_scale);
        ctx->painter->translate(-car_img->width() / 2, -car_img->height() / 2);
        ctx->painter->drawImage(*car_img, 0, 0);
        ctx->painter->restore();
    }

    /*void draw(CarState &state, Viewport* ctx, double alpha)
    {
        ctx->setFillStyle(255, 0, 255, alpha);

        ctx->save();
        ctx->translate(state.x, state.y);
        ctx->rotate(state.ray_angle);
        ctx->fillRect(-car_w / 2 , -car_h / 2, car_w, car_h);
        ctx->restore();
    }*/
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

    vector<shared_ptr<Car>> cars;

    QImage map_img;
    QNanoImage *map;
    

    void sceneAttributes() override;

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

struct AnnoyingRacing_Project : public Project
{
    int panel_count = 2;

    void projectAttributes() override;
    void projectPrepare() override;
    //void projectStart() override;
    //void projectStop() override;
    //void projectDestroy() override;

};

SIM_END(AnnoyingRacing)