#pragma once

#include <QMainWindow>
#include <QSplitter>
#include <QTimer>
#include <QMenuBar>
#include <QStatusBar>
#include <QMenu>
#include <QAction>
#include <QMessageBox>

#include "Simulation.h"
#include "Canvas2D.h"
//#include "SimSelector.h"
#include "Options.h"
#include "toolbar.h"
//#include "types.h"



class QtSim : public QMainWindow
{
    Q_OBJECT

    QTimer* timer;

public:

    QtSim(QWidget *parent = nullptr);
    ~QtSim();

protected:

    void resizeEvent(QResizeEvent* event) override;

private:

    Options* options = nullptr;
    Canvas2D* canvas = nullptr;
    Toolbar* toolbar = nullptr;

    Simulation* simulation;
    int active_sim_uid;

    void setSimulation(int sim_uid);
    void setFPS(int fps);

    void startSelectedSimulation();
    void stopSelectedSimulation();
    void pauseSelectedSimulation();
    void toggleRecordSelectedSimulation(bool b);

    private slots:

    void onAbout() 
    {
    }
};
