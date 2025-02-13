#pragma once
#include "Simulation.h"
#include <QOpenGLFunctions>

SIM_BEG(SpaceEngine)

enum BondState
{
    ESCAPING,
    RESTING,
    BONDED
};

struct Particle;
struct Bond
{
    Particle* a;
    Particle* b;
    BondState state;
};

struct Cluster
{
    std::vector<Particle*> particles;
};

struct Particle : public Serializable
{
    Cluster* cluster;
    std::vector<Bond*> bonds;

    double x, y; // m
    double vx, vy; // m/s^2
    double mass;// = 1; // Kg
    double r;// = 5;
    //double pressure = 0;
    //double angle;
    //double spin;
    //vector<Point> path;

    void calculateMass(double real_radius, double density)
    {
        double volume = (4.0 / 3.0) * M_PI * pow(real_radius, 3);
        mass = density * volume;
    }

    void cache(SerializeContext* cache)
    {
        cache->apply(x);
        cache->apply(y);
        cache->apply(vx);
        cache->apply(vy);
    }
};

/*struct PressureCmp
{
    bool operator()(const Particle* a, const Particle* b)
    {
        return a->pressure > b->pressure;
    }
};*/

struct CellCoord
{
    int cx, cy;
    
    // Hash/equals so we can put CellCoord in an unordered_map
    bool operator==(const CellCoord& rhs) const 
    {
        return (cx == rhs.cx && cy == rhs.cy);
    }
};

struct CellData
{
    std::vector<Particle*> particles;
    bool initialized = false;
    double cx;
    double cy;
    double total_mass;
    double com_x;
    double com_y;
};

/*struct CellCoordHash
{
    std::size_t operator()(const CellCoord& c) const 
    {
        // A typical 2D -> 1D integer hash
        // Could be improved if collisions, but fine for demonstration
        // For example (cx * someLargePrime) ^ (cy * anotherLargePrime)
        auto h1 = std::hash<long long>()(((long long)c.cx << 32) ^ (c.cx >> 32));
        auto h2 = std::hash<long long>()(((long long)c.cy << 32) ^ (c.cy >> 32));
        return h1 ^ (h2 + 0x9e3779b97f4a7c15ULL + (h1 << 6) + (h1 >> 2));
    }
};*/
struct CellCoordHash
{
    std::size_t operator()(const CellCoord& c) const
    {
        constexpr uint64_t prime1 = 73856093ULL;  // Large prime
        constexpr uint64_t prime2 = 19349663ULL;  // Another large prime

        return (static_cast<uint64_t>(c.cx) * prime1) ^
               (static_cast<uint64_t>(c.cy) * prime2);
    }
};

/// Convert (x,y) -> integer grid cell coords (cx,cy)
inline CellCoord computeCellCoord(double x, double y, double cellSize)
{
    int cx = static_cast<int>(floor(x / cellSize));
    int cy = static_cast<int>(floor(y / cellSize));
    return { cx, cy };
}

struct ParticleGrid : public std::unordered_map<CellCoord, CellData, CellCoordHash>
{
    double cell_size;
    bool is_gravity;

    void draw(DrawingContext* ctx, Camera &cam, const char *color)
    {
        ctx->setStrokeStyle(color);
        ctx->setFillStyle({ 0,0,255 });
        ctx->setLineWidth(1);

        double cell_r = cell_size / 2;
        //for (auto& item : this)
        for (auto it=begin(); it!=end(); ++it)
        {
            CellData& cellData = it->second;
            double cell_cx = cellData.cx;
            double cell_cy = cellData.cy;
            double cell_x0 = cell_cx - cell_r;
            double cell_y0 = cell_cy - cell_r;
            double cell_x1 = cell_cx + cell_r;
            double cell_y1 = cell_cy + cell_r;
            Vec2 p0 = cam.toStage(cell_x0, cell_y0);
            Vec2 p1 = cam.toStage(cell_x1, cell_y1);
            ctx->strokeRect(p0.x, p0.y, p1.x - p0.x, p1.y - p0.y);
       
            if (is_gravity)
            {
                ctx->beginPath();
                Vec2 com_p = cam.toStage(cellData.com_x, cellData.com_y);
                Vec2 com_s = cam.toStageSize(cell_size / 30.0, 0);
                ctx->circle(com_p.x, com_p.y, com_s.x);
                ctx->fill();
            }
        }

    }
};

struct SpaceEngineInstance : public SimulationInstance
{
    //Camera cam;

    // Constants
    const double G = 6.67430e-11;
    const double kilometer = 1000;
    const double gigameter = 1.0e+9;

    // Starting values
    int density_bmp_size = 10;
    //double start_world_size = 100;
    //double start_particle_speed = 1;
    //double start_particle_radius = 1;
    double particle_bounce = 0.9;
    double particle_magnify = 1;
    double gravity = 0.1;

    int gravity_cell_near_grid_radius = 1;
    double gravity_cell_near_ratio = 0.05;
    //double gravity_cell_far_ratio = 0.05;
    //double collision_cell_size = start_particle_radius * 10;
    
    // Ranges

    //double step_seconds_min = 1;
    //double step_seconds_max = 10;
    //double step_seconds_step = 1;
    //double world_size_min = 100;
    //double world_size_max = 1000;
    //double world_size_step = 100;
    //double particle_radius_min = 1;
    //double particle_radius_max = 10;
    //double particle_radius_step = 1;

    //double collision_cell_size_min = collision_cell_size * 0.1;
    //double collision_cell_size_max = collision_cell_size * 2;
    //double collision_cell_size_step = collision_cell_size * 0.1;




    double world_size = 100;// start_world_size;
    

    double hex_lattice_density = 0.952;
    //double hex_lattice_density_scalar = 1 / hex_lattice_density;

    //vector<Particle> start_particles;
    vector<Particle> particles;


    bool draw_gravity_grid = false;
    bool draw_collision_grid = false;

    ParticleGrid gravity_grid_near;
    //ParticleGrid gravity_grid_far;
    ParticleGrid collision_grid;

    // Density map
    Bitmap density_bmp;
    double bmp_scale;

    double step_seconds = 1.0/60.0;
    double time_elapsed = 0;

    int steps_per_frame = 5;
    int collision_substeps = 5;

    bool optimize_gravity = true;
    bool optimize_collisions = true;

    void instanceAttributes(Options* options) override;
    void start() override;
    void destroy() override;
    void processScene() override;
    void draw(Panel* ctx) override;

    

    // Gravity
    inline void processCellPairGravity(CellData &c0, CellData& c1);
    inline void processParticlePairGravity(Particle *n0, Particle *n1);
    void processGravity();

    // Collisions
    inline bool checkAndResolveCollision(Particle* n0, Particle* n1);
    void processCollisions();

    // Particle grid
    void buildUniformGrid(double cellSize, ParticleGrid& grid);

    /*template <typename Func>
    void forEachCellAndNeighbors(
        std::unordered_map<CellCoord, CellData, CellCoordHash> &grid,
        Func processPair,
        int r=1)
    {
        for (auto& kv : grid)
        {
            const CellCoord& cell = kv.first;
            CellData& cellData = kv.second;

            // For each neighbor cell (including the cell itself)
            for (int ny = -r; ny <= r; ny++)
            {
                for (int nx = -r; nx <= r; nx++)
                {
                    CellCoord neighborCell{ cell.cx + nx, cell.cy + ny };

                    auto it = grid.find(neighborCell);
                    if (it == grid.end())
                        continue;

                    CellData& neighborCellData = it->second;

                    if (nx == 0 && ny == 0)
                    {
                        // same cell => check i<j
                        auto& cellParticles = cellData.particles;
                        size_t len = cellParticles.size();
                        for (size_t i = 0; i < len; i++)
                        {
                            Particle* n0 = cellParticles[i];
                            for (size_t j = i + 1; j < len; j++)
                            {
                                Particle* n1 = cellParticles[j];
                                processPair(n0, n1);
                            }
                        }
                    }
                    else if (nx >= 0 && ny >= 0)
                    {
                        // "forward" neighbors only => avoid double checks
                        for (Particle* n0 : cellData.particles)
                        {
                            for (Particle* n1 : neighborCellData.particles)
                            {
                                processPair(n0, n1);
                            }
                        }
                    }
                }
            }
        }
    }*/


    template <typename Func>
    void forEachCellAndNeighbors(
        std::unordered_map<CellCoord, CellData, CellCoordHash>& grid,
        Func processCellPair,
        int r = 1)
    {
        for (auto& kv : grid)
        {
            const CellCoord& cell = kv.first;
            CellData& cellData = kv.second;

            // For each neighbor cell (including the cell itself)
            for (int ny = -r; ny <= r; ny++)
            {
                for (int nx = -r; nx <= r; nx++)
                {
                    CellCoord neighborCell{ cell.cx + nx, cell.cy + ny };

                    auto it = grid.find(neighborCell);
                    if (it == grid.end())
                        continue;

                    CellData& neighborCellData = it->second;

                    if (nx == 0 && ny == 0)
                    {
                        processCellPair(cellData, neighborCellData);
                    }
                    else if (nx > 0 || (nx == 0 && ny >= 0))
                    {
                        processCellPair(cellData, neighborCellData);
                    }
                }
            }
        }
    }

    template <typename Func>
    void forEachCellAndNeighborsParticles(
        std::unordered_map<CellCoord, CellData, CellCoordHash>& grid,
        Func processPair,
        int r = 1)
    {
        for (auto& kv : grid)
        {
            const CellCoord& cell = kv.first;
            CellData& cellData = kv.second;

            // For each neighbor cell (including the cell itself)
            for (int ny = -r; ny <= r; ny++)
            {
                for (int nx = -r; nx <= r; nx++)
                {
                    CellCoord neighborCell{ cell.cx + nx, cell.cy + ny };

                    auto it = grid.find(neighborCell);
                    if (it == grid.end())
                        continue;

                    CellData& neighborCellData = it->second;

                    if (nx == 0 && ny == 0)
                    {
                        // Same cell => check i < j to avoid duplicate pairs
                        auto& cellParticles = cellData.particles;
                        size_t len = cellParticles.size();
                        for (size_t i = 0; i < len; i++)
                        {
                            Particle* n0 = cellParticles[i];
                            for (size_t j = i + 1; j < len; j++)
                            {
                                Particle* n1 = cellParticles[j];
                                processPair(n0, n1);
                            }
                        }
                    }
                    else if (nx > 0 || (nx == 0 && ny >= 0))
                    {
                        // "Forward" neighbors only => avoid double checks
                        for (Particle* n0 : cellData.particles)
                        {
                            for (Particle* n1 : neighborCellData.particles)
                            {
                                processPair(n0, n1);
                            }
                        }
                    }
                }
            }
        }
    }

    // Returns a COPY
    Particle newParticle(
        double x,
        double y,
        double real_radius,
        double real_density
    )
    {
        Particle n;
        n.x = x;
        n.y = y;
        n.vx = 0;
        n.vy = 0;

        // Magnify simulated radius
        n.r = real_radius * particle_magnify;

        // But use real radius to calculate mass
        n.calculateMass(real_radius, real_density);

        return n;
    }

    std::vector<Particle> newPlanetFromParticleSize(
        double cx,
        double cy,
        double planet_radius,
        double density,
        double particle_radius)
    {
        double planet_volume = (4.0 / 3.0) * M_PI * pow(planet_radius, 3);
        double planet_target_mass = density * planet_volume;

        double dx = 2.0 * particle_radius;            // horizontal spacing
        double dy = sqrt(3.0) * particle_radius;      // vertical spacing

        double maxAllowedRadius = planet_radius;// -particle_radius;

        int maxRow = static_cast<int>(std::ceil((planet_radius * 2.0) / dx)) + 2;
     
        std::vector<Particle> planet_particles;

        for (int row = -maxRow; row <= maxRow; ++row)
        {
            double y = row * dy;

            // Shift in x for every other row (to achieve the "hex" staggering)
            double xOffset = (row % 2 == 0) ? 0.0 : particle_radius;

            int maxCol = static_cast<int>(std::ceil((planet_radius * 2.0) / dx)) + 2;

            for (int col = -maxCol; col <= maxCol; ++col)
            {
                double x = xOffset + col * dx;

                // Check if (x, y) is inside the circle of radius maxAllowedRadius
                double dist = std::sqrt(x * x + y * y);
                if (dist <= maxAllowedRadius)
                {
                    // Accept this position
                    Particle p;
                    p.x = cx + x;
                    p.y = cy + y;
                    p.vx = 0;
                    p.vy = 0;
                    p.r = particle_radius;
                    planet_particles.push_back(p);
                }
            }
        }

        double particle_count = planet_particles.size();
        double particle_mass = planet_target_mass / particle_count;
        for (Particle& p : planet_particles)
        {
            p.mass = particle_mass;
        }

        return planet_particles;
    }

    std::vector<Particle> newPlanetFromParticleCount(
        double cx,
        double cy,
        double planet_radius,
        double density,
        int subparticle_count)
    {
        //density *= hex_lattice_density_scalar;

        double planet_volume = (4.0 / 3.0) * M_PI * pow(planet_radius, 3);
        double planet_target_mass = density * planet_volume;

        double particle_radius = particleRadiusForPlanet(planet_radius, subparticle_count);
        return newPlanetFromParticleSize(cx, cy, planet_radius, density, particle_radius);
    }

    double particleRadiusForPlanet(double planet_radius, int subparticle_count)
    {
        return (hex_lattice_density * (planet_radius / sqrt((double)subparticle_count)));
    }

    void addParticles(const Particle& p)
    {
        particles.push_back(p);
    }

    void addParticles(const std::vector<Particle>& _particles)
    {
        for (const Particle& p : _particles)
            particles.push_back(p);
    }

    double sumMass(const std::vector<Particle>& _particles)
    {
        double sum = 0;
        for (const Particle& p : _particles)
            sum += p.mass;
        return sum;
    }

};

struct SpaceEngine : public Simulation
{
    void prepare() override;
};

SIM_END
