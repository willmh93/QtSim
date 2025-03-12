#include "Multithreading.h"
SIM_DECLARE(Multithreading, "Framework Tests", "Multithreading")

/// Project ///

void Multithreading_Project::projectAttributes(Input* options)
{
    options->realtime_slider("Panel Count", &panel_count, 1, 16, 1);
}

void Multithreading_Project::projectPrepare()
{
    auto& layout = newLayout();

    /// Create multiple instance of Scene and add to separate viewports
    create<Multithreading_Scene>(panel_count)->mountTo(layout);

    /// Or create individual instances of Scene and add them to Layout
    //for (int i = 0; i < panel_count; ++i)
    //    create<Multithreading_Scene>()->mountTo(layout[i]);

    /// Or create a single Scene instance and view on multiple Viewports
    //auto* scene = create<Multithreading_Scene>();
    //for (int i = 0; i < panel_count; ++i)
    //    layout << scene;
}

/// Scene ///

void Multithreading_Scene::sceneAttributes(Input* options)
{
    ///--- Only updated on sceneStart ---///
    thread_count = 4;// QThread::idealThreadCount();

    //options->starting_checkbox("Starting Flag", &var1);                
    options->starting_slider("Thread Count", &thread_count, 1, 32);
    options->realtime_slider("Acceleration", &acceleration, 0, 0.000002);

    ///--- Updated in realtime ---///

    //options->realtime_slider("Realtime Double", &var2, 0.0, 1.0, 0.1); 
    
}

void Multithreading_Scene::sceneStart()
{
    /// Initialize Scene
    
    pool.setMaxThreadCount(thread_count);

    for (int i = 0; i < 10000; i++)
    {
        particles.push_back({
            random(-200, 200),
            random(-200, 200),
            random(-2, 2),
            random(-2, 2)
        });
    }
}

void Multithreading_Scene::sceneDestroy()
{
    /// Destroy Scene
}

void Multithreading_Scene::sceneMounted(Viewport* viewport)
{
    /// Initialize viewport (after sceneStart)
    camera->setOriginViewportAnchor(Anchor::CENTER);
    //camera->focusWorldRect(0, 0, 300, 300);
}

void Multithreading_Scene::computeForces()
{
    std::vector<QFuture<void>> futures(thread_count);
    auto ranges = splitRanges(particles.size(), thread_count);

    for (int ti = 0; ti < thread_count; ti++)
    {
        auto& range = ranges[ti];
        futures[ti] = QtConcurrent::run([this, ti, range]()
        {
            for (int i = range.first; i < range.second; i++)
            {
                auto& p = particles[i];
                p.fx = 0;
                p.fy = 0;
            }

            int particle_count = particles.size();
            for (int i = range.first; i < range.second; i++)
            {
                auto& p1 = particles[i];
                for (int j = 0; j < particle_count; j++)
                {
                    auto& p2 = particles[j];

                    p1.fx += (p2.x - p1.x) * acceleration;
                    p1.fy += (p2.y - p1.y) * acceleration;
                    //p2.fx += (p1.x - p2.x) * 0.000001;
                    //p2.fy += (p1.y - p2.y) * 0.000001;
                }
            }
        });
    }

    for (auto& future : futures)
        future.waitForFinished();
}

void Multithreading_Scene::applyForces()
{
    std::vector<QFuture<void>> futures(thread_count);
    auto ranges = splitRanges(particles.size(), thread_count);

    for (int ti = 0; ti < thread_count; ti++)
    {
        auto& range = ranges[ti];
        futures[ti] = QtConcurrent::run([this, ti, range]()
        {
            for (int i = range.first; i < range.second; i++)
            {
                auto& p = particles[i];
                p.vx += p.fx;
                p.vy += p.fy;
            }
        });
    }

    for (auto& future : futures)
        future.waitForFinished();
}

void Multithreading_Scene::updatePositions()
{
    std::vector<QFuture<void>> futures(thread_count);
    auto ranges = splitRanges(particles.size(), thread_count);

    for (int ti = 0; ti < thread_count; ti++)
    {
        auto& range = ranges[ti];
        futures[ti] = QtConcurrent::run([this, ti, range]()
        {
            for (int i = range.first; i < range.second; i++)
            {
                auto& p = particles[i];
                p.x += p.vx;
                p.y += p.vy;
            }
        });
    }

    for (auto& future : futures)
        future.waitForFinished();
}

void Multithreading_Scene::sceneProcess()
{
    /// Process Scene update
    computeForces();
    applyForces();
    updatePositions();
}

void Multithreading_Scene::viewportProcess(Viewport* ctx)
{
    /// Process Viewports running this Scene
}

void Multithreading_Scene::viewportDraw(Viewport* ctx)
{
    /// Draw Scene to Viewport
    ctx->drawWorldAxis();

    // Draw particles
    ctx->setFillStyle(255, 0, 255);
    ctx->beginPath();
    for (Particle& p : particles)
    {
        ctx->circle(p.x, p.y, 3);
    }
    ctx->fill();

    auto ranges = splitRanges(particles.size(), thread_count);

    for (int ti = 0; ti < thread_count; ti++)
    {
        int count = ranges[ti].second - ranges[ti].first;
        ctx->print() << "Thread " << ti << ": " << count << " particles\n";
    }
    ctx->print() << "dt: " << scene_dt(20) << "\n\n";
}

/// User Interaction

void Multithreading_Scene::mouseDown() {}
void Multithreading_Scene::mouseUp() {}
void Multithreading_Scene::mouseMove() {}
void Multithreading_Scene::mouseWheel() {}

SIM_END(Multithreading)