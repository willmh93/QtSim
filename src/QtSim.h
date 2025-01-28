#pragma once

#include <QMainWindow>
#include <QSplitter>
#include <QTimer>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QMessageBox>

#include "simulations/Simulation.h"
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

private:

    Options* options;
    Canvas2D* canvas;

    Simulation* simulation;
    int simulation_type;

    void setSimulation(int type);
    void startSelectedSimulation();
    void stopSelectedSimulation();

    /*private slots:

    void onSimSelector() {
        SimSelector selector(this);
        if (selector.exec() == QDialog::Accepted)
        {
            int selectedIndex = selector.getSelectedIndex();
        }
    }

    void onAbout() {
    }*/
};
