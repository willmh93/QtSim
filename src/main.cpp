#include "QtSim.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    // load res from libqnanopainter
    Q_INIT_RESOURCE(libqnanopainterdata);

#ifdef Q_OS_WIN
    // Select between OpenGL and OpenGL ES (Angle)
    //QCoreApplication::setAttribute(Qt::AA_UseOpenGLES);
    QCoreApplication::setAttribute(Qt::AA_UseDesktopOpenGL);
#endif

    //test();

    QApplication a(argc, argv);
    //a.setStyleSheet("* { border: 1px solid red; }");

    QtSim w;
    w.setWindowTitle("QtSim - Developer: Will Hemsworth");
    w.resize(1024, 768);
    //w.show();
    w.showMaximized();
    return a.exec();
}
