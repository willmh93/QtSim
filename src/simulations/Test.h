#ifndef TEST_H
#define TEST_H
#include "Simulation.h"

SIM_BEG(Test)

struct Test_Instance : public SimulationInstance
{
    bool transform_coordinates = true;
    bool scale_lines_text = true;
    bool rotate_text = true;

    Test_Instance()
    {
        // Args passed from Test.
        // Use initializer list for reference variables
    }

    void instanceAttributes() override;

    void start();
    void destroy();
    void process(DrawingContext* ctx);
    void draw(DrawingContext* ctx);

    //void mouseDown(MouseInfo mouse);
    //void mouseUp(MouseInfo mouse);
    //void mouseMove(MouseInfo mouse);
    //void mouseWheel(MouseInfo mouse);
};

struct Test : public Simulation<Test_Instance>
{
    int panel_count = 1;

    void projectAttributes() override;
    void prepare();
};

SIM_END
#endif