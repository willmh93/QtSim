#pragma once
#include <QString>
#include <QDir>

double min(double a, double b);
double max(double a, double b);

QString normalizeSeconds(double seconds);

#define PI_FLOAT     3.14159265
#define PIBY2_FLOAT  1.5707963

// |error| < 0.005
inline float atan2_approximation2(double y, double x);

QString getEnvironmentVariable(const char* varName);
QString getDesktopPath();
