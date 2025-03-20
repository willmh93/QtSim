#include "BigBang.h"


// Create an orthographic projection matrix for 2D rendering
// Assume viewport size: width and height
QMatrix4x4 createOrthoProjectionMatrix(float width, float height) {
    QMatrix4x4 projection;
    projection.ortho(0.0f, width, height, 0.0f, -1.0f, 1.0f);  // Top-left origin (like screen coordinates)
    return projection;
}


SIM_DECLARE(BigBang, "Physics", "Space Engine", "Big Bang")

/*CHILD_SORT("Physics",
    "Space Engine",
    "Experimental"
)

CHILD_SORT("Physics", "Space Engine",
    "Big Bang",
    "Earth Moon"
)*/



void BigBang_Project::projectPrepare()
{
    auto &layout = newLayout();
    //create<BigBang_Scene>()->mountTo(layout);
    create<BigBang_Scene>()->mountTo(layout);
}

/*void BigBangScene::projectPrepare()
{

    world_size_min = 100;
    world_size_max = 10000;
    steps_per_frame = 1;
    step_seconds = 0.001;
    particle_count = 50;// 5000;
    collision_substeps = 10;
    gravity_cell_near_ratio = 0.01;
    //start_particle_radius = 0.05;
    gravity = 0.03;

    start_world_size = 5000;

    step_seconds_step = step_seconds * 0.1;
    step_seconds_min = 0;// step_seconds * 0.5;
    step_seconds_max = step_seconds * 10;

    SpaceEngineScene::projectPrepare();

    

}*/

void BigBang_Scene::sceneAttributes(Input* input)
{
    steps_per_frame = 1;
    step_seconds = 0.001;
    //particle_count = 1000;
    collision_substeps = 10;
    gravity_cell_near_ratio = 0.01;
    //start_particle_radius = 0.05;
    gravity = 0.003; // 0.03;

    //start_world_size = 5000;
    world_size = 5000;

    //input->realtime_slider("Start Zoom", &camera->targ_zoom_x, 0.1, 10);
    input->realtime_slider("Zoom", &zoom, 1, 10);
    input->realtime_slider("Particle Count", &particle_count, 10, 50000);

    SpaceEngine_Scene::sceneAttributes(input);
}

void BigBang_Scene::sceneStart()
{
    //world_size_min = 100;
    //world_size_max = 10000;
    

    //step_seconds_step = step_seconds * 0.1;
    //step_seconds_min = 0;// step_seconds * 0.5;
    //step_seconds_max = step_seconds * 10;


    SpaceEngine_Scene::sceneStart();

    auto universe_particles = PlanetGenerator::planetFromParticleCount<Particle>(0, 0, 50, 1, particle_count);
    //auto universe_particles = PlanetGenerator::randomDistributedPlanet<Particle>(0, 0, 50, particle_count, 1);

    /*std::vector<Particle> universe_particles;
    for (int i = 0; i < particle_count; i++)
    {
        double position_angle = random(-M_PI, M_PI);
        double position_dist = sqrt(random(0, 1)) * 10;

        Particle p;
        p.x = cos(position_angle) * position_dist;
        p.y = sin(position_angle) * position_dist;
        p.vx = 0;
        p.vy = 0;
        p.r = 1;
        p.mass = 1000000;
        universe_particles.push_back(p);
    }*/

    double explode_speed = 0.1;// 1000.0;
    double max_perp_speed_ratio = 1;
   // double perp_bias = 1;

    double angle_randomness = 0;// ((M_PI * 2) / 360.0) * 45.0;

    for (Particle& p : universe_particles)
    {
        double angle = atan2(p.y, p.x) + random(-angle_randomness, angle_randomness);
        //double perp_angle = angle + (M_PI / 2.0);
        double dist_ratio = sqrt(p.x * p.x + p.y * p.y) / 20.0;
        double speed = random(0, explode_speed) * (dist_ratio + 0.1);
        //double max_perp_speed = speed;
        //double perp_speed = random(-max_perp_speed, max_perp_speed) + (max_perp_speed * perp_bias);
        p.vx = cos(angle) * speed;// + cos(perp_angle) * perp_speed;
        p.vy = sin(angle) * speed;// + sin(perp_angle) * perp_speed;
    }


    addParticles(universe_particles);

    

    /*for (int i = 0; i < particle_count; i++)
    {
        double f = ((double)i) / ((double)particle_count);
        double a = f * M_PI * 2;

        double pos_angle = random(-M_PI, M_PI);
        double pos_radius = sqrt(random(0, 1)) * (world_size / 2);

        Particle n;
        n.x = cos(pos_angle) * pos_radius;
        n.y = sin(pos_angle) * pos_radius;
        n.vx = random(-start_particle_speed, start_particle_speed);
        n.vy = random(-start_particle_speed, start_particle_speed);

        double real_radius = start_particle_radius;// *0.01;// random(start_particle_radius, 10.0 * start_particle_radius);


        // Magnify simulated radius
        n.r = real_radius * particle_magnify;//

        // But use real radius to calculate mass
        n.calculateMass(real_radius, 100);
        //n.mass = 1;

        particles.emplace_back(n);
    }*/
}

/*void BigBang_Scene::initializeGL()
{
    
}*/

void BigBang_Scene::sceneProcess()
{
    SpaceEngine_Scene::sceneProcess();

    double x1 = std::numeric_limits<double>::max();
    double x2 = std::numeric_limits<double>::lowest();
    double y1 = std::numeric_limits<double>::max();
    double y2 = std::numeric_limits<double>::lowest();

    for (const Particle &p: particles)
    {
        if (p.x < x1) x1 = p.x;
        if (p.x > x2) x2 = p.x;
        if (p.y < y1) y1 = p.y;
        if (p.y > y2) y2 = p.y;
    }


    /*if (!focus_rect_initialized)
    {
        focus_rect_initialized = true;
        focus_rect.set(x1, y1, x2, y2);
    }
    else
    {
        focus_rect = lerpRect(focus_rect, FRect(x1, y1, x2, y2), 0.1);
    }

    ctx->camera.focusWorldRect(focus_rect);*/
}

void BigBang_Scene::sceneMounted(Viewport* ctx)
{
    camera->setOriginViewportAnchor(Anchor::CENTER);

    camera->focusWorldRect(boundaries(particles).scaled(zoom), false);
    focus_rect.set(boundaries(particles).scaled(zoom));
}



void BigBang_Scene::loadShaders()
{
    shader = std::make_unique<QOpenGLShaderProgram>();
    shader->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/particle.vert");
    shader->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/particle.frag");
    if (!shader->link())
    {
        qDebug() << "Shader linking failed:" << shader->log();
        return;
    }
}

void BigBang_Scene::viewportProcess(Viewport* ctx)
{
    //focus_rect = lerpRect(focus_rect, boundaries(particles).scaled(zoom), 0.02);
    //camera->focusWorldRect(focus_rect, false);
}

void BigBang_Scene::viewportDraw(Viewport* ctx)
{
    
    // Particle glow
    /*{
        QOpenGLExtraFunctions *glF = ctx->beginGL();

        shader->bind();
        shader->setUniformValue("transform", ctx->modelViewMatrix());// ctx->modelViewMatrix());
        shader->setUniformValue("particleSize", 100.0f * (float)ctx->camera.zoom_x);  // Or per-particle if you want
        shader->setUniformValue("glowColor", QVector4D(1.0f, 0.8f, 0.0f, 0.5f));
        shader->setUniformValue("glowStrength", 5.0f);

        // Upload particle positions (x, y)
        std::vector<float> vertexData;
        for (const auto& p : particles) {
            vertexData.push_back(p.x);
            vertexData.push_back(p.y);
        }
        
        // Blending
        glF->glEnable(GL_BLEND);
        glF->glBlendFunc(GL_SRC_ALPHA, GL_ONE);

        glF->glEnable(GL_POINT_SPRITE);
        glF->glEnable(GL_PROGRAM_POINT_SIZE);

        // Assign particle data to buffer
        GLuint vbo;
        glF->glGenBuffers(1, &vbo);
        glF->glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glF->glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(float), vertexData.data(), GL_STATIC_DRAW);
        glF->glEnableVertexAttribArray(0);
        glF->glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

        // Draw
        glF->glDrawArrays(GL_POINTS, 0, particles.size());

        // Cleanup
        glF->glDisableVertexAttribArray(0);
        glF->glBindBuffer(GL_ARRAY_BUFFER, 0);
        shader->release();

        ctx->endGL();
    }
    //ctx->painter->reset();

    SpaceEngine_Scene::viewportDraw(ctx);

    // Particle glow
    {
        QOpenGLExtraFunctions* glF = ctx->beginGL();

        shader->bind();
        shader->setUniformValue("transform", ctx->modelViewMatrix());// ctx->modelViewMatrix());
        shader->setUniformValue("particleSize", 30.0f * (float)ctx->camera.zoom_x);  // Or per-particle if you want
        shader->setUniformValue("glowColor", QVector4D(0.0f, 1.0f, 1.0f, 1.0f));
        shader->setUniformValue("glowStrength", 5.0f);

        // Upload particle positions (x, y)
        std::vector<float> vertexData;
        for (const auto& p : particles) {
            vertexData.push_back(p.x);
            vertexData.push_back(p.y);
        }

        // Blending
        glF->glEnable(GL_BLEND);
        glF->glBlendFunc(GL_SRC_ALPHA, GL_ONE);

        glF->glEnable(GL_POINT_SPRITE);
        glF->glEnable(GL_PROGRAM_POINT_SIZE);

        // Assign particle data to buffer
        GLuint vbo;
        glF->glGenBuffers(1, &vbo);
        glF->glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glF->glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(float), vertexData.data(), GL_STATIC_DRAW);
        glF->glEnableVertexAttribArray(0);
        glF->glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

        // Draw
        glF->glDrawArrays(GL_POINTS, 0, particles.size());

        // Cleanup
        glF->glDisableVertexAttribArray(0);
        glF->glBindBuffer(GL_ARRAY_BUFFER, 0);
        shader->release();

        ctx->endGL();
    }*/
}

SIM_END(BigBang)