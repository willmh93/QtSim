#include "Superfluid.h"
SIM_DECLARE(Superfluid, "My Projects", "Superfluid")

/// Project ///

void Superfluid_Project::projectAttributes(Input* input)
{
    input->realtime_slider("Panel Count", &panel_count, 1, 16, 1);
}

void Superfluid_Project::projectPrepare()
{
    auto& layout = newLayout();

    /// Create multiple instance of Scene and add to separate viewports
    create<Superfluid_Scene>(panel_count)->mountTo(layout);

    /// Or create individual instances of Scene and add them to Layout
    //for (int i = 0; i < panel_count; ++i)
    //    layout[i]->mountScene(create<Superfluid_Scene>());

    /// Or create a single Scene instance and view on multiple Viewports
    //auto* scene = create<Superfluid_Scene>();
    //for (int i = 0; i < panel_count; ++i)
    //    layout << scene;
}

/// Scene ///

void Superfluid_Scene::sceneAttributes(Input* input)
{
    //--- Only updated on sceneStart ---//

    //input->starting_checkbox("Starting Flag", &var1);                
    //input->starting_slider("Starting Double", &var3, 0.0, 1.0);

    //--- Updated in realtime ---//

    //input->realtime_slider("Realtime Double", &var2, 0.0, 1.0); 
    
}

void Superfluid_Scene::sceneStart()
{
    /// Initialize Scene
}

void Superfluid_Scene::sceneDestroy()
{
    /// Destroy Scene
}

void Superfluid_Scene::sceneMounted(Viewport* viewport)
{
    /// Initialize viewport (after sceneStart)
    camera->setOriginViewportAnchor(Anchor::CENTER);
    //camera->focusWorldRect(0, 0, 300, 300);
}

void Superfluid_Scene::sceneProcess()
{
    /// Process Scene update
}

void Superfluid_Scene::loadShaders()
{
    shader = std::make_unique<QOpenGLShaderProgram>();
    shader->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/superfluid.vert");
    shader->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/superfluid.frag");
    if (!shader->link())
    {
        qDebug() << "Shader linking failed:" << shader->log();
        return;
    }
}

void Superfluid_Scene::viewportProcess(Viewport* ctx)
{
    /// Process Viewports running this Scene
    t += 1.0f / 60.0f;
}

void Superfluid_Scene::viewportDraw(Viewport* ctx)
{

    {
        //shader->setUniformValue("transform", ctx->modelViewMatrix());
        QOpenGLExtraFunctions* gl = ctx->beginGL();

        shader->bind();

        shader->setUniformValue("projection", ctx->projectionMatrix);
        shader->setUniformValue("transform", ctx->transformMatrix);

        shader->setUniformValue("iTime", t);
        shader->setUniformValue("iResolution", QSizeF(ctx->width, ctx->height));

        static GLuint vao = 0, vbo = 0;
        if (vao == 0) {
            gl->glGenVertexArrays(1, &vao);
            gl->glGenBuffers(1, &vbo);

            gl->glBindVertexArray(vao);
            gl->glBindBuffer(GL_ARRAY_BUFFER, vbo);

            float w = (float)ctx->width;
            float h = (float)ctx->height;

            static const GLfloat vertices[] = {
                0, 0,
                w, 0,
                0, h,
                0, h,
                w, 0,
                w, h
            };

            /*static const GLfloat vertices[] = {
                -1.0f, -1.0f, 0.0f, 0.0f,  // Bottom-left
                 1.0f, -1.0f, 1.0f, 0.0f,  // Bottom-right
                -1.0f,  1.0f, 0.0f, 1.0f,  // Top-left
                -1.0f,  1.0f, 0.0f, 1.0f,  // Top-left
                 1.0f, -1.0f, 1.0f, 0.0f,  // Bottom-right
                 1.0f,  1.0f, 1.0f, 1.0f   // Top-right
            };*/


            gl->glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
            gl->glEnableVertexAttribArray(0);
            gl->glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), nullptr);

            gl->glBindBuffer(GL_ARRAY_BUFFER, 0);
            gl->glBindVertexArray(0);
        }

        // Now draw the quad
        gl->glBindVertexArray(vao);
        gl->glDrawArrays(GL_TRIANGLES, 0, 6);
        gl->glBindVertexArray(0);

        
        shader->release();

        ctx->endGL();
    }

    /// Draw Scene to Viewport
    ctx->drawWorldAxis();

}

/// User Interaction

void Superfluid_Scene::mouseDown() {}
void Superfluid_Scene::mouseUp() {}
void Superfluid_Scene::mouseMove() {}
void Superfluid_Scene::mouseWheel() {}

SIM_END(Superfluid)