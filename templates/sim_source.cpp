#include "{CLASS_NAME}.h"
SIM_DECLARE({CLASS_NAME}_Project, "My Projects", "{SIM_NAME}")

/// Project ///

void {CLASS_NAME}_Project::projectAttributes(Options* options)
{{
    options->realtime_slider("Panel Count", &panel_count, 1, 16, 1);
}}

void {CLASS_NAME}_Project::projectPrepare()
{{
    auto& layout = newLayout();

    // Create separate instances of our Scene and add them to Layout
    for (int i = 0; i < panel_count; ++i)
        layout << {CLASS_NAME}_Project::createScene();

    // Or create a single instance of our Scene and view on multiple Viewports
    //auto* scene = Explosion_Project::createScene();
    //for (int i = 0; i < panel_count; ++i)
    //    layout << scene;
}}

/// Scene ///

void {CLASS_NAME}_Scene::sceneAttributes(Options* options)
{{
    // Only updated on sceneStart()

    //options->starting_checkbox("Starting Flag", &var1);                
    //options->starting_slider("Starting Double", &var3, 0.0, 1.0, 0.1);

    // Updated in realtime

    //options->realtime_slider("Realtime Double", &var2, 0.0, 1.0, 0.1); 
    
}}

void {CLASS_NAME}_Scene::sceneStart()
{{
    // Initialize Scene
}}

void {CLASS_NAME}_Scene::sceneDestroy()
{{
    // Destroy Scene
}}

void {CLASS_NAME}_Scene::sceneMounted(Viewport* viewport)
{{
    // Initialize viewport (after sceneStart)
    camera->setOriginViewportAnchor(Anchor::CENTER);
    //camera->focusWorldRect(0, 0, 300, 300);
}}

void {CLASS_NAME}_Scene::sceneProcess()
{{
    // Process Scene update
}}

void {CLASS_NAME}_Scene::viewportProcess(Viewport* ctx)
{{
    // Process Viewports running this Scene
}}

void {CLASS_NAME}_Scene::viewportDraw(Viewport* ctx)
{{
    // Draw Scene to Viewport
}}

/// User Interaction

void {CLASS_NAME}_Scene::mouseDown() {{}}
void {CLASS_NAME}_Scene::mouseUp() {{}}
void {CLASS_NAME}_Scene::mouseMove() {{}}
void {CLASS_NAME}_Scene::mouseWheel() {{}}

SIM_END({CLASS_NAME}_Project)