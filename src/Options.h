#pragma once

#include <unordered_map>

#include <QWidget>
#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include <QPainter>

#include "ui_Options.h"
#include "AttributeList.h"

//#include "Projects/project_types.h"
#include "types.h"


template<typename T>
void assign_ptr(std::variant<T, T*>& value, T*& val_ptr, T& val_placeholder)
{
    std::visit([&val_ptr, &val_placeholder](auto&& v)
    {
        using DecayedType = std::decay_t<decltype(v)>;
        if constexpr (std::is_pointer_v<DecayedType>)
            val_ptr = v; // If ptr passed, use the passed ptr
        else if constexpr (std::is_same_v<DecayedType, T>)
            val_placeholder = v; // If value passed, set placeholder value to target
    }, value);
}

template<typename T>
void assign_ptr(std::variant<T, T*>& target, std::vector<T*>& val_ptr, T& val_placeholder, bool keep_existing_value)
{
    // Visit possible "target" types
    std::visit([&val_ptr, &val_placeholder, keep_existing_value](auto&& v)
    {
        // Is target a pointer? 
        using DecayedType = std::decay_t<decltype(v)>;
        if constexpr (std::is_pointer_v<DecayedType>)
        {
            // Yes, target is a pointer, add pointer to list to update on change
            val_ptr.push_back(v);

            if (!keep_existing_value)
                val_placeholder = *v; // Realtime updates, immediately set placeholder to current pointer value
            else
                *v = val_placeholder; // Only update on start simulation, let it use the cached placeholder until restart
        }
        else if constexpr (std::is_same_v<DecayedType, T>)
        {
            // No, target is a hardcoded value
            if (!keep_existing_value)
                val_placeholder = v; // Realtime updates, immediately set placeholder to current pointer value
        }
    }, target);
}

template<typename T>
void append_ptrs(std::vector<void*>& dest, const std::vector<T>& src)
{
    for (auto& v : src)
        dest.push_back(static_cast<void*>(v));
}


using IntVar = std::variant<int, int*>;
using DoubleVar = std::variant<double, double*>;
using BoolVar = std::variant<bool, bool*>;

using IntCallback = std::function<void(int)>;
using DoubleCallback = std::function<void(double)>;
using BoolCallback = std::function<void(bool)>;

struct InputValueBase
{
    virtual ~InputValueBase() = default;
    virtual void broadcast() = 0;
};

template<typename T>
struct BroadcastableValue : public InputValueBase
{
    // For real target values, multiple updatable pointers can exist
    // For ranges/possible values, only the latest pointer is retained

    T value;
    std::vector<T*> update_ptrs;

    bool realtime_updates;

    BroadcastableValue() = default;
    BroadcastableValue(const BroadcastableValue& rhs)
    {
        value = rhs.value;
        update_ptrs = rhs.update_ptrs;
        realtime_updates = rhs.realtime_updates;
    }

    void provide(
        std::variant<T, T*>& target, 
        bool _realtime_updates, 
        bool item_already_exists)
    {
        realtime_updates = _realtime_updates;

        std::visit([this, item_already_exists](auto&& v)
        {
            using DecayedType = std::decay_t<decltype(v)>;
            if constexpr (std::is_pointer_v<DecayedType>)
            {
                // (v) "target" is T*
                if (!item_already_exists)
                    value = *v;
                else
                    *v = value;

                update_ptrs.push_back(v);
            }
            else if constexpr (std::is_same_v<DecayedType, T>)
            {
                // (v) "target" is T
                if (!item_already_exists)
                    value = v;
            }
        }, target);

        //assign_ptr(target, update_ptrs, value, !realtime_updates);
    }

    /*void assign(const BroadcastableValue& rhs)
    {
        value = rhs.value;
        update_ptrs = rhs.update_ptrs;
        realtime_updates = rhs.realtime_updates;
    }*/

    void set(const T& v)
    {
        value = v;
    }

    void broadcast()
    {
        for (T* ptr : update_ptrs)
            *ptr = value;
    }

    bool removePtr(void* ptr)
    {
        auto it = std::find(update_ptrs.begin(), update_ptrs.end(), ptr);
        if (it != update_ptrs.end())
        {
            update_ptrs.erase(it);
            return true;
        }
        return false;
    }

    operator T()
    {
        return value;
    }
};

using BroadcastableInt     = BroadcastableValue<int>;
using BroadcastableDouble  = BroadcastableValue<double>;
using BroadcastableBool    = BroadcastableValue<bool>;

struct SimItemProxy
{
    AttributeType type;

    BroadcastableInt int_value;
    BroadcastableInt int_min;
    BroadcastableInt int_max;
    BroadcastableInt int_step;

    BroadcastableDouble double_value;
    BroadcastableDouble double_min;
    BroadcastableDouble double_max;
    BroadcastableDouble double_step;

    BroadcastableBool bool_value;

    const std::vector<void*> getPtrs() const
    {
        std::vector<void*> ret;

        append_ptrs(ret, int_value.update_ptrs);
        append_ptrs(ret, int_min.update_ptrs);
        append_ptrs(ret, int_max.update_ptrs);
        append_ptrs(ret, int_step.update_ptrs);

        append_ptrs(ret, double_value.update_ptrs);
        append_ptrs(ret, double_min.update_ptrs);
        append_ptrs(ret, double_max.update_ptrs);
        append_ptrs(ret, double_step.update_ptrs);

        append_ptrs(ret, bool_value.update_ptrs);

        return ret;
    }

    bool removePtr(void* ptr)
    {
        if (int_value.removePtr(ptr)) return true;
        if (int_min.removePtr(ptr)) return true;
        if (int_max.removePtr(ptr)) return true;
        if (int_step.removePtr(ptr)) return true;

        if (double_value.removePtr(ptr)) return true;
        if (double_min.removePtr(ptr)) return true;
        if (double_max.removePtr(ptr)) return true;
        if (double_step.removePtr(ptr)) return true;

        if (bool_value.removePtr(ptr)) return true;

        return false;
    }
};

using SimItemProxyPtr = std::shared_ptr<SimItemProxy>;

class Input;


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

    SimTreeModel model;
    QStandardItem* rootItem;
    DynamicIconDelegate* icon_delegate;

    Input* proxy = nullptr;

public:

    Options(QWidget *parent = nullptr);
    ~Options();


    AttributeItem* int_realtime_slider(QString name, int value, int min, int max, int step=1, IntCallback changed=nullptr);
    //AttributeItem* int_starting_slider(QString name, int value, int min, int max, int step=1, IntCallback changed=nullptr);
    
    ///

    AttributeItem* double_realtime_slider(QString name, double value, double min, double max, double step, DoubleCallback changed=nullptr);
    //AttributeItem* double_starting_slider(QString name, double value, double min, double max, double step, DoubleCallback changed=nullptr);

    ///

    AttributeItem* realtime_checkbox(QString name, bool value, BoolCallback changed = nullptr);
    //AttributeItem* starting_checkbox(QString name, bool value, BoolCallback changed = nullptr);

    ///

    /*void clearAllPointers()
    {
        for (AttributeItem* item : attributeList->item_widgets)
            item->removeAllPointers();
    }*/

    //AttributeItem* slider(const QString& name, double *target, double min, double max, double step=0.1, std::function<void(double)> on_change = nullptr);
    //AttributeItem* number(const QString& name, int min, int max, int step, std::function<void(int)> on_change = nullptr);
    //AttributeItem* number(const QString& name, double min, double max, double step, std::function<void(double)> on_change = nullptr);
    AttributeItem* combo(const QString& name, const std::vector<QString> &items=std::vector<QString>(), std::function<void(int)> on_change = nullptr);
    

    void addSimListEntry(const std::shared_ptr<ProjectInfo>& info);
    void addSimListEntry(ProjectInfo info)
    {
        addSimListEntry(std::make_shared<ProjectInfo>(info));
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

    void onChooseProject(int sim_uid);
    void onForceStartBeginProject(int sim_uid);
    void onStartProject();
    void onStopProject();
    void onChangeFPS(int fps);

    //void onValueChanged(QString name);

    void valueChangedInt(QString name, int value);
    void valueChangedDouble(QString name, double value);
    void valueChangedBool(QString name, bool value);

protected slots:

    void selectionChanged(const QItemSelection& selected, const QItemSelection&);
    void doubleClickSim(const QModelIndex& index);

    friend class Input;

    void removeInput(QString name)
    {
        attributeList->removeItem(name);
    }

    void addInputSliderInt(QString name, SimItemProxyPtr p)
    {
        int_realtime_slider(name, p->int_value, p->int_min, p->int_max, p->int_step, 
            [this, name](int value)
        {
            // On value changed, broadcast
            emit valueChangedInt(name, value);
        });
    }

    void addInputSliderDouble(QString name, SimItemProxyPtr p)
    {
        double_realtime_slider(name, p->double_value, p->double_min, p->double_max, p->double_step,
            [this, name](double value)
        {
            // On value changed, broadcast
            emit valueChangedDouble(name, value);
        });
    }

    void addInputCheckbox(QString name, SimItemProxyPtr p)
    {
        realtime_checkbox(name, p->bool_value, [this, name](bool checked)
        {
            // On value changed, broadcast
            emit valueChangedBool(name, checked);
        });
    }

private:
    Ui::OptionsClass ui;
};


class Input : public QObject
{
    /* Purpose:  
    **    - Middle layer between Options GUI and Thread which contains variables to be modified
    **    - Keep all GUI interaction off of Simulation worker thread
    **
    **
    **           [Options]              - Qt GUI Thread:  e.g. Emits change:  "speed" = 3.7
    **             ^   |
    **             |   v
    **      [======Input======]         - Simulation Thread:  e.g. Listens for "speed" slider changer
    **       ^  |         ^  |
    **       |  v         |  v
    **      [Sim1]       [Sim2]         - Simulation Thread:  e.g.  realtime_slider("speed", &var)
    **                                                              ...all "speed" ptrs get updated
    */

    Q_OBJECT;

    // Mirrored data on Simulation thread
    std::unordered_map<QString, std::shared_ptr<SimItemProxy>> items;

public:

    Options* options = nullptr;

    Input(QObject *parent, Options* options);

    const std::vector<void*> getAllPtrs() const
    {
        std::vector<void*> ret;
        for (auto& item : items)
            append_ptrs(ret, item.second->getPtrs());
        return ret;
    }

    bool removeInputPtr(void* ptr)
    {
        for (auto& item : items)
        {
            if (item.second->removePtr(ptr))
                return true;
        }
        return false;
    }

    void removeUnusedInputs()
    {
        for (auto it = items.begin(); it != items.end();  )
        {
            const QString& name = it->first;
            auto item = it->second;

            if (item->getPtrs().size() == 0)
            {
                QMetaObject::invokeMethod(options, [this, name]() {
                    options->removeInput(name);
                });

                it = items.erase(it);
            }
            else
            {
                it++;
            }

        }
    }

    void clearPointers()
    {
        auto &all_ptrs = getAllPtrs();
        for (void* ptr : all_ptrs)
            removeInputPtr(ptr);
    }

    //void forceBroadcast()
    //{
    //    for (auto& pair : items)
    //    {
    //        pair.second->int_value.broadcast();
    //        pair.second->int_min.broadcast();
    //        pair.second->int_max.broadcast();
    //        pair.second->int_step.broadcast();
    //    }
    //}

    SimItemProxyPtr createItem(QString name, AttributeType type)
    {
        if (!items[name])
        {
            items[name] = std::make_shared<SimItemProxy>();
            items[name]->type = type;
        }
        return items[name];
    }

private:

    // Helpers to avoid code duplication

    SimItemProxyPtr _slider_int(bool realtime, QString name,
        IntVar target, IntVar min, IntVar max, IntVar step = 1, IntCallback changed = nullptr);

    SimItemProxyPtr _slider_double(bool realtime, QString name,
        DoubleVar target, DoubleVar min, DoubleVar max, DoubleVar step = 1.0, DoubleCallback changed = nullptr);
    
    SimItemProxyPtr _checkbox(bool realtime, QString name,
        BoolVar target, BoolCallback changed = nullptr);

public:

    SimItemProxyPtr realtime_slider(QString name, IntVar target,
        IntVar min, IntVar max, IntVar step = 1, IntCallback changed = nullptr);

    SimItemProxyPtr starting_slider(QString name,
        IntVar target, IntVar min, IntVar max, IntVar step = 1, IntCallback changed = nullptr);

    ///

    SimItemProxyPtr realtime_slider(QString name, DoubleVar target,
        DoubleVar min, DoubleVar max, DoubleVar step=-1, DoubleCallback changed = nullptr);

    SimItemProxyPtr starting_slider(QString name, DoubleVar target,
        DoubleVar min, DoubleVar max, DoubleVar step=-1, DoubleCallback changed = nullptr);

    ///

    SimItemProxyPtr realtime_checkbox(QString name, BoolVar target,
        BoolCallback changed = nullptr);

    SimItemProxyPtr starting_checkbox(QString name, BoolVar target,
        BoolCallback changed = nullptr);

public slots:

    void receiveIntChange(QString name, int value)
    {
        auto& item = items[name]->int_value;
        item.set(value);


        if (item.realtime_updates)
            item.broadcast();
    }

    void receiveDoubleChange(QString name, double value)
    {
        auto& item = items[name]->double_value;
        item.set(value);

        if (item.realtime_updates)
            item.broadcast();
    }

    void receiveBoolChange(QString name, bool checked)
    {
        auto& item = items[name]->bool_value;
        item.set(checked);

        if (item.realtime_updates)
            item.broadcast();
    }
};