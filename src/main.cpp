#include "QtSim.h"
#include <QtWidgets/QApplication>

#include <QProcess>
#include <QLibrary>


//#include <clang/Tooling/Tooling.h>
//#include <clang/Frontend/CompilerInstance.h>
//#include <llvm/Support/Host.h>


QString getCompilerCommand() {
#ifdef Q_OS_WIN
    return QCoreApplication::applicationDirPath() + "/external/mingw64/bin/g++.exe";
#elif defined(Q_OS_MAC)
    return QCoreApplication::applicationDirPath() + "/external/clang/bin/clang++";
#else
    return QCoreApplication::applicationDirPath() + "/external/g++";
#endif
}

QString getOutputLibraryName(std::string name) {
#ifdef Q_OS_WIN
    return (name + ".dll").c_str();
#elif defined(Q_OS_MAC)
    return (name + ".dylib").c_str();
#else
    return (name + ".so").c_str();
#endif
}

bool compileCode(const QString& sourceFile, const QString& outputLibrary)
{
    QProcess process;

    // Get the application directory
    QString appDir = QCoreApplication::applicationDirPath();

    // Define paths to MinGW-w64 components
    QString compilerPath = appDir + "/external/mingw64/bin/g++.exe";
    //QString libexecPath = appDir + "/external/mingw64/libexec/gcc/x86_64-w64-mingw32/13.2.0";
    QString srcPath = appDir + "/" + sourceFile;
    QString outPath = appDir + "/" + outputLibrary;

    // Create a custom environment with the correct PATH
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("PATH", QCoreApplication::applicationDirPath() + "/external/bin/mingw/bin"); // Ensure local MinGW is used
    process.setProcessEnvironment(env);

    // Compiler arguments
    QStringList args = { "-shared", "-o", outPath, srcPath };

    // Start the process
    process.start(compilerPath, args);
    process.waitForFinished();

    // Debug output
    qDebug() << "STDOUT:" << process.readAllStandardOutput();
    qDebug() << "STDERR:" << process.readAllStandardError();

    return (process.exitStatus() == QProcess::NormalExit && process.exitCode() == 0);
}

typedef int (*FunctionSignature)(int, int);

void loadLibraryAndExecute(const QString& libName)
{
    QLibrary lib(libName);

    if (!lib.load()) {
        qDebug() << "Failed to load library:" << lib.errorString();
        return;
    }

    FunctionSignature func = (FunctionSignature)lib.resolve("addNumbers");
    if (func) {
        int result = func(7, 3);
        qDebug() << "Function result:" << result;
    }
    else {
        qDebug() << "Function resolution failed!";
    }

    lib.unload();
}

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

    QString sourceFile = "mycode.cpp";
    QString outFile = getOutputLibraryName("mycode");

    if (compileCode(sourceFile, outFile))
    {
        loadLibraryAndExecute(getOutputLibraryName("mycode"));
    }

    QtSim w;
    w.setWindowTitle("QtSim - Developer: Will Hemsworth");
    w.setStyleSheet("background: #2E2E3E;");
    w.resize(1024, 768);
    //w.setWindowIcon(QIcon(":/resources/icon.png"));
    w.setWindowIcon(QIcon(":/resources/icon.ico"));
    //w.show();
    w.showMaximized();
    return a.exec();
}
