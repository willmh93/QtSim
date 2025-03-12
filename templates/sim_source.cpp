#include "{CLASS_NAME}.h"
SIM_DECLARE({CLASS_NAME}, "My Projects", "{SIM_NAME}")

/// Project ///

void {CLASS_NAME}_Project::projectAttributes(Input* input)
{{
    input->realtime_slider("Panel Count", &panel_count, 1, 16, 1);
}}

void {CLASS_NAME}_Project::projectPrepare()
{{
    auto& layout = newLayout();

    /// Create multiple instance of Scene and add to separate viewports
    create<{CLASS_NAME}_Scene>(panel_count)->mountTo(layout);

    /// Or create individual instances of Scene and add them to Layout
    //for (int i = 0; i < panel_count; ++i)
    //    layout[i]->mountScene(create<{CLASS_NAME}_Scene>());

    /// Or create a single Scene instance and view on multiple Viewports
    //auto* scene = create<{CLASS_NAME}_Scene>();
    //for (int i = 0; i < panel_count; ++i)
    //    layout << scene;
}}

/// Scene ///

void {CLASS_NAME}_Scene::sceneAttributes(Input* input)
{{
    //--- Only updated on sceneStart ---//

    //input->starting_checkbox("Starting Flag", &var1);                
    //input->starting_slider("Starting Double", &var3, 0.0, 1.0);

    //--- Updated in realtime ---//

    //input->realtime_slider("Realtime Double", &var2, 0.0, 1.0); 
    
}}

void {CLASS_NAME}_Scene::sceneStart()
{{
    /// Initialize Scene
}}

void {CLASS_NAME}_Scene::sceneDestroy()
{{
    /// Destroy Scene
}}

void {CLASS_NAME}_Scene::sceneMounted(Viewport* viewport)
{{
    /// Initialize viewport (after sceneStart)
    camera->setOriginViewportAnchor(Anchor::CENTER);
    //camera->focusWorldRect(0, 0, 300, 300);
}}

void {CLASS_NAME}_Scene::sceneProcess()
{{
    /// Process Scene update
}}

void {CLASS_NAME}_Scene::viewportProcess(Viewport* ctx)
{{
    /// Process Viewports running this Scene
}}

void {CLASS_NAME}_Scene::viewportDraw(Viewport* ctx)
{{
    /// Draw Scene to Viewport
    ctx->drawWorldAxis();
}}

/// User Interaction

void {CLASS_NAME}_Scene::mouseDown() {{}}
void {CLASS_NAME}_Scene::mouseUp() {{}}
void {CLASS_NAME}_Scene::mouseMove() {{}}
void {CLASS_NAME}_Scene::mouseWheel() {{}}

SIM_END({CLASS_NAME})