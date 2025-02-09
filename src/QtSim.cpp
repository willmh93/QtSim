#include "QtSim.h"
#include <QDebug>


QtSim::QtSim(QWidget *parent)
    : QMainWindow(parent)
{
    // Main Menu
    {
        QMenuBar* menuBar = new QMenuBar(this);
        setMenuBar(menuBar);

        menuBar->setStyleSheet("color: rgb(255,255,255);");

        // Create File menu
        QMenu* fileMenu = menuBar->addMenu("Simulation");

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
    }

    // Main Menu
    {
        QStatusBar* statusBar = new QStatusBar(this);
        statusBar->setStyleSheet("color: rgb(255,255,255);");

        statusBar->showMessage("No simulation active");
        setStatusBar(statusBar);
    }

    simulation_type = -1;
    simulation = nullptr;

    // Create the main splitter
    QSplitter* mainSplitter = new QSplitter(Qt::Horizontal, this);

    // Add two OpenGL panels
    options = new Options(this);
    canvas = new Canvas2D(this);

    auto& simulationNames = SimulationBase::getNames();
    for (auto name : simulationNames)
    {
        options->addSimListEntry(name);
    }

    /*options->slider("FPS", 1, 200, 1, [](int v)
    {
        qDebug() << QString("Value: %1").arg(v);
    });

    options->slider("Gravitiational constant (G)", 1.0, 200.0, 10.0, [](double v)
    {
        qDebug() << QString("Value: %1").arg(v);
    });

    options->number("What", 1, 200, 10);
    options->number("What", 1.0, 200.0, 10.0);*/

    // Assign default simulation
    //setSimulation((SimulationType)simulation_index);

    // Connect to start/stop simulation signals
    connect(options, &Options::onChooseSimulation, this, [this](int type)
    {
        setSimulation(type);
    });
    connect(options, &Options::onStartSimulation, this, [this]()
    {
        startSelectedSimulation();
    });
    connect(options, &Options::onStopSimulation, this, [this]()
    {
        stopSelectedSimulation();
    });

    QTimer* timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [this]()
    {
        if (simulation && simulation->started)
        {
            simulation->_process();
            canvas->update();
            simulation->postProcess();
        }
    });
    timer->start(16); // ~60 FPS

    mainSplitter->addWidget(options);
    mainSplitter->addWidget(canvas);

    options->setSizePolicy(QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Minimum);
    mainSplitter->setSizePolicy(QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Minimum);

    // Set initial sizes of the splitter panels
    options->setBaseSize({ 150, 100 });
    mainSplitter->setStretchFactor(0, 0);
    mainSplitter->setStretchFactor(1, 1);

    // Set the splitter as the central widget
    setCentralWidget(mainSplitter);
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

void QtSim::setSimulation(int type)
{
    if (simulation)
    {
        options->clearAttributeList();
        delete simulation;
    }

    simulation_type = type;

    //simulation = SimulationFactory(simulation_type);
    simulation = SimulationBase::getCreators()[simulation_type]();
    simulation->setCanvas(canvas);
    simulation->setOptions(options);
    simulation->configure();

    canvas->setSimulation(simulation);

    simulation->_prepare();
    options->updateListUI();
}

void QtSim::startSelectedSimulation()
{
    if (!simulation)
        return;

    simulation->_destroy();
    simulation->started = true;
    simulation->_start();
}

void QtSim::stopSelectedSimulation()
{
    if (!simulation)
        return;

    simulation->destroy();
    simulation->_stop();
    simulation->started = false;
}
