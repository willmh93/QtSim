#include "qtsim.h"

#include <QApplication>

int main(int argc, char *argv[])
{
#ifdef Q_OS_WIN
    // Select between OpenGL and OpenGL ES (Angle)
    //QCoreApplication::setAttribute(Qt::AA_UseOpenGLES);
    QCoreApplication::setAttribute(Qt::AA_UseDesktopOpenGL);
#endif

    QApplication a(argc, argv);

    QtSim w;
    w.setWindowTitle("QtSim - Developer: Will Hemsworth");
    w.resize(1024, 768);
    //w.show();
    w.showMaximized();
    return a.exec();
}
