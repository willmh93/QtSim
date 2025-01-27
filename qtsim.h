#ifndef QTSIM_H
#define QTSIM_H

#include <QMainWindow>

class QtSim : public QMainWindow
{
    Q_OBJECT

public:
    QtSim(QWidget *parent = nullptr);
    ~QtSim();
};
#endif // QTSIM_H
