#include "toolbar.h"
#include "ui_toolbar.h"

QPixmap recolorIcon(const QIcon& icon, QSize size, QColor color) {
    QPixmap pixmap = icon.pixmap(size);
    QImage image = pixmap.toImage();

    for (int y = 0; y < image.height(); ++y) {
        for (int x = 0; x < image.width(); ++x) {
            QColor pixelColor = image.pixelColor(x, y);
            if (pixelColor.alpha() > 0) { // Preserve transparency
                image.setPixelColor(x, y, QColor(color.red(), color.green(), color.blue(), pixelColor.alpha()));
            }
        }
    }

    return QPixmap::fromImage(image);
}

void setBtnIconColor(QToolButton* btn, QColor color)
{
    btn->setIcon(recolorIcon(btn->icon(), btn->iconSize(), color));
}

Toolbar::Toolbar(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Toolbar)
{
    ui->setupUi(this);
    //QPalette palette;
    //palette.setColor(QPalette::Window, Qt::darkGray);  // Background
    //palette.setColor(QPalette::WindowText, Qt::white); // Text color
    //QApplication::setPalette(palette);

    /*ui->pauseBtn->setStyleSheet("");
    ui->playBtn->setStyleSheet("");
    ui->stopBtn->setStyleSheet("");
    ui->recordBtn->setStyleSheet("");

    //QApplication::setStyle(QStyleFactory::create("Fusion"));

    ui->recordBtn->setAutoFillBackground(true);


    QPalette pal = ui->recordBtn->palette();
    //pal.setBrush(QPalette::Base, QColor(Qt::red));
    //pal.setBrush(QPalette::Button, QColor(Qt::red));

    pal.setBrush(QPalette::WindowText     , QColor(Qt::red));
    pal.setBrush(QPalette::Button         , QColor(Qt::red));
    pal.setBrush(QPalette::Light          , QColor(Qt::red));
    pal.setBrush(QPalette::Midlight       , QColor(Qt::red));
    pal.setBrush(QPalette::Dark           , QColor(Qt::red));
    pal.setBrush(QPalette::Mid            , QColor(Qt::red));
    pal.setBrush(QPalette::Text           , QColor(Qt::red));
    pal.setBrush(QPalette::BrightText     , QColor(Qt::red));
    pal.setBrush(QPalette::ButtonText     , QColor(Qt::red));
    pal.setBrush(QPalette::Base           , QColor(Qt::red));
    pal.setBrush(QPalette::Window         , QColor(Qt::red));
    pal.setBrush(QPalette::Shadow         , QColor(Qt::red));
    pal.setBrush(QPalette::Highlight      , QColor(Qt::red));
    pal.setBrush(QPalette::HighlightedText, QColor(Qt::red));
    pal.setBrush(QPalette::Link           , QColor(Qt::red));
    pal.setBrush(QPalette::LinkVisited    , QColor(Qt::red));
    pal.setBrush(QPalette::AlternateBase  , QColor(Qt::red));
    pal.setBrush(QPalette::NoRole         , QColor(Qt::red));
    pal.setBrush(QPalette::ToolTipBase    , QColor(Qt::red));
    pal.setBrush(QPalette::ToolTipText    , QColor(Qt::red));
    pal.setBrush(QPalette::PlaceholderText, QColor(Qt::red));
    pal.setBrush(QPalette::Accent         , QColor(Qt::red));*/

    
    //ui->pauseBtn->setPalette(pal);
    //ui->playBtn->setPalette(pal);
    //ui->stopBtn->setPalette(pal);
    //ui->recordBtn->setPalette(pal);
    //ui->recordBtn->update();

    //qDebug() << QApplication::style()->objectName();

    setBtnIconColor(ui->pauseBtn, Qt::white);
    setBtnIconColor(ui->playBtn, Qt::white);
    setBtnIconColor(ui->stopBtn, Qt::white);
    setBtnIconColor(ui->recordBtn, Qt::white);

    //ui->recordBtn->setEnabled(!ui->recordBtn->isEnabled());

    connect(ui->playBtn, &QToolButton::clicked, this, [this]()
    {
        ui->playBtn->setEnabled(false);
        ui->pauseBtn->setEnabled(true);
        ui->stopBtn->setEnabled(true);
        emit onPlayPressed();
    });

    connect(ui->stopBtn, &QToolButton::clicked, this, [this]()
    {
        ui->playBtn->setEnabled(true);
        ui->pauseBtn->setEnabled(false);
        ui->stopBtn->setEnabled(false);
        emit onStopPressed();
    });

    connect(ui->pauseBtn, &QToolButton::clicked, this, [this]()
    {
        ui->playBtn->setEnabled(true);
        ui->pauseBtn->setEnabled(false);
        ui->stopBtn->setEnabled(true);
        emit onPausePressed();
    });

    connect(ui->recordBtn, &QToolButton::clicked, this, [this]()
    {
        //setBtnIconColor(ui->recordBtn, ui->recordBtn->isChecked() ? Qt::red : Qt::white);
        emit onToggleRecordProject(ui->recordBtn->isChecked());

        //if (ui->recordBtn->isChecked())
        //    ui->recordBtn->setStyleSheet("background-color: qlineargradient(x1:0,y1:0,x2:1,y2:1, stop:0 #AA2000, stop:1 #300500);");
        //else
        //    ui->recordBtn->setStyleSheet("background: #1B1B2B;");
    });

    setButtonStates(false, false, false, false);
    //ui->playBtn->setEnabled(false);
    //ui->pauseBtn->setEnabled(false);
    //ui->stopBtn->setEnabled(false);
    //ui->recordBtn->setEnabled(false);
    ui->recordBtn->setChecked(false);
}

void Toolbar::setButtonStates(bool pause_enabled, bool play_enabled, bool stop_enabled, bool record_enabled)
{
    ui->pauseBtn->setEnabled(pause_enabled);
    ui->playBtn->setEnabled(play_enabled);
    ui->stopBtn->setEnabled(stop_enabled);
    ui->recordBtn->setEnabled(record_enabled);
}

void Toolbar::setRecordingUI(bool isRecording)
{
    ui->recordBtn->setChecked(isRecording);
    setBtnIconColor(ui->recordBtn, isRecording ? Qt::red : Qt::white);
}

Toolbar::~Toolbar()
{
    delete ui;
}


