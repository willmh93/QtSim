/*
 * This file is part of QtSim
 *
 * Copyright (C) 2025 William Hemsworth
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "QtSim.h"
#include <QDebug>

QtSim::QtSim(QWidget *parent)
    : QMainWindow(parent)
{
    // Main Menu
    /*{
        QMenuBar* menuBar = new QMenuBar(this);
        setMenuBar(menuBar);

        menuBar->setStyleSheet("color: rgb(255,255,255);");

        // Create File menu
        QMenu* fileMenu = menuBar->addMenu("Project");

        // Add actions to the File menu
        //QAction* newAction = new QAction("New", this);
        //QAction* openAction = new QAction("Open", this);
        QAction* exitAction = new QAction("Exit", this);

        //fileMenu->addAction(newAction);
        //fileMenu->addAction(openAction);
        fileMenu->addSeparator(); // Optional: Add a separator
        fileMenu->addAction(exitAction);

        // Connect actions to slots
        //connect(newAction, &QAction::triggered, this, &QtSim::onSimSelector);
        connect(exitAction, &QAction::triggered, this, &QtSim::close);

        // Create Help menu
        QMenu* helpMenu = menuBar->addMenu("Help");

        // Add About action to Help menu
        QAction* aboutAction = new QAction("About", this);
        helpMenu->addAction(aboutAction);

        connect(aboutAction, &QAction::triggered, this, &QtSim::onAbout);
    }*/

    // Status bar
    {
        QStatusBar* statusBar = new QStatusBar(this);
        statusBar->setStyleSheet("color: rgb(255,255,255);");

        statusBar->showMessage("No project active");
        setStatusBar(statusBar);
    }

    active_sim_uid = -1;
    project = nullptr;

    // Create the main splitter
    QSplitter* mainSplitter = new QSplitter(Qt::Horizontal, this);

    // Add two OpenGL viewports
    options = new Options(this);
    canvas = new Canvas2D(this);

    QVBoxLayout* simToolbarLayout = new QVBoxLayout(this);
    toolbar = new Toolbar(this);
    simToolbarLayout->setSpacing(0);
    simToolbarLayout->setContentsMargins(0, 0, 0, 0);
    simToolbarLayout->addWidget(toolbar);
    simToolbarLayout->addWidget(canvas);

    QWidget* simToolbarWidget = new QWidget();
    simToolbarWidget->setLayout(simToolbarLayout);

    mainSplitter->addWidget(options);
    mainSplitter->addWidget(simToolbarWidget);

    options->setSizePolicy(QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Minimum);
    mainSplitter->setSizePolicy(QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Minimum);

    // Set initial sizes of the splitter viewports
    options->setBaseSize({ 150, 100 });
    mainSplitter->setStretchFactor(0, 0);
    mainSplitter->setStretchFactor(1, 1);

    // Set the splitter as the central widget
    setCentralWidget(mainSplitter);

    auto& sim_factory_list = ProjectBase::projectInfoList();
    for (auto factory_info : sim_factory_list)
    {
        options->addSimListEntry(factory_info);
    }
    options->addSimListEntry(ProjectInfo({ "3D" }));
    options->addSimListEntry(ProjectInfo({ "Javascript" }));
    options->addSimListEntry(ProjectInfo({ "Python" }));

    // Handle Sim Treeview signals
    connect(options, &Options::onChooseProject, this, &QtSim::setProject);
    connect(options, &Options::onForceStartBeginProject, this, [this](int type)
    {
        setProject(type);
        startSelectedProject();
    });

    // Handle FPS change
    connect(options, &Options::onChangeFPS, this, &QtSim::setFPS);

    // Handle Toolbar signals
    connect(toolbar, &Toolbar::onPlayPressed, this, &QtSim::startSelectedProject);
    connect(toolbar, &Toolbar::onStopPressed, this, &QtSim::stopSelectedProject);
    connect(toolbar, &Toolbar::onPausePressed, this, &QtSim::pauseSelectedProject);
    connect(toolbar, &Toolbar::onToggleRecordProject, this, &QtSim::toggleRecordSelectedProject);

    // Setup main frame timer
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [this]()
    {
        if (project && project->started)
        {
            project->_projectProcess();
            canvas->update();

            if (!project->paused)
                project->postProcess();
        }
    });

    setFPS(60);
    timer->start();
}

QtSim::~QtSim()
{}

void QtSim::resizeEvent(QResizeEvent * event)
{
    /*if (canvas)
    {
        canvas->update();
    }*/
}

void QtSim::setProject(int sim_uid)
{
    if (project)
    {
        options->clearAttributeList();
        project->_projectDestroy();
        delete project;
    }

    active_sim_uid = sim_uid;

    project = ProjectBase::findProjectInfo(active_sim_uid)->creator();
    project->configure(active_sim_uid, canvas, options);

    canvas->setProject(project);

    project->_projectPrepare();
    options->updateListUI();
    toolbar->setButtonStates(false, true, false, true);
}

void QtSim::setFPS(int fps)
{
    int delay = 1000 / fps;
    timer->setInterval(delay);
}

void QtSim::startSelectedProject()
{
    if (!project)
        return;

    project->_projectDestroy();
    project->_projectStart();
    toolbar->setButtonStates(true, false, true, true);
}

void QtSim::stopSelectedProject()
{
    if (!project)
        return;

    project->_projectDestroy();
    project->_projectStop();

    toolbar->setRecordingUI(false);
    toolbar->setButtonStates(false, true, false, true);
}

void QtSim::pauseSelectedProject()
{
    if (!project)
        return;

    project->_projectPause();
}

void QtSim::toggleRecordSelectedProject(bool b)
{
    if (project)
    {
        if (b)
        {
            if (project->started)
            {
                // If already started, immediately start recording
                if (project->startRecording())
                    toolbar->setRecordingUI(true);
            }
            else
            {
                // Indicate to project that recording will begin on project start
                project->setRecordOnStart(true);
                toolbar->setRecordingUI(true);
            }
        }
        else
        {
            project->finalizeRecording();
            project->setRecordOnStart(false);

            toolbar->setRecordingUI(false);
        }
    }
}