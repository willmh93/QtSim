#include "GLCube.h"
SIM_DECLARE(GLCube, "Framework Tests", "FBO Surfaces")

/// Project ///

void GLCube_Project::projectAttributes()
{
    //input->realtime_slider("Panel Count", &panel_count, 1, 16, 1);

    ImGui::SliderInt("Panel Count", Initial(&panel_count), 1, 16);
}

void GLCube_Project::projectPrepare()
{
    auto& layout = newLayout();
    create<GLCube_Scene>(panel_count)->mountTo(layout);
}

/// Scene ///





void GLCube_Scene::sceneAttributes()
{
    //input->realtime_slider("Camera Rotatation", &camera_rotation, 0.0, M_PI * 2.0, 0.0001);

    ImGui::SliderFloat("Rotation", Initial(&camera_rotation), 0.0f, (float)M_PI * 2.0f);

    auto rotation_id = QString("float%0").arg(sceneIndex()).toStdString().c_str();

    ImGui::SliderFloat(rotation_id, &camera_rotation, 0.0f, (float)M_PI * 2.0f);
}

void GLCube_Scene::sceneStart()
{
    /// Initialize Scene
    //bmp = new GLBitmap();
    bmp.create(400, 400);
}

void GLCube_Scene::sceneDestroy()
{
    /// Destroy Scene
}

void GLCube_Scene::sceneMounted(Viewport* viewport)
{
    /// Initialize viewport (after sceneStart)
    camera->setOriginViewportAnchor(Anchor::CENTER);
    // 
    camera->focusWorldRect(-600, -600, 600, 600);
}

//int i = 0;
void GLCube_Scene::sceneProcess()
{
    /// Process Scene update
    
    /*for (int y = 0; y < bmp.height(); y++)
    {
        for (int x = 0; x < bmp.width(); x++)
        {
            bmp.setPixel(x, y, i % 255, 0, 0, 255);
            i++;
        }
    }*/
}

void GLCube_Scene::initGL()
{
    // Vertex Shader
    const char* vertexShaderSrc = R"(
        #version 330 core
        layout(location = 0) in vec3 position;
        layout(location = 1) in vec3 color;
        out vec3 fragColor;
        uniform mat4 modelViewProjection;
        void main() {
            fragColor = color;
            gl_Position = modelViewProjection * vec4(position, 1.0);
        }
    )";

    // Fragment Shader
    const char* fragmentShaderSrc = R"(
        #version 330 core
        in vec3 fragColor;
        out vec4 outColor;
        void main() {
            outColor = vec4(fragColor, 1.0);
        }
    )";

    // Compile shaders
    shaderProgram = new QOpenGLShaderProgram();
    shaderProgram->addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShaderSrc);
    shaderProgram->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderSrc);
    shaderProgram->link();

    // Cube vertices (positions + colors)
    float cubeVertices[] = {
        // Position            // Color
        -0.5f, -0.5f, -0.5f,   1.0f, 0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,   0.0f, 1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,   0.0f, 0.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,   1.0f, 1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,   1.0f, 0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,   0.0f, 1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,   1.0f, 1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,   0.0f, 0.0f, 0.0f
    };

    unsigned int cubeIndices[] = {
        0, 1, 2,   2, 3, 0,
        1, 5, 6,   6, 2, 1,
        5, 4, 7,   7, 6, 5,
        4, 0, 3,   3, 7, 4,
        3, 2, 6,   6, 7, 3,
        4, 5, 1,   1, 0, 4
    };

    surface = createSurface(200, 200);
    surface.bind();

    // Setup VAO, VBO, EBO
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeIndices), cubeIndices, GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    glBindVertexArray(0);

    surface.release();
}

void GLCube_Scene::viewportProcess(Viewport* ctx)
{
    /// Process Viewports running this Scene
    angle += 1.0f;
    camera->rotation = camera_rotation;
}

void GLCube_Scene::viewportDraw(Viewport* ctx)
{
    //\QOpenGLExtraFunctions glF(QOpenGLContext::currentContext());

    /// Draw Scene to Viewport
    


    //setSurfaceViewport(surface, ctx->width, ctx->height);

    
    // Set model/view/projection matricies
    QMatrix4x4 model, view, projection;
    model.rotate(angle, 1.0f, 1.0f, 0.0f);
    view.lookAt(QVector3D(0, 0, 5), QVector3D(0, 0, 0), QVector3D(0, 1, 0));
    projection.perspective(45.0f, surface.aspectRatio(), 0.1f, 50.0f);
    QMatrix4x4 mvp = projection * view * model;

    surface.bind();
    surface.clear(0, 0, 0, 0);

    shaderProgram->bind();
    shaderProgram->setUniformValue("modelViewProjection", mvp);

    glEnable(GL_DEPTH_TEST);
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    shaderProgram->release();

    //ctx->paintSurface(surface, -200, -200, 800, 800);
    //ctx->paintSurface(surface, 200, 200, 1500, 400);


    ctx->setFillStyle(0, 0, 100, 255);
    ctx->fillCheckeredGrid(-500, -500, 500, 500);
    ctx->drawSurface(surface, -500, -500, 500, 500);


    ctx->drawSurface(surface, 0, -500, 500, 500);
    ctx->setFillStyle(100, 0, 0, 255);
    ctx->fillCheckeredGrid(0, 0, 500, -500);

    surface.release();

    ctx->drawSurface(bmp, 0, 0, bmp.width(), bmp.height());
    //ctx->drawSurface(bmp, 500, 500, 5000, 5000);



    ctx->setFillStyle(255, 255, 255, 255);
    ctx->setTextAlign(TextAlign::ALIGN_CENTER);
    ctx->fillText("Offscreen Surface #1 infront", -250, -450);
    ctx->fillText("Offscreen Surface #1 behind", 250, -450);
    ctx->drawWorldAxis();

    //releaseSurface(surface);
    
    //for (int i = 0; i < 10000; i++)
    //    pixels[i] = rand() % 255;
    ctx->print() << "dt: " << this->project_draw_dt(10);
}

/// User Interaction

void GLCube_Scene::mouseDown() {}
void GLCube_Scene::mouseUp() {}
void GLCube_Scene::mouseMove() {}
void GLCube_Scene::mouseWheel() {}

SIM_END(GLCube)