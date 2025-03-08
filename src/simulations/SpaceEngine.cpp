#include "SpaceEngine.h"
#include "Options.h"
SIM_BEG(SpaceEngine)

void SpaceEngine_Project::projectPrepare()
{
    create<SpaceEngine_Scene>(1)->mountTo(newLayout());
}

void SpaceEngine_Scene::sceneAttributes(Input* options)
{
    //camera.enable();

    //setFocusedCamera(&cam);

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


    options->realtime_checkbox("Optimize Gravity", &optimize_gravity);
    options->realtime_checkbox("Optimize Collisions", &optimize_collisions);

    options->realtime_checkbox("Draw Gravity Grid", &draw_gravity_grid);
    options->realtime_checkbox("Draw Collision Grid", &draw_collision_grid);

    options->realtime_slider("Steps Per Frame", &steps_per_frame, 1, 100);
    
    //step_seconds = 0.1;

    auto timestep_slider = options->realtime_slider("Time step (seconds)",
        &step_seconds,
        0,//step_seconds * 0.5,
        step_seconds * 10,
        step_seconds * 0.1);
        //&step_seconds_min, 
        //&step_seconds_max, 
        //&step_seconds_step);

    ///timestep_slider->label_value = [this](double seconds)
    ///{
    ///    double seconds_processed_by_second = seconds * 60;
    ///    return normalizeSeconds(seconds_processed_by_second) + " / second";
    ///};


    options->realtime_slider("Collision Substeps", &collision_substeps, 1, 20, 1);


    options->realtime_slider("Gravity Cell Ratio", &gravity_cell_near_ratio, 0.001, 0.1, 0.001);
    options->realtime_slider("Gravity Near-Distance Cells", &gravity_cell_near_grid_radius, 1, 10, 1);

    //options->slider("Gravity Cell Far-Ratio", &gravity_cell_far_ratio, 0.01, 0.5, 0.01);
    //options->slider("Collision Cell Size", &collision_cell_size, &collision_cell_size_min, &collision_cell_size_max, &collision_cell_size_step);
    //options->slider("Max Speed", &max_speed, 0.0, 1000.0, 0.1);
    //options->slider("Minimum Particle Distance", &min_particle_dist, 1.0, 50.0, 0.1);

    ///

    //options->starting_slider("World Size (gm) ", &world_size, &world_size_min, &world_size_max, &world_size_step);
}


void SpaceEngine_Scene::sceneStart()
{
    //world_size = start_world_size;
    time_elapsed = 0;

    //cam.setZoom(((double)height()) / world_size);

    //bmp_scale = world_size / density_bmp_size;
    //density_bmp.create(density_bmp_size, density_bmp_size, true);
    
    pool.setMaxThreadCount(thread_count);
    particles.clear();
    //particles = start_particles;
}

void SpaceEngine_Scene::sceneDestroy()
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



void SpaceEngine_Scene::buildUniformGrid(double cellSize, ParticleGrid& grid)
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

void SpaceEngine_Scene::processParticlePairGravity(Particle* n0, Particle* n1, bool only_n0)
{
    double dx = n1->x - n0->x;
    double dy = n1->y - n0->y;
    double r = sqrt(dx * dx + dy * dy);

    if (r < n0->r * 2)
        r = n0->r * 2;

    double force_magnitude = gravity * (n0->mass * n1->mass) / (r * r); // No r^2 because F = G_2D * m1 * m2 / r
    double fx = force_magnitude * (dx / r);
    double fy = force_magnitude * (dy / r);

    //// Compute accelerations
    //double ax = fx / n0->mass;
    //double ay = fy / n0->mass;
    //
    //// By Newton's Third Law, apply the opposite force to particle 2
    //double ax2 = -fx / n1->mass;
    //double ay2 = -fy / n1->mass;

    // Update velocities of particle 1
    n0->fx += fx;// * step_seconds;
    n0->fy += fy;// * step_seconds;

    if (!only_n0)
    {
        n1->fx -= fx;// * step_seconds;
        n1->fy -= fy;// * step_seconds;
    }
}

void SpaceEngine_Scene::processParticleRepulsion(Particle* n0, Particle* n1, bool only_n0)
{
    /*double dx = n1->x - n0->x;
    double dy = n1->y - n0->y;
    double d = sqrt(dx * dx + dy * dy);
    double collide_dist = n0->r + n1->r;

    if (d <= collide_dist)
    {
        double near_ratio = (collide_dist - d) / collide_dist;
        double impuse_x = ;
    }*/
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

void SpaceEngine_Scene::processCellPairGravity(CellData& c0, CellData& c1)
{
    //double dx = c1.cx - c0.cx;
    //double dy = c1.cy - c0.cy;
    //double r = sqrt(dx * dx + dy * dy);

    double dx = c1.com_x - c0.com_x;
    double dy = c1.com_y - c0.com_y;
    //float r = fastSqrt(dx * dx + dy * dy);
    double r = sqrt(dx * dx + dy * dy);

    double force_magnitude = gravity * (c0.total_mass * c1.total_mass) / (r * r); // No r^2 because F = G_2D * m1 * m2 / r
    double fx = force_magnitude * (dx / r);
    double fy = force_magnitude * (dy / r);

    // Compute accelerations
    double ax1 = fx / c0.total_mass;
    double ay1 = fy / c0.total_mass;

    // By Newton's Third Law, apply the opposite force to particle 2
    double ax2 = -fx / c1.total_mass;
    double ay2 = -fy / c1.total_mass;

    for (Particle* p : c0.particles)
    {
        p->vx += ax1 * step_seconds;
        p->vy += ay1 * step_seconds;
    }
    for (Particle* p : c1.particles)
    {
        p->vx += ax2 * step_seconds;
        p->vy += ay2 * step_seconds;
    }

    // Update velocities of particle 1
    /*for (Particle* p : c0.particles)
    {
        float dx = c1.com_x - p->x;
        float dy = c1.com_y - p->y;
        double r = sqrt(dx * dx + dy * dy);

        double force_magnitude = gravity * (p->mass * c1.total_mass) / (r * r); // No r^2 because F = G_2D * m1 * m2 / r
        double fx = force_magnitude * (dx / r);
        double fy = force_magnitude * (dy / r);

        // Compute accelerations
        double ax = fx / c0.total_mass;
        double ay = fy / c0.total_mass;

        p->vx += ax * step_seconds;
        p->vy += ay * step_seconds;
    }

    // Update velocities of particle 1
    for (Particle* p : c1.particles)
    {
        float dx = c0.com_x - p->x;
        float dy = c0.com_y - p->y;
        double r = sqrt(dx * dx + dy * dy);

        double force_magnitude = gravity * (p->mass * c0.total_mass) / (r * r); // No r^2 because F = G_2D * m1 * m2 / r
        double fx = force_magnitude * (dx / r);
        double fy = force_magnitude * (dy / r);

        // Compute accelerations
        double ax = fx / c1.total_mass;
        double ay = fy / c1.total_mass;

        p->vx += ax * step_seconds;
        p->vy += ay * step_seconds;
    }*/

    //for (Particle* p : c1.particles)
    //{
    //    p->vx += ax2 * step_seconds;
    //    p->vy += ay2 * step_seconds;
    //}
}

// Return true if a collision was resolved, false if not
/*bool SpaceEngineScene::checkAndResolveCollision(Particle* n0, Particle* n1)
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
}*/



bool SpaceEngine_Scene::resolveCollisionOld(Particle* n0, Particle* n1)
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
    n0->sum_ox -= push0 * nx;
    n0->sum_oy -= push0 * ny;
    n1->sum_ox += push1 * nx;
    n1->sum_oy += push1 * ny;

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

    n0->fx -= (impulseX / m0);
    n0->fy -= (impulseY / m0);
    n1->fx += (impulseX / m1);
    n1->fy += (impulseY / m1);

    return true;
}


bool SpaceEngine_Scene::resolveCollision(Particle* n0, Particle* n1)
{
    double dx = n1->x - n0->x;
    double dy = n1->y - n0->y;
    double dist2 = dx * dx + dy * dy;
    double radiusSum = n0->r + n1->r;

    if (dist2 >= radiusSum * radiusSum) {
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
    double overlap = radiusSum - dist;

    // 3) Positionally correct so circles do not overlap
    double m0 = n0->mass, m1 = n1->mass;
    double invMassSum = 1.0 / (m0 + m1);

    double correctionX = 0.5 * overlap * nx;
    double correctionY = 0.5 * overlap * ny;

    n0->sum_ox -= correctionX;
    n0->sum_oy -= correctionY;
    n1->sum_ox += correctionX;
    n1->sum_oy += correctionY;

    // how much to push each
    ///double push0 = (m1 * invMassSum) * overlap * 0.5;
    ///double push1 = (m0 * invMassSum) * overlap * 0.5;
    ///
    ///// move n0 back along normal, n1 forward along normal
    ///n0->sum_ox -= push0 * nx;
    ///n0->sum_oy -= push0 * ny;
    ///n1->sum_ox += push1 * nx;
    ///n1->sum_oy += push1 * ny;

    // 4) Now compute the relative velocity along that normal
    double rvx = n1->vx - n0->vx;
    double rvy = n1->vy - n0->vy;
    double velAlongNormal = (rvx * nx + rvy * ny);

    // If they are already separating, no impulse needed
    if (velAlongNormal > 0) {
        return true;
    }

    // 5) Compute impulse scalar with restitution
    //   j = -(1 + e) * (v_rel . n) / (1/m0 + 1/m1)
    //double impulseMagnitude = -(1.0 + particle_bounce) * relDotN / ((1.0 / m0) + (1.0 / m1));
    //float impulseMagnitude = -(1.0f + particle_bounce) * velAlongNormal / 2.0f;
   
    //double mult = (1.0 / ((1.0 / n0->mass) + (1.0 / n1->mass)));// / 5.0;
    double impulseMagnitude = -(1.0 + particle_bounce) * velAlongNormal / ((1.0 / n0->mass) + (1.0 / n1->mass));

    // 6) Apply impulse to each velocity
    double impulseX = impulseMagnitude  * nx;
    double impulseY = impulseMagnitude  * ny;

    n0->fx -= impulseX;//100000;
    n0->fy -= impulseY;//100000;
    n1->fx += impulseX;//100000;
    n1->fy += impulseY;//100000;
    ///n0->fx -= (impulseX / m0);
    ///n0->fy -= (impulseY / m0);
    ///n1->fx += (impulseX / m1);
    ///n1->fy += (impulseY / m1);

    return true;
}

void SpaceEngine_Scene::processCollisions()
{
    int len = particles.size();

    // Calculate appropriate cell size based on particle speed
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
        for (int i = 0; i < len; i++)
        {
            Particle& p = particles[i];
            p.sum_ox = 0;
            p.sum_oy = 0;
            p.sum_delta_vx = 0;
            p.sum_delta_vy = 0;
        }

        if (optimize_collisions)
        {
            // Optimized
            forEachCellAndNeighborsParticles(collision_grid, [&](Particle* a, Particle* b)
            {
                resolveCollision(a, b);
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
                    resolveCollision(a, b);
                }
            }

            for (int i = 0; i < len; i++)
            {
                auto& p = particles[i];
                p.x += p.sum_ox;
                p.y += p.sum_oy;
                p.sum_ox = 0;
                p.sum_oy = 0;
            }
        }

        /*for (int i = 0; i < len; i++)
        {
            Particle& p = particles[i];
            p.x += p.sum_ox;
            p.y += p.sum_oy;
            p.vx += p.sum_delta_vx;
            p.vy += p.sum_delta_vy;
        }*/
    }


}

/////////////////////////////////////////////////
//// Physics Pipeline

void SpaceEngine_Scene::processGravity()
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

        forEachCellAndNeighborsParticles(gravity_grid_near, [&](Particle* a, Particle* b)
        {
            processParticlePairGravity(a, b, false);
        }, gravity_cell_near_grid_radius);

        

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
                processParticlePairGravity(a, b, false);
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

void SpaceEngine_Scene::resetForces()
{
    int len = particles.size();
    for (int i = 0; i < len; i++)
    {
        auto& p = particles[i];
        p.fx = 0;
        p.fy = 0;
        p.sum_ox = 0;
        p.sum_oy = 0;
    }
}

void SpaceEngine_Scene::computeForces()
{
    std::vector<QFuture<void>> futures(thread_count);
    auto ranges = splitRanges(particles.size(), thread_count);

    /*
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
    */

    /*for (int ti = 0; ti < thread_count; ti++)
    {
        auto& range = ranges[ti];
        futures[ti] = QtConcurrent::run([this, ti, range]()
        {
            // For each particle range...
            int particle_count = particles.size();
            for (int i = range.first; i < range.second; i++)
            {
                // Attract each particle to EVERY other particle (besides itself)
                auto& p1 = particles[i];
                for (int j = 0; j < particle_count; j++)
                {
                    if (i == j) continue;
                    auto& p2 = particles[j];

                    //processParticlePairGravity(&p1, &p2, true);
                    //processParticleRepulsion(&p1, &p2, true);
                }
            }
        });
    }

    for (auto& future : futures)
        future.waitForFinished();*/

    int len = particles.size();
    for (int i = 0; i < len; i++)
    {
        for (int j = i + 1; j < len; j++)
        {
            Particle* a = &particles[i];
            Particle* b = &particles[j];
            processParticlePairGravity(a, b, false);
        }
    }

    for (int j = 0; j < len; j++)
    {
        for (int q = j + 1; q < len; q++)
        {
            Particle* a = &particles[j];
            Particle* b = &particles[q];
            resolveCollision(a, b);
        }
    }

    for (int i = 0; i < len; i++)
    {
        auto& p = particles[i];
        p.x += p.sum_ox;
        p.y += p.sum_oy;
        p.sum_ox = 0;
        p.sum_oy = 0;
    }
}

void SpaceEngine_Scene::applyForcesAndUpdatePositions()
{
    std::vector<QFuture<void>> futures(thread_count);
    auto ranges = splitRanges(particles.size(), thread_count);

    //double substep_step_seconds = step_seconds / steps_per_frame;

    for (int ti = 0; ti < thread_count; ti++)
    {
        auto& range = ranges[ti];
        futures[ti] = QtConcurrent::run([this, ti, range]()
        {
            for (int i = range.first; i < range.second; i++)
            {
                auto& p = particles[i];
                double ax = p.fx / p.mass;
                double ay = p.fy / p.mass;
                p.vx += ax * step_seconds;
                p.vy += ay * step_seconds;
                p.x += p.vx * step_seconds + p.sum_ox;
                p.y += p.vy * step_seconds + p.sum_oy;
            }
        });
    }

    for (auto& future : futures)
        future.waitForFinished();
}

void SpaceEngine_Scene::sceneProcess()
{
    //if (cache->missing())
    {
        for (int i = 0; i < steps_per_frame; i++)
        {
            resetForces();
            computeForces();
            applyForcesAndUpdatePositions();
        }
        time_elapsed += step_seconds * steps_per_frame;
    }

    //cache->apply(particles);


    /*vector<vector<double>> density_map;
    double min_density = std::numeric_limits<double>::max();
    double max_density = 0;

    double world_pixel_radius = bmp_scale / 2;

    int len = (int)particles.size();
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
    }*/
}

/////////////////////////////////////////////////

void SpaceEngine_Scene::viewportDraw(Viewport*ctx)
{
    double left = -world_size / 2;
    double top = -world_size / 2;
    double right = world_size / 2;
    double bottom = world_size / 2;

    //density_bmp.draw(ctx,
    //    left, top,
    //    world_size, world_size
    //);

    ctx->drawWorldAxis();

    //density_bmp.draw(p, 
    //    cam.toStage(left, top), 
    //    cam.toStageSize(world_size, world_size)
    //);

    ctx->setFillStyle("#00ffff");
    ctx->setTextAlign(TextAlign::ALIGN_CENTER);

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

        ctx->setFillStyle(255,255,255);
        ctx->beginPath();
        //ctx->moveTo(x0, y0);
        //ctx->lineTo(x1, y1);
        //ctx->circle(cam.toStage(n.x, n.y), n.r * cam.zoom_x);
        ctx->circle(n.x, n.y, n.r/* * ctx->camera.zoom_x*/);
        ctx->fill();

        
    }

    /*if (draw_gravity_grid)
        gravity_grid_near.draw(ctx, ctx->camera, "#7f0000");
    if (draw_collision_grid)
    collision_grid.draw(ctx, ctx->camera, "#007f00");

    ctx->beginPath();
    ctx->setLineWidth(10);
    ctx->setStrokeStyle({ 0,255,0,50 });
    forEachCellAndNeighbors(gravity_grid_near, [this, ctx](CellData& a, CellData& b)
    {
        Vec2 pt1 = ctx->camera.toStage(a.cx, a.cy);
        Vec2 pt2 = ctx->camera.toStage(b.cx, b.cy);
        Draw::arrow(ctx, pt1, pt2, { 0,255,0,50 });
    }, gravity_cell_near_grid_radius);

    ctx->stroke();

    ctx->beginPath();
    ctx->setStrokeStyle({ 0,0,255,50 });
    int r = gravity_cell_near_grid_radius;
    for (auto it1 = gravity_grid_near.begin(); it1 != gravity_grid_near.end(); ++it1)
    {
        CellData& a = it1->second;
        int cx = it1->first.cx;
        int cy = it1->first.cy;

        //for (auto it2 = std::next(it1); it2 != gravity_grid_near.end(); ++it2)
        for (auto it2 = gravity_grid_near.begin(); it2 != gravity_grid_near.end(); ++it2)
        {
            CellData& b = it2->second;

            int dx = it2->first.cx - cx;
            int dy = it2->first.cy - cy;
            if (dx < 0) dx = -dx;
            if (dy < 0) dy = -dy;
            if (dx <= r && dy <= r)
                continue;

            ctx->moveTo(a.cx, a.cy);
            ctx->lineTo(b.cx, b.cy);

            //Vec2 pt1 = cam.toStage(a.cx, a.cy);
            //Vec2 pt2 = cam.toStage(b.cx, b.cy);
            //p->moveTo(pt1);
            //p->lineTo(pt2);
        }
    }
    ctx->stroke();*/
    

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

    auto ranges = splitRanges(particles.size(), thread_count);

    for (int ti = 0; ti < thread_count; ti++)
    {
        int count = ranges[ti].second - ranges[ti].first;
        ctx->print() << "Thread " << ti << ": " << count << " particles\n";
    }
    ctx->print() << "Time elapsed: " << normalizeSeconds(time_elapsed) << "\n\n";
    ctx->print() << "dt: " << scene_dt(20) << "\n";

    /*camera->setTransformFilters(false, false, false);

    int ty = 5;
    int row_h = 18;

    ctx->setTextAlign(TextAlign::ALIGN_LEFT);
    ctx->setTextBaseline(TextBaseline::BASELINE_TOP);
    ctx->setFillStyle("#ffffff");

    ctx->fillText("Time elapsed: " + normalizeSeconds(time_elapsed), 5, ty);
    ty += row_h;

    ctx->fillText(QString("Particles: %1").arg(particles.size()), 5, ty);*/

    //ty += row_h;
    //ctx->fillText(QString("Frame dt: %1").arg(dt_projectProcess), 5, ty);
}


SIM_END(SpaceEngine)
