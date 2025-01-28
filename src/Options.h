#pragma once

#include <QWidget>
#include <QStandardItemModel>
#include "ui_Options.h"
#include "AttributeList.h"

//#include "Simulations/simulation_types.h"
#include "types.h"

class Options : public QWidget
{
    Q_OBJECT;

    AttributeList* attributeList;
    QStandardItemModel list_model;

public:
    Options(QWidget *parent = nullptr);
    ~Options();

    //AttributeItem* add(const QString& name, AttributeType type);
    AttributeItem* slider(
        const QString& name, 
        std::variant<int,int*> target, 
        std::variant<int, int*> min, 
        std::variant<int, int*> max, 
        std::variant<int, int*> step=1, 
        std::function<void(int)> on_change=nullptr);

    AttributeItem* slider(
        const QString& name,
        std::variant<double, double*> target,
        std::variant<double, double*> min,
        std::variant<double, double*> max,
        std::variant<double, double*> step = 1,
        std::function<void(double)> on_change = nullptr);

    AttributeItem* checkbox(
        const QString& name,
        std::variant<bool, bool*> target,
        std::function<void(bool)> on_change = nullptr);

    //AttributeItem* slider(const QString& name, double *target, double min, double max, double step=0.1, std::function<void(double)> on_change = nullptr);
    //AttributeItem* number(const QString& name, int min, int max, int step, std::function<void(int)> on_change = nullptr);
    //AttributeItem* number(const QString& name, double min, double max, double step, std::function<void(double)> on_change = nullptr);
    AttributeItem* combo(const QString& name, const std::vector<QString> &items=std::vector<QString>(), std::function<void(int)> on_change = nullptr);

    void addSimListEntry(const QString& name);

    void clearAttributeList();
    void updateListUI();

    bool getRecordChecked();
    Size getRecordResolution();
    QString getRecordPath();

signals:

    void onChooseSimulation(int type);
    void onStartSimulation();
    void onStopSimulation();

protected slots:

    void selectionChanged(const QItemSelection& selected, const QItemSelection&);
    void startClicked();
    void stopClicked();

private:
    Ui::OptionsClass ui;
};
