#include "helpers.h"


double min(double a, double b)
{
    if (a < b)
        return a;
    else
        return b;
}

double max(double a, double b)
{
    if (a > b)
        return a;
    else
        return b;
}

QString normalizeSeconds(double seconds)
{
    if (seconds <= 60.0)
        return QString::number(seconds, 'f', 0) + " seconds";

    double minutes = seconds / 60.0;
    if (minutes <= 60.0)
        return QString::number(minutes, 'f', 0) + " minutes";

    double hours = minutes / 60.0;
    if (hours <= 24.0)
        return QString::number(hours, 'f', 1) + " hours";

    double days = hours / 24.0;
    if (days < 365.2422)
        return QString::number(days, 'f', 2) + " days";

    double years = days / 365.2422;
    if (years < 10)
        return QString::number(years, 'f', 2) + " years";

    double million_years = years / 1000000.0;
    if (years < 1000000)
        return QString::number(million_years, 'f', 3) + " million years";

    double billion_years = years / 1000000000.0;
    if (years < 1000000000)
        return QString::number(billion_years, 'f', 3) + " billion years";

    double trillion_years = years / 1000000000000.0;
    return QString::number(trillion_years, 'f', 3) + " trillion years";
}

// |error| < 0.005
float atan2_approximation2(double y, double x)
{
    if (x == 0.0f)
    {
        if (y > 0.0f) return PIBY2_FLOAT;
        if (y == 0.0f) return 0.0f;
        return -PIBY2_FLOAT;
    }
    double atan;
    double z = y / x;
    if (fabs(z) < 1.0f)
    {
        atan = z / (1.0f + 0.28f * z * z);
        if (x < 0.0f)
        {
            if (y < 0.0f) return atan - PI_FLOAT;
            return atan + PI_FLOAT;
        }
    }
    else
    {
        atan = PIBY2_FLOAT - z / (z * z + 0.28f);
        if (y < 0.0f) return atan - PI_FLOAT;
    }
    return atan;
}

QString getEnvironmentVariable(const char* varName) {
#ifdef _WIN32
    // Use _dupenv_s on Windows
    char* buffer = nullptr;
    size_t size = 0;
    if (_dupenv_s(&buffer, &size, varName) == 0 && buffer != nullptr) {
        QString value = QString::fromLocal8Bit(buffer);
        free(buffer);  // Free the allocated buffer
        return value;
    }
    return QString();  // Return an empty QString if the environment variable is not found
#else
    // Use getenv on other platforms
    const char* value = std::getenv(varName);
    return value ? QString::fromLocal8Bit(value) : QString();
#endif
}

QString getDesktopPath() {
#ifdef _WIN32
    QString userProfile = getEnvironmentVariable("USERPROFILE");
    if (!userProfile.isEmpty()) {
        return QDir::toNativeSeparators(userProfile + "\\Desktop");
    }
#elif defined(__APPLE__) || defined(__linux__)
    QString homeDir = getEnvironmentVariable("HOME");
    if (!homeDir.isEmpty()) {
        return QDir::toNativeSeparators(homeDir + "/Desktop");
    }
#endif

    // Fallback: Use the current working directory
    return QDir::toNativeSeparators(QDir::currentPath());
}
