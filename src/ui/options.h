#pragma once

#include <unordered_map>

#include <QWidget>
#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include <QPainter>

#include "ui_Options.h"
#include "attribute_list.h"

#include "types.h"

class Project;

class DynamicIconDelegate : public QStyledItemDelegate {
public:
    DynamicIconDelegate(QObject* parent = nullptr) : QStyledItemDelegate(parent) {}

    void paint(QPainter* painter, const QStyleOptionViewItem& option,
        const QModelIndex& index) const override;
};

class SimTreeItem : public QStandardItem
{
public:
    std::shared_ptr<ProjectInfo> sim_info;

    SimTreeItem(const QString& label, std::shared_ptr<ProjectInfo> sim_info)
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

        SimTreeItem* item = dynamic_cast<SimTreeItem*>(itemFromIndex(index));

        if (!item->sim_info || item->sim_info->sim_uid < 0)
            return QAbstractItemModel::flags(index) & ~Qt::ItemIsSelectable;

        return QAbstractItemModel::flags(index);
    }
};

// todo: Rename to 'ControlPanel'
class Options : public QWidget
{
    Q_OBJECT;

    AttributeList* attributeList;
    ImOptions* imOptions;

    SimTreeModel model;
    QStandardItem* rootItem;
    DynamicIconDelegate* icon_delegate;

public:

    Options(QWidget *parent = nullptr);
    ~Options();

    void addSimListEntry(const std::shared_ptr<ProjectInfo>& info);
    void addSimListEntry(ProjectInfo info)
    {
        addSimListEntry(std::make_shared<ProjectInfo>(info));
    }

    void refreshTreeUI();

    Size getRecordResolution();
    int getRecordFPS();
    bool isWindowCapture();

    QString getProjectsDirectory();

    void setCurrentProject(Project* project);

signals:

    void onChooseProject(int sim_uid);
    void onForceStartBeginProject(int sim_uid);
    void onStartProject();
    void onStopProject();
    void onChangeFPS(int fps);

    void valueChangedInt(QString name, int value);
    void valueChangedDouble(QString name, double value);
    void valueChangedBool(QString name, bool value);

protected slots:

    void selectionChanged(const QItemSelection& selected, const QItemSelection&);
    void doubleClickSim(const QModelIndex& index);

private:
    Ui::OptionsClass ui;
};

