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

    using IntVar    = std::variant<int, int*>;
    using DoubleVar = std::variant<double, double*>;
    using BoolVar   = std::variant<bool, bool*>;

    using IntCallback    = std::function<void(int)>;
    using DoubleCallback = std::function<void(double)>;
    using BoolCallback   = std::function<void(bool)>;


    AttributeItem* realtime_slider(QString name, IntVar target, IntVar min, IntVar max, IntVar step=1, IntCallback changed=nullptr);
    AttributeItem* starting_slider(QString name, IntVar target, IntVar min, IntVar max, IntVar step=1, IntCallback changed=nullptr);
    
    ///

    AttributeItem* realtime_slider(QString name, DoubleVar target, DoubleVar min, DoubleVar max, DoubleVar step, DoubleCallback changed=nullptr);
    AttributeItem* starting_slider(QString name, DoubleVar target, DoubleVar min, DoubleVar max, DoubleVar step, DoubleCallback changed=nullptr);

    ///

    AttributeItem* realtime_checkbox(QString name, BoolVar target, BoolCallback changed = nullptr);
    AttributeItem* starting_checkbox(QString name, BoolVar target, BoolCallback changed = nullptr);

    ///

    void clearAllPointers()
    {
        for (AttributeItem* item : attributeList->item_widgets)
            item->removeAllPointers();
    }

    //void forceRefreshPointers()
    //{
    //    attributeList->forceRefreshPointers();
    //}


    /*void garbageTakePriorSnapshot()
    {
        for (AttributeItem* item : attributeList->item_widgets)
        {
            item->touched = false;
            item->garbageTakePriorSnapshot();
            //AttributeItemSnapshot item_snapshot;
            //item_snapshot.name = item->name;
            //item_snapshot.ptrs = item->getValuePointers();
        }
    }

    void garbageRemoveUnreferencedPointers()
    {
        for (AttributeItem* item : attributeList->item_widgets)
        {
            if (item->touched)
            {
                item->garbageRemoveUnreferencedPointers();
            }
        }
        updateListUI();
    }*/

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
