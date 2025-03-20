#pragma once

#include <QMainWindow>
#include <QSplitter>
#include <QTimer>
#include <QMenuBar>
#include <QStatusBar>
#include <QMenu>
#include <QAction>
#include <QMessageBox>
#include <QMutexLocker>
#include <QMutex>

#include "project.h"
#include "gl_engine_abstract.h"
#include "canvas.h"
#include "options.h"
#include "toolbar.h"

class ProjectThread : public QThread
{
    Q_OBJECT;

public:

    ProjectThread(QObject* parent = nullptr) : QThread(parent) 
    {
        setObjectName("Simulation Thread");
    }

    void run() override
    {
        QThread::exec();
    }
};

class ProjectWorker : public QObject
{
    Q_OBJECT;

    QTimer* tick_timer = nullptr;

public:

    MainWindow* main_window = nullptr;
    Project* project = nullptr;

    Options* options = nullptr;
    Input* input_proxy = nullptr;

    RecordableCanvasWidget* canvas = nullptr;

   
    ProjectWorker(QObject* parent = nullptr)
        : QObject(parent)
    {
        tick_timer = new QTimer(this);
        
        connect(tick_timer, &QTimer::timeout, this, &ProjectWorker::tick);

        tick_timer->start(1000 / 60);
    }

public slots:

    void tick();

signals:

    void onProjectSet();
    void onProjectStarted();
    void onProjectStopped();

public slots:

    void setProject(int sim_uid);
    void destroyProject();
    void startProject();
    void stopProject();
    void pauseProject();
    void startRecording();
    void stopRecording();

    void setFPS(int fps)
    {
        tick_timer->setInterval(1000 / fps);
    }
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

    //QTimer* tick_timer = nullptr;
    ProjectWorker* project_worker = nullptr;
    ProjectThread* project_thread = nullptr;

public:

    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:

    friend class ProjectWorker;
    friend class CanvasWidget;

    QMutex sim_lock;

    void closeEvent(QCloseEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:

    Options* options = nullptr;
    //CanvasWidget* canvas = nullptr;
    RecordableCanvasWidget* canvas = nullptr;
    Toolbar* toolbar = nullptr;

    
    //int active_sim_uid;

    void setProject(int sim_uid);

    void setFPS(int fps);

    void startSelectedProject();
    void stopSelectedProject();
    void pauseSelectedProject();
    void toggleRecordSelectedProject(bool b);

private slots:

    void onProjectSet();
    void onProjectStarted();
    void onProjectStopped();

    void onAbout() {}
};
