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

#include "main_window.h"
#include <QDebug>


void ProjectWorker::tick()
{
    if (project && project->started)
    {
        QMutexLocker locker(&main_window->sim_lock);
        //qDebug() << "ProjectThread::run() - Mutex LOCKED:" << QThread::currentThread()->objectName();

        project->_projectProcess();
        //canvas->update();

        //QMetaObject::invokeMethod(main_window, [this]() {
        //    main_window->update();
        //});


        // todo: Move to post-draw on GUI thread?
        if (!project->paused)
            project->postProcess();

        //qDebug() << "ProjectThread::run() - Mutex UNLOCKED:" << QThread::currentThread()->objectName();
    }
}

void ProjectWorker::setProject(int sim_uid)
{
    QMutexLocker locker(&main_window->sim_lock);
    qDebug() << "ProjectThread::setProject() - Thread:" << QThread::currentThread()->objectName();

    canvas->setProject(nullptr);

    if (project)
    {
        project->_projectDestroy();
        delete project;
        project = nullptr;
    }

    // Pointers were cleared in _projectDestroy, but when we switch project, 
    // clean up all components rather than keeping them (which we do in case
    // we restart the same simulation but want to preserve input values)
    input_proxy->removeUnusedInputs();

    project = Project::findProjectInfo(sim_uid)->creator();
    project->worker = this;
    project->input_proxy = input_proxy;
    project->configure(sim_uid, canvas, options);

    canvas->setProject(project);

    project->onResize();
    project->_projectPrepare();

    emit onProjectSet();
}


void ProjectWorker::destroyProject()
{
    //QMutexLocker locker(&simulation_lock);
    qDebug() << "ProjectThread::destroyProject() - Thread:" << QThread::currentThread()->objectName();

    if (project)
    {
        project->_projectDestroy();
        delete project;
        project = nullptr;
    }
}

void ProjectWorker::startProject()
{
    if (!project)
        return;

    qDebug() << "ProjectThread::startProject() - Thread:" << QThread::currentThread()->objectName();

    project->_projectDestroy();
    project->_projectStart();

    ///toolbar->setButtonStates(true, false, true, true);
    emit onProjectStarted();
}

void ProjectWorker::stopProject()
{
    if (!project)
        return;

    qDebug() << "ProjectThread::stopProject() - Thread:" << QThread::currentThread()->objectName();

    project->_projectDestroy();
    project->_projectStop();

    emit onProjectStopped();
}

void ProjectWorker::pauseProject()
{
    if (!project)
        return;

    qDebug() << "ProjectThread::pauseProject() - Thread:" << QThread::currentThread()->objectName();

    project->_projectPause();
}

void ProjectWorker::startRecording()
{

}

void ProjectWorker::stopRecording()
{

}



MainWindow::MainWindow(QWidget *parent)
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
        //connect(newAction, &QAction::triggered, this, &MainWindow::onSimSelector);
        connect(exitAction, &QAction::triggered, this, &MainWindow::close);

        // Create Help menu
        QMenu* helpMenu = menuBar->addMenu("Help");

        // Add About action to Help menu
        QAction* aboutAction = new QAction("About", this);
        helpMenu->addAction(aboutAction);

        connect(aboutAction, &QAction::triggered, this, &MainWindow::onAbout);
    }*/

    // Status bar
    {
        QStatusBar* statusBar = new QStatusBar(this);
        statusBar->setStyleSheet("color: rgb(255,255,255);");

        statusBar->showMessage("No project active");
        setStatusBar(statusBar);
    }

    // Create the main splitter
    QSplitter* mainSplitter = new QSplitter(Qt::Horizontal, this);

    // Add two OpenGL viewports
    options = new Options(this);
    canvas = new Canvas2D(this);
    canvas->main_window = this;

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

    auto& sim_factory_list = Project::projectInfoList();
    for (auto factory_info : sim_factory_list)
    {
        options->addSimListEntry(factory_info);
    }
    options->addSimListEntry(ProjectInfo({ "3D" }));
    options->addSimListEntry(ProjectInfo({ "Javascript" }));
    options->addSimListEntry(ProjectInfo({ "Python" }));

    // Handle Sim Treeview signals
    connect(options, &Options::onChooseProject, this, &MainWindow::setProject);
    connect(options, &Options::onForceStartBeginProject, this, [this](int type)
    {
        setProject(type);
        startSelectedProject();
    });

    // Handle FPS change
    connect(options, &Options::onChangeFPS, this, &MainWindow::setFPS);

    // Handle Toolbar signals
    connect(toolbar, &Toolbar::onPlayPressed, this, &MainWindow::startSelectedProject);
    connect(toolbar, &Toolbar::onStopPressed, this, &MainWindow::stopSelectedProject);
    connect(toolbar, &Toolbar::onPausePressed, this, &MainWindow::pauseSelectedProject);
    connect(toolbar, &Toolbar::onToggleRecordProject, this, &MainWindow::toggleRecordSelectedProject);

    project_worker = new ProjectWorker();
    project_worker->options = options;
    project_worker->input_proxy = new Input(this, options);
    project_worker->canvas = canvas;

    project_thread = new ProjectThread(this);
    project_worker->main_window = this;
    project_worker->moveToThread(project_thread);

    QMutexLocker locker(&sim_lock);
    if (project_worker && project_worker->project)
        project_worker->project->onResize();

    project_thread->start();

    QTimer* renderTimer = new QTimer(this);
    connect(renderTimer, &QTimer::timeout, this, [this]()
    {
        canvas->update();
    });
    renderTimer->start(1000 / 60); // 60 FPS
}

MainWindow::~MainWindow()
{
    qDebug() << "~QtSim() Destructor Called";
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    if (project_thread)
    {
        project_thread->quit();
        project_thread->wait();
        delete project_thread;
        project_thread = nullptr;
    }

    QMainWindow::closeEvent(event);
}

void MainWindow::resizeEvent(QResizeEvent *e)
{
    QMutexLocker locker(&sim_lock);
    if (project_worker && project_worker->project)
        project_worker->project->onResize();
}

void MainWindow::setFPS(int fps)
{
    QMetaObject::invokeMethod(project_worker, [this, fps]()
    {
        project_worker->setFPS(fps);
    }, Qt::QueuedConnection);
}

void MainWindow::setProject(int sim_uid)
{
    connect(
        project_worker, &ProjectWorker::onProjectSet,
        this, &MainWindow::onProjectSet,
        static_cast<Qt::ConnectionType>(Qt::QueuedConnection | Qt::UniqueConnection)
    );

    QMetaObject::invokeMethod(project_worker, [this, sim_uid]()
    {
        project_worker->setProject(sim_uid);
    }, Qt::QueuedConnection);
}

void MainWindow::onProjectSet()
{
    options->updateListUI();
    toolbar->setButtonStates(false, true, false, true);
}

void MainWindow::startSelectedProject()
{
    connect(
        project_worker, &ProjectWorker::onProjectStarted,
        this, &MainWindow::onProjectStarted,
        static_cast<Qt::ConnectionType>(Qt::QueuedConnection | Qt::UniqueConnection)
    );

    QMetaObject::invokeMethod(project_worker, [this]()
    {
        project_worker->startProject();
    }, Qt::QueuedConnection);
}

void MainWindow::onProjectStarted()
{
    toolbar->setButtonStates(true, false, true, true);
}

void MainWindow::stopSelectedProject()
{
    connect(
        project_worker, &ProjectWorker::onProjectStopped,
        this, &MainWindow::onProjectStopped,
        static_cast<Qt::ConnectionType>(Qt::QueuedConnection | Qt::UniqueConnection)
    );

    QMetaObject::invokeMethod(project_worker, [this]()
    {
        project_worker->stopProject();
    }, Qt::QueuedConnection);

}

void MainWindow::onProjectStopped()
{
    toolbar->setRecordingUI(false);
    toolbar->setButtonStates(false, true, false, true);
    canvas->update();
}

void MainWindow::pauseSelectedProject()
{
    QMetaObject::invokeMethod(project_worker, [this]()
    {
        project_worker->pauseProject();
    }, Qt::QueuedConnection);
}

void MainWindow::toggleRecordSelectedProject(bool b)
{
    if (project_worker->project)
    {
        if (b)
        {
            if (project_worker->project->started)
            {
                // If already started, immediately start recording
                if (project_worker->project->startRecording())
                    toolbar->setRecordingUI(true);
            }
            else
            {
                // Indicate to project that recording will begin on project start
                project_worker->project->setRecordOnStart(true);
                toolbar->setRecordingUI(true);
            }
        }
        else
        {
            project_worker->project->finalizeRecording();
            project_worker->project->setRecordOnStart(false);

            toolbar->setRecordingUI(false);
        }
    }
}

