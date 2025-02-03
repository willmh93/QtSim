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
//#include "types.h"



class QtSim : public QMainWindow
{
    Q_OBJECT

public:

    QtSim(QWidget *parent = nullptr);
    ~QtSim();

protected:

    void resizeEvent(QResizeEvent* event) override;

private:

    Options* options = nullptr;
    Canvas2D* canvas = nullptr;

    SimulationBase* simulation;
    int simulation_type;

    void setSimulation(int type);
    void startSelectedSimulation();
    void stopSelectedSimulation();

    private slots:

    void onAbout() 
    {
    }
};
