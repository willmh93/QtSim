#include "SpaceEngine.h"
#include "../Options.h"
SIM_BEG(SpaceEngine)

void Sim::prepare()
{
    //camera.enable();

    attachCameraControls(&cam);

    //n.x = random(-world_size / 2, world_size / 2);
     //n.y = random(-world_size / 2, world_size / 2);
     //n.mass = start_particle_mass;
     //n.angle = 0;// random(-M_PI, M_PI);
     //n.spin = (a - M_PI) * 0.001;// random(-M_PI / 200.0, M_PI / 200.0);

    //templates_combo = options->combo("Templates");
    //templates_combo->addComboItem("Earth Moon", [this]()
    //{
    //    prepareActiveTemplate();
    //});
    //templates_combo->setActiveComboItem("Earth Moon");

    //options->slider("Particles", &particle_count, 10, 20000);

    options->checkbox("Optimize Gravity", &optimize_gravity);
    options->checkbox("Optimize Collisions", &optimize_collisions);

    options->checkbox("Draw Gravity Grid", &draw_gravity_grid);
    options->checkbox("Draw Collision Grid", &draw_collision_grid);


    options->slider("Steps Per Frame", &steps_per_frame, 1, 100, 1);
    
    AttributeItem* timestep_slider = options->slider("Time step (seconds)",
        &step_seconds, 
        &step_seconds_min, 
        &step_seconds_max, 
        &step_seconds_step);

    timestep_slider->label_value = [this](double seconds)
    {
        double seconds_processed_by_second = seconds * 60;
        return normalizeSeconds(seconds_processed_by_second) + "/ second";
    };


    options->slider("Collision Substeps", &collision_substeps, 1, 20, 1);


    options->slider("Gravity Cell Ratio", &gravity_cell_near_ratio, 0.001, 0.1, 0.001);
    options->slider("Gravity Near-Distance Cells", &gravity_cell_near_grid_radius, 1, 10, 1);

    //options->slider("Gravity Cell Far-Ratio", &gravity_cell_far_ratio, 0.01, 0.5, 0.01);
    //options->slider("Collision Cell Size", &collision_cell_size, &collision_cell_size_min, &collision_cell_size_max, &collision_cell_size_step);
    //options->slider("Max Speed", &max_speed, 0.0, 1000.0, 0.1);
    //options->slider("Minimum Particle Distance", &min_particle_dist, 1.0, 50.0, 0.1);

    ///

    options->slider("World Size (gm) ", &start_world_size, &world_size_min, &world_size_max, &world_size_step);
}


void Sim::start()
{
    world_size = start_world_size;
    bmp_scale = world_size / density_bmp_size;
    time_elapsed = 0;

    cam.setZoom(((double)height()) / world_size);

    density_bmp.create(density_bmp_size, density_bmp_size, true);
    
    particles.clear();
    //particles = start_particles;
}

void Sim::destroy()
{
    particles.clear();
}

/*void Sim::checkCollision(Particle& n0, Particle& n1)
{
    double dx = n1.x - n0.x;
    double dy = n1.y - n0.y;
    double distSq = dx * dx + dy * dy;
    double combined_r = n0.r + n1.r;

    // Check if particles are overlapping
    if (distSq < (combined_r * combined_r))
    {
        double dist = std::sqrt(distSq);
        if (dist == 0.0) return; // Avoid divide by zero

        // Calculate overlap
        double overlap = combined_r - dist;

        // Normalize the collision vector
        double nx = dx / dist;
        double ny = dy / dist;

        // Relative velocity
        double dvx = n1.vx - n0.vx;
        double dvy = n1.vy - n0.vy;

        // Velocity along the normal
        double vn = dvx * nx + dvy * ny;

        // If particles are moving apart, no collision
        if (vn > 0) return;

        // Impulse scalar (elastic collision)
        double impulse = (2 * vn) / (n0.mass + n1.mass);

        // Apply impulse to both particles
        double impulseX = impulse * nx;
        double impulseY = impulse * ny;

        n0.vx -= impulseX * n1.mass;
        n0.vy -= impulseY * n1.mass;

        n1.vx += impulseX * n0.mass;
        n1.vy += impulseY * n0.mass;

        // Resolve overlap proportionally to their masses
        double correction = overlap / (n0.mass + n1.mass);
        n0.x -= correction * nx * n1.mass;
        n0.y -= correction * ny * n1.mass;
        n1.x += correction * nx * n0.mass;
        n1.y += correction * ny * n0.mass;
    }
}*/

void Sim::process()
{
    for (int i = 0; i < steps_per_frame; i++)
    {
        processGravity();
        processCollisions();

        for (Particle& n : particles)
        {
            n.x += n.vx * step_seconds;
            n.y += n.vy * step_seconds;
        }

        time_elapsed += step_seconds;
    }

 
    vector<vector<double>> density_map;
    double min_density = std::numeric_limits<double>::max();
    double max_density = 0;

    double world_pixel_radius = bmp_scale / 2;

    int len = particles.size();
    for (int y = 0; y < density_bmp_size; y++)
    {
        vector<double> density_row;
        for (int x = 0; x < density_bmp_size; x++)
        {
            double cx = ((-density_bmp_size / 2) + (x + 0.5)) * bmp_scale;
            double cy = ((-density_bmp_size / 2) + (y + 0.5)) * bmp_scale;
            double sum_density = 0;

            for (int i = 0; i < len; i++)
            {
                Particle& n = particles[i];
                //if (n.x < left || n.y < top || n.x > right || n.y > bottom)
               
                double dx = n.x - cx;
                double dy = n.y - cy;
                double d = sqrt(dx * dx + dy * dy);
                if (d < world_pixel_radius) d = world_pixel_radius;
                sum_density += 1 / (d + 0.001);
            }

            double avg_density = sum_density / len;
            if (avg_density > max_density) max_density = avg_density;
            if (avg_density < min_density) min_density = avg_density;

            density_row.push_back(avg_density);
        }
        density_map.emplace_back(density_row);
    }

    double density_range = (max_density - min_density);
    for (int y = 0; y < density_bmp_size; y++)
    {
        vector<double>& dist_row = density_map[y];
        for (int x = 0; x < density_bmp_size; x++)
        {
            double avg_density = dist_row[x];
            double normalized_density = (avg_density - min_density) / density_range;
            int heat = (int)(normalized_density * 255);
            density_bmp.setPixel(x, y, heat, 0, 0, 170);
        }
    }
}

void Sim::buildUniformGrid(double cellSize, ParticleGrid& grid)
{
    grid.clear();
    grid.reserve(particles.size());
    grid.cell_size = cellSize;

    for (int i = 0; i < (int)particles.size(); i++)
    {
        Particle& p = particles[i];

        // Compute cell for the center of this particle
        CellCoord c = computeCellCoord(p.x, particles[i].y, cellSize);
        CellData& cellData = grid[c];
        if (!cellData.initialized)
        {
            cellData.cx = (floor(p.x / cellSize) * cellSize) + cellSize / 2;
            cellData.cy = (floor(p.y / cellSize) * cellSize) + cellSize / 2;
            cellData.initialized = true;
        }

        cellData.particles.push_back(&p);
    }
}

void Sim::processParticlePairGravity(Particle* n0, Particle* n1)
{
    double dx = n1->x - n0->x;
    double dy = n1->y - n0->y;
    double r = sqrt(dx * dx + dy * dy);

    double force_magnitude = gravity * (n0->mass * n1->mass) / (r * r); // No r^2 because F = G_2D * m1 * m2 / r
    double fx = force_magnitude * (dx / r);
    double fy = force_magnitude * (dy / r);

    // Compute accelerations
    double ax = fx / n0->mass;
    double ay = fy / n0->mass;

    // By Newton's Third Law, apply the opposite force to particle 2
    double ax2 = -fx / n1->mass;
    double ay2 = -fy / n1->mass;

    // Update velocities of particle 1
    n0->vx += ax * step_seconds;
    n0->vy += ay * step_seconds;

    n1->vx += ax2 * step_seconds;
    n1->vy += ay2 * step_seconds;
}

inline float fastSqrt(float number) {
    if (number == 0.0f) return 0.0f;

    int32_t i;
    float x2, y;
    const float threehalfs = 1.5F;

    x2 = number * 0.5F;
    y = number;
    // Bit-level hack to get initial approximation
    i = *(int*)&y;               // Convert float to int bits
    i = 0x5f375a86 - (i >> 1);   // Magic constant for sqrt
    y = *(float*)&i;             // Convert bits back to float
    y = y * (threehalfs - (x2 * y * y)); // 1st iteration of Newton-Raphson

    // Optional: Uncomment the next line for a second iteration to improve accuracy
    // y  = y * (threehalfs - (x2 * y * y));

    return number * y; // Since y is approximately 1/sqrt(number), multiply to get sqrt(number)
}

void Sim::processCellPairGravity(CellData& c0, CellData& c1)
{
    double dx = c1.cx - c0.cx;
    double dy = c1.cy - c0.cy;
    double r = sqrt(dx * dx + dy * dy);

    /*float dx = c1.com_x - c0.com_x;
    float dy = c1.com_y - c0.com_y;
    float r = fastSqrt(dx * dx + dy * dy);

    float force_magnitude = gravity * (c0.total_mass * c1.total_mass) / (r * r); // No r^2 because F = G_2D * m1 * m2 / r
    float fx = force_magnitude * (dx / r);
    float fy = force_magnitude * (dy / r);

    // Compute accelerations
    double ax1 = fx / c0.total_mass;
    double ay1 = fy / c0.total_mass;*/

    // By Newton's Third Law, apply the opposite force to particle 2
    //double ax2 = -fx / c1.total_mass;
    //double ay2 = -fy / c1.total_mass;

    // Update velocities of particle 1
    for (Particle* p : c0.particles)
    {
        float dx = c1.com_x - p->x;
        float dy = c1.com_y - p->y;
        //double r = fastSqrt(dx * dx + dy * dy);

        double force_magnitude = gravity * (p->mass * c1.total_mass) / (r * r); // No r^2 because F = G_2D * m1 * m2 / r
        double fx = force_magnitude * (dx / r);
        double fy = force_magnitude * (dy / r);

        // Compute accelerations
        double ax1 = fx / c0.total_mass;
        double ay1 = fy / c0.total_mass;

        p->vx += ax1 * step_seconds;
        p->vy += ay1 * step_seconds;
    }

    //for (Particle* p : c1.particles)
    //{
    //    p->vx += ax2 * step_seconds;
    //    p->vy += ay2 * step_seconds;
    //}
}


void Sim::processGravity()
{
    int len = particles.size();

    if (optimize_gravity)
    {
        // Build the uniform grid
        double cellSize = world_size * gravity_cell_near_ratio;

        gravity_grid_near.is_gravity = true;
        buildUniformGrid(world_size * gravity_cell_near_ratio, gravity_grid_near);
        //buildUniformGrid(world_size * gravity_cell_far_ratio, gravity_grid_far);

        // Calculate all short-distance attractions first, flag them as "gravity_applied"

        forEachCellAndNeighbors(gravity_grid_near, [&](Particle* a, Particle* b)
        {
            processParticlePairGravity(a, b);
        }, gravity_cell_near_grid_radius);

        for (auto& cell : gravity_grid_near)
        {
            CellData& cellData = cell.second;

            cellData.total_mass = 0;

            double sum_x_mass_prod = 0;
            double sum_y_mass_prod = 0;
            double sum_mass = 0;

            for (Particle* n : cellData.particles)
            {
                sum_x_mass_prod += n->x * n->mass;
                sum_y_mass_prod += n->y * n->mass;
                sum_mass += n->mass;
            }

            cellData.com_x = (sum_x_mass_prod / sum_mass);
            cellData.com_y = (sum_y_mass_prod / sum_mass);
            cellData.total_mass = sum_mass;
        }

        int r = gravity_cell_near_grid_radius;
        for (auto it1 = gravity_grid_near.begin(); it1 != gravity_grid_near.end(); ++it1)
        {
            int cx = it1->first.cx;
            int cy = it1->first.cy;

            //for (auto it2 = std::next(it1); it2 != gravity_grid_near.end(); ++it2)
            for (auto it2 = gravity_grid_near.begin(); it2 != gravity_grid_near.end(); ++it2)
            {
                int dx = it2->first.cx - cx;
                int dy = it2->first.cy - cy;
                if (dx < 0) dx = -dx;
                if (dy < 0) dy = -dy;
                if (dx <= r && dy <= r)
                    continue;

                processCellPairGravity(it1->second, it2->second);
            }
        }
    }
    else
    {
        // Unoptimized
        for (int i = 0; i < len; i++)
        {
            for (int j = i + 1; j < len; j++)
            {
                Particle* a = &particles[i];
                Particle* b = &particles[j];
                processParticlePairGravity(a, b);
            }
        }
    }
    
    

    /*for (auto& kv : grid)
    {
        const CellCoord& cell = kv.first;
        CellData& cellData = kv.second;

        // For each neighbor cell (including the cell itself)
        for (int ny = -1; ny <= 1; ny++)
        {
            for (int nx = -1; nx <= 1; nx++)
            {
                CellCoord neighborCell{ cell.cx + nx, cell.cy + ny };

                // Find the neighbor cell in the map
                auto it = grid.find(neighborCell);
                if (it == grid.end()) continue;

                CellData& neighborCellData = it->second;

                if (nx == 0 && ny == 0) 
                {
                    std::vector<Particle*> cell_particles = cellData.particles;
                    size_t cell_len = cell_particles.size();
                    for (size_t i=0; i< cell_len; i++)
                    {
                        Particle* n0 = cell_particles[i];
                        for (size_t j = i+1; j < cell_len; j++)
                        {
                            Particle* n1 = cell_particles[j];
                            processParticlePairGravity(n0, n1);
                        }
                    }
                }
                else if (nx >= 0 && ny >= 0) 
                {
                    // to avoid double checks with neighbors,
                    // we can define a rule: only check "forward" neighbors
                    // e.g. (nx >= 0 && ny >= 0).
                    // Then do all i in cell vs j in neighborCell.
                    for (Particle* n0 : cellData.particles)
                    {
                        for (Particle* n1 : neighborCellData.particles)
                        {
                            processParticlePairGravity(n0, n1);
                        }
                    }
                }
                // else do nothing if (nx<0 or ny<0) to avoid duplicates
            }
        }

        // Calculate total mass of each cell (after minor gravity change)
        cellData.total_mass = 0;
        for (Particle* n : cellData.particles)
        {
            cellData.total_mass += n->mass;
        }
    }*/

    

    /*for (int i = 0; i < len; i++)
    {
        Particle& n0 = particles[i];
        for (int j = i + 1; j < len; j++)
        {
            Particle& n1 = particles[j];
            double dx = n1.x - n0.x;
            double dy = n1.y - n0.y;
            double d2 = (dx * dx + dy * dy);
            //if (d < 0.001) d = 0.001;

            double angle = atan2_approximation2(dy, dx);
            double force = G * ((n0.mass * n1.mass) / d2);
            double cosr = cos(angle);
            double sinr = sin(angle);

            n0.vx += cosr * force;
            n0.vy += sinr * force;
            n1.vx -= cosr * force;
            n1.vy -= sinr * force;
        }
        //n0.vx *= 0.99;
        //n0.vy *= 0.99;
    }*/
}

// Return true if a collision was resolved, false if not
bool Sim::checkAndResolveCollision(Particle* n0, Particle* n1)
{
    double dx = n1->x - n0->x;
    double dy = n1->y - n0->y;
    double dist2 = dx * dx + dy * dy;
    double rSum = n0->r + n1->r;

    if (dist2 >= rSum * rSum) {
        return false; // no collision
    }

    double dist = std::sqrt(dist2);
    if (dist < 1e-12) {
        // Prevent division by zero
        // You could either skip or forcibly nudge them...
        return false;
    }

    // 1) Compute the collision normal
    double nx = dx / dist; // unit collision normal x
    double ny = dy / dist; // unit collision normal y

    // 2) Compute overlap
    double overlap = rSum - dist;

    // 3) Positionally correct so circles do not overlap
    double m0 = n0->mass, m1 = n1->mass;
    double invMassSum = 1.0 / (m0 + m1);

    // how much to push each
    double push0 = (m1 * invMassSum) * overlap;
    double push1 = (m0 * invMassSum) * overlap;

    // move n0 back along normal, n1 forward along normal
    n0->x -= push0 * nx;
    n0->y -= push0 * ny;
    n1->x += push1 * nx;
    n1->y += push1 * ny;

    // 4) Now compute the relative velocity along that normal
    double vxRel = n1->vx - n0->vx;
    double vyRel = n1->vy - n0->vy;
    double relDotN = (vxRel * nx + vyRel * ny);

    // If they are already separating, no impulse needed
    if (relDotN > 0) {
        return true;
    }

    // 5) Compute impulse scalar with restitution
    //   j = -(1 + e) * (v_rel . n) / (1/m0 + 1/m1)
    double j = -(1.0 + particle_bounce) * relDotN / ((1.0 / m0) + (1.0 / m1));

    // 6) Apply impulse to each velocity
    double impulseX = j * nx;
    double impulseY = j * ny;

    n0->vx -= impulseX / m0;
    n0->vy -= impulseY / m0;
    n1->vx += impulseX / m1;
    n1->vy += impulseY / m1;

    return true;
}

void Sim::processCollisions()
{
    int len = particles.size();

    double max_speed = 0;
    double max_radius = 0;
    for (int i = 0; i < len; i++)
    {
        Particle& p = particles[i];

        if (p.vx > max_speed)
            max_speed = p.vx;
        else if (-p.vx > max_speed)
            max_speed = -p.vx;
        if (p.vy > max_speed)
            max_speed = p.vx;
        else if (-p.vy > max_speed)
            max_speed = -p.vy;

        if (p.r > max_radius)
            max_radius = p.r;
    }

    double scaled_max_speed = max_speed * step_seconds;
    double min_collision_cell_size = (max_radius * 2) + scaled_max_speed;

    buildUniformGrid(min_collision_cell_size, collision_grid);


    for (int i = 0; i < collision_substeps; i++)
    {
        if (optimize_collisions)
        {
            // Optimized
            forEachCellAndNeighbors(collision_grid, [&](Particle* a, Particle* b)
            {
                checkAndResolveCollision(a, b);
            });
        }
        else
        {
            // Unoptimized
            for (int j = 0; j < len; j++)
            {
                for (int q = j + 1; q < len; q++)
                {
                    Particle* a = &particles[j];
                    Particle* b = &particles[q];
                    checkAndResolveCollision(a, b);
                }
            }
        }
    }
}

void Sim::draw(QNanoPainter* p)
{
    double left = -world_size / 2;
    double top = -world_size / 2;
    double right = world_size / 2;
    double bottom = world_size / 2;

    density_bmp.draw(p, 
        cam.toStage(left, top), 
        cam.toStageSize(world_size, world_size)
    );

    p->setFillStyle("#00ffff");
    p->setTextAlign(QNanoPainter::TextAlign::ALIGN_CENTER);

    int len = particles.size();
    
    /*double max_pressure = 0;
    for (int i = 0; i < len; i++)
    {
        Particle& n = particles[i];
        if (n.pressure > max_pressure)
            max_pressure = n.pressure;
    }*/

    for (int i = 0; i < len; i++)
    {
        Particle& n = particles[i];
        

        //double x0 = n.x - cos(n.angle) * 10;
        //double y0 = n.y - sin(n.angle) * 10;
        //double x1 = n.x + cos(n.angle) * 10;
        //double y1 = n.y + sin(n.angle) * 10;

        //int pressure_heat = (n.pressure / max_pressure) * 255;

        p->setFillStyle({255,255,255});
        p->beginPath();
        //p->moveTo(x0, y0);
        //p->lineTo(x1, y1);
        p->circle(cam.toStage(n.x, n.y), n.r * cam.zoom_x);
        p->fill();

        
    }

    if (draw_gravity_grid)
        gravity_grid_near.draw(p, cam, "#7f0000");
    if (draw_collision_grid)
    collision_grid.draw(p, cam, "#007f00");

    /*p->setStrokeStyle({255,255,255,100});
    p->setLineWidth(1);
    p->beginPath();
    for (int i = 0; i < len; i++)
    {
        Particle& n = particles[i];
        if (n.path.size() == 0)
            continue;

        p->moveTo(n.path[0]);
        for (int j = 0; j < n.path.size(); j++)
        {
            p->lineTo(n.path[j]);
        }
    }
    p->stroke();*/

    /*p->setFillStyle("#ff00ff");
    for (int i = 0; i < len; i++)
    {
        Particle& n = particles[i];
        QString id = QString("n%1").arg(i);
        p->fillText(id, n.x, n.y - 10);
    }*/

    int ty = 5;
    int row_h = 18;

    p->setTextAlign(QNanoPainter::TextAlign::ALIGN_LEFT);
    p->setTextBaseline(QNanoPainter::TextBaseline::BASELINE_TOP);
    p->setFillStyle("#ffffff");

    p->fillText("Time elapsed: " + normalizeSeconds(time_elapsed), 5, ty);
    ty += row_h;

    p->fillText(QString("Particles: %1").arg(particles.size()), 5, ty);

    ty += row_h;
    p->fillText(QString("Frame dt: %1").arg(frame_dt), 5, ty);
}

void Sim::mouseWheel(int delta)
{
    Simulation::mouseWheel(delta);
}

SIM_END
