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
    }*/

    // Status bar
    {
        QStatusBar* statusBar = new QStatusBar(this);
        statusBar->setStyleSheet("color: rgb(255,255,255);");

        statusBar->showMessage("No simulation active");
        setStatusBar(statusBar);
    }

    active_sim_uid = -1;
    simulation = nullptr;

    // Create the main splitter
    QSplitter* mainSplitter = new QSplitter(Qt::Horizontal, this);

    // Add two OpenGL panels
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

    // Set initial sizes of the splitter panels
    options->setBaseSize({ 150, 100 });
    mainSplitter->setStretchFactor(0, 0);
    mainSplitter->setStretchFactor(1, 1);

    // Set the splitter as the central widget
    setCentralWidget(mainSplitter);

    auto& sim_factory_list = Simulation::simulationInfoList();
    for (auto factory_info : sim_factory_list)
    {
        options->addSimListEntry(factory_info);
    }
    options->addSimListEntry(SimulationInfo({ "3D" }));
    options->addSimListEntry(SimulationInfo({ "Javascript" }));
    options->addSimListEntry(SimulationInfo({ "Python" }));

    // Handle Sim Treeview signals
    connect(options, &Options::onChooseSimulation, this, &QtSim::setSimulation);
    connect(options, &Options::onForceStartBeginSimulation, this, [this](int type)
    {
        setSimulation(type);
        startSelectedSimulation();
    });

    // Handle FPS change
    connect(options, &Options::onChangeFPS, this, &QtSim::setFPS);

    // Handle Toolbar signals
    connect(toolbar, &Toolbar::onPlayPressed, this, &QtSim::startSelectedSimulation);
    connect(toolbar, &Toolbar::onStopPressed, this, &QtSim::stopSelectedSimulation);
    connect(toolbar, &Toolbar::onPausePressed, this, &QtSim::pauseSelectedSimulation);
    connect(toolbar, &Toolbar::onToggleRecordSimulation, this, &QtSim::toggleRecordSelectedSimulation);

    // Setup main frame timer
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [this]()
    {
        if (simulation && simulation->started)
        {
            simulation->_process();
            canvas->update();

            if (!simulation->paused)
                simulation->postProcess();
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

void QtSim::setSimulation(int sim_uid)
{
    if (simulation)
    {
        options->clearAttributeList();
        simulation->_destroy();
        delete simulation;
    }

    active_sim_uid = sim_uid;

    simulation = Simulation::findSimulationInfo(active_sim_uid)->creator();
    simulation->configure(active_sim_uid, canvas, options);

    canvas->setSimulation(simulation);

    simulation->_prepare();
    options->updateListUI();
    toolbar->setButtonStates(false, true, false, true);
}

void QtSim::setFPS(int fps)
{
    int delay = 1000 / fps;
    timer->setInterval(delay);
}

void QtSim::startSelectedSimulation()
{
    if (!simulation)
        return;

    simulation->_destroy();
    simulation->_start();
    toolbar->setButtonStates(true, false, true, true);
}

void QtSim::stopSelectedSimulation()
{
    if (!simulation)
        return;

    simulation->destroy();
    simulation->_stop();

    toolbar->setRecordingUI(false);
    toolbar->setButtonStates(false, true, false, true);
}

void QtSim::pauseSelectedSimulation()
{
    if (!simulation)
        return;

    simulation->_pause();
}

void QtSim::toggleRecordSelectedSimulation(bool b)
{
    if (simulation)
    {
        if (b)
        {
            if (simulation->started)
            {
                // If already started, immediately start recording
                if (simulation->startRecording())
                    toolbar->setRecordingUI(true);
            }
            else
            {
                // Indicate to simulation that recording will begin on simulation start
                simulation->setRecordOnStart(true);
                toolbar->setRecordingUI(true);
            }
        }
        else
        {
            simulation->finalizeRecording();
            simulation->setRecordOnStart(false);

            toolbar->setRecordingUI(false);
        }
    }
}