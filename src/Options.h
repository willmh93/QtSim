#pragma once

#include <QWidget>
#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include <QPainter>

#include "ui_Options.h"
#include "AttributeList.h"

//#include "Simulations/simulation_types.h"
#include "types.h"

class DynamicIconDelegate : public QStyledItemDelegate {
public:
    DynamicIconDelegate(QObject* parent = nullptr) : QStyledItemDelegate(parent) {}

    void paint(QPainter* painter, const QStyleOptionViewItem& option,
        const QModelIndex& index) const override;
};

class SimTreeItem : public QStandardItem
{
public:
    std::shared_ptr<SimulationInfo> sim_info;

    SimTreeItem(const QString& label, std::shared_ptr<SimulationInfo> sim_info)
        : QStandardItem(label), sim_info(sim_info)
    {}
};

class SimTreeModel : public QStandardItemModel
{
public:
    SimTreeModel(QObject* parent = nullptr) : QStandardItemModel(parent)
    {}

    Qt::ItemFlags flags(const QModelIndex& index) const override
    {
        if (!index.isValid())
            return Qt::NoItemFlags;

        SimTreeItem *item = dynamic_cast<SimTreeItem*>(itemFromIndex(index));

        if (!item->sim_info || item->sim_info->sim_uid < 0)
            return QAbstractItemModel::flags(index) & ~Qt::ItemIsSelectable;

        return QAbstractItemModel::flags(index);
    }
};

class Options : public QWidget
{
    Q_OBJECT;

    AttributeList* attributeList;

    //SimTreeModel tree_model;
    SimTreeModel model;
    QStandardItem* rootItem;
    DynamicIconDelegate* icon_delegate;

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

    //AttributeItem* slider(const QString& name, double *target, double min, double max, double step=0.1, std::function<void(double)> on_change = nullptr);
    //AttributeItem* number(const QString& name, int min, int max, int step, std::function<void(int)> on_change = nullptr);
    //AttributeItem* number(const QString& name, double min, double max, double step, std::function<void(double)> on_change = nullptr);
    AttributeItem* combo(const QString& name, const std::vector<QString> &items=std::vector<QString>(), std::function<void(int)> on_change = nullptr);

    void addSimListEntry(const std::shared_ptr<SimulationInfo>& info);
    void addSimListEntry(SimulationInfo info)
    {
        addSimListEntry(std::make_shared<SimulationInfo>(info));
    }

    void clearAttributeList();
    void updateListUI();
    void refreshTreeUI();

    //bool getRecordChecked();
    Size getRecordResolution();
    int getRecordFPS();
    bool isWindowCapture();

    QString getProjectsDirectory();

signals:

    void onChooseSimulation(int sim_uid);
    void onForceStartBeginSimulation(int sim_uid);
    void onStartSimulation();
    void onStopSimulation();
    void onChangeFPS(int fps);

protected slots:

    void selectionChanged(const QItemSelection& selected, const QItemSelection&);
    void doubleClickSim(const QModelIndex& index);

private:
    Ui::OptionsClass ui;
};
