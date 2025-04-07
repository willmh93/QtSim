#pragma once
#include <QOpenGLWidget>
#include "imgui_widget.h"

// ImGui-based options side-panel for Simulations

class Project;

class ImOptions : public QOpenGLWidget, private QOpenGLExtraFunctions
{
    QTimer timer;

    QMutex project_mutex;
    //std::mutex project_mutex;
    Project* project = nullptr;

public:
    explicit ImOptions(QWidget* parent = nullptr) : QOpenGLWidget(parent)
    {
        setFocusPolicy(Qt::StrongFocus);
    }

    void setCurrentProject(Project* _project);

    protected:
        void initializeGL() override;
        void paintGL() override;
    private:
        bool show_imgui_demo_window = true;
        bool show_implot_demo_window = false;
        ImVec4 clear_color = ImColor(114, 144, 154);
};

