#ifndef TOOLBAR_H
#define TOOLBAR_H

#include <QWidget>

namespace Ui {
class Toolbar;
}

class Toolbar : public QWidget
{
    Q_OBJECT

public:
    explicit Toolbar(QWidget *parent = nullptr);
    ~Toolbar();

    void setButtonStates(
        bool pause_enabled,
        bool play_enabled,
        bool stop_enabled,
        bool record_enabled
    );

    void setRecordingUI(bool isRecording);

signals:

    void onPlayPressed();
    void onStopPressed();
    void onPausePressed();
    void onToggleRecordSimulation(bool b);

private:
    Ui::Toolbar *ui;
};

#endif // TOOLBAR_H
