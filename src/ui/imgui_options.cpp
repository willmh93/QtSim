#include "imgui_options.h"
#include "project.h"
#include "main_window.h"

void ImOptions::setCurrentProject(Project* _project)
{
    project = _project;
}

void ImOptions::initializeGL()
{
    initializeOpenGLFunctions();
    QtImGui::initialize(this);


    QObject::connect(&timer, &QTimer::timeout, this, [this]() { update(); });
    timer.start(16);

    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    // Corners
    style.WindowRounding = 8.0f;
    style.ChildRounding = 8.0f;
    style.FrameRounding = 6.0f;
    style.FramePadding = ImVec2(8.0f, 5.0f);
    style.PopupRounding = 3.0f;
    style.ScrollbarRounding = 6.0f;
    style.GrabRounding = 2.0f;
    style.TabRounding = 6.0f;

    // Colors
    colors[ImGuiCol_Text] = ImVec4(0.95f, 0.96f, 0.98f, 1.00f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.36f, 0.42f, 0.47f, 1.00f);
    colors[ImGuiCol_WindowBg] = ImVec4(0.1058f, 0.1058f, 0.1686f, 1.00f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.15f, 0.18f, 0.22f, 1.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
    colors[ImGuiCol_Border] = ImVec4(0.43f, 0.50f, 0.56f, 0.50f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

    colors[ImGuiCol_FrameBg] = ImVec4(0.30f, 0.3f, 0.3f, 1.00f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.4f, 0.4f, 0.4f, 1.00f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.5f, 0.5f, 0.5f, 1.00f);

    colors[ImGuiCol_TitleBg] = ImVec4(0.09f, 0.12f, 0.14f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.12f, 0.20f, 0.28f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.15f, 0.18f, 0.22f, 1.00f);
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
    colors[ImGuiCol_CheckMark] = ImVec4(0.28f, 0.56f, 1.00f, 1.00f);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.28f, 0.56f, 1.00f, 1.00f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.37f, 0.61f, 1.00f, 1.00f);
    colors[ImGuiCol_Button] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.28f, 0.56f, 1.00f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.37f, 0.61f, 1.00f, 1.00f);
    colors[ImGuiCol_Header] = ImVec4(0.20f, 0.25f, 0.29f, 0.55f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_Separator] = ImVec4(0.43f, 0.50f, 0.56f, 0.50f);
    colors[ImGuiCol_SeparatorHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.78f);
    colors[ImGuiCol_SeparatorActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_ResizeGrip] = ImVec4(0.26f, 0.59f, 0.98f, 0.25f);
    colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
    colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
    colors[ImGuiCol_Tab] = ImVec4(0.11f, 0.15f, 0.17f, 1.00f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.28f, 0.56f, 1.00f, 0.80f);
    colors[ImGuiCol_TabActive] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
    colors[ImGuiCol_TabUnfocused] = ImVec4(0.07f, 0.10f, 0.15f, 0.97f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.14f, 0.22f, 0.36f, 1.00f);
    //colors[ImGuiCol_DockingPreview] = ImVec4(0.26f, 0.59f, 0.98f, 0.70f);
    //colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
    colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
    colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
    colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 0.00f, 0.00f, 0.90f);
    colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
    colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
}

void ImOptions::paintGL()
{
    QtImGui::newFrame();


    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::Begin("options", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize);

    //ImGui::Text("Hello, world!");
    
    {
        //std::lock_guard<std::mutex> lock(project_mutex);
        QMutexLocker locker(&project_mutex);
        if (project)
        {
            QReadLocker readLock(&project->worker->main_window->simLock);

            // Project section
            //if (ImGui::CollapsingHeader("Project"))
            ImGui::SeparatorText("Project");
            {
                ImGui::PushID("project_section");
                project->projectAttributes();
                ImGui::PopID();
            }

            ImGui::SeparatorText("Scenes");

            // Scene sections
            Layout& layout = project->currentLayout();
            auto& scenes = layout.scenes();
            for (Scene* scene : scenes)
            {
                std::string sceneName = scene->name() + " " + std::to_string(scene->sceneIndex());
                if (ImGui::CollapsingHeader(sceneName.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
                {
                    // Allow Scene to populate inputs for section
                    ImGui::PushID((sceneName + "_section").c_str());
                    scene->sceneAttributes();
                    ImGui::PopID();
                }
            }
        }
    }

    //{
    //    static float f = 0.0f;
    //    ImGui::Text("Hello, world!");
    //    ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
    //    ImGui::ColorEdit3("clear color", (float*)&clear_color);
    //    if (ImGui::Button("ImGui Demo")) show_imgui_demo_window ^= 1;
    //    if (ImGui::Button("ImPlot Demo")) show_implot_demo_window ^= 1;
    //    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    //}

    ImGui::End();
    ImGui::PopStyleVar();

    // 2. Show another simple window, this time using an explicit Begin/End pair
    ///if (show_imgui_demo_window)
    ///{
    ///    ImGui::SetNextWindowSize(ImVec2(200, 100), ImGuiCond_FirstUseEver);
    ///    ImGui::ShowDemoWindow(&show_imgui_demo_window);
    ///}

    // 3. Show the ImGui test window. Most of the sample code is in ImGui::ShowTestWindow()
    ///if (show_implot_demo_window)
    ///{
    ///    ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiCond_FirstUseEver);
    ///    ImPlot::CreateContext();
    ///    ImPlot::ShowDemoWindow(&show_implot_demo_window);
    ///    ImPlot::DestroyContext();
    ///}

    // Do render before ImGui UI is rendered
    glViewport(0, 0, width(), height());
    glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);

    ImGui::Render();
    QtImGui::render();
}
