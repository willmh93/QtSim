#include "Options.h"

Options::Options(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);
    attributeList = ui.attributeList;

    // Set the model to the QListView
    ui.simList->setModel(&list_model);

    connect(ui.startBtn, &QPushButton::clicked, this, &Options::startClicked);
    connect(ui.stopBtn, &QPushButton::clicked, this, &Options::stopClicked);
    connect(ui.simList->selectionModel(), &QItemSelectionModel::selectionChanged, this, &Options::selectionChanged);

    connect(ui.checkBox_record, &QCheckBox::toggled, this, [this](bool checked) 
    {
        if (checked)
            ui.frame_recordOptions->show();
        else
            ui.frame_recordOptions->hide();
    });
    ui.frame_recordOptions->hide();

    /*attributeList->on_update = [this]()
    {
        for (auto& item : attributeList->item_widgets)
        {
            item->updateUIValue();
        }
    };*/
}

Options::~Options()
{}


void Options::selectionChanged(const QItemSelection& selected, const QItemSelection&)
{
    QModelIndex currentIndex = ui.simList->currentIndex();

    if (currentIndex.isValid())
    {
        int selectedIndex = currentIndex.row();
        emit onChooseSimulation(selectedIndex);
    }
    else
    {
        emit onChooseSimulation(-1);
    }
}

void Options::startClicked()
{
    emit onStartSimulation();
}

void Options::stopClicked()
{
    emit onStopSimulation();
}

template<typename T>
void assign_ptr(std::variant<T, T*>& value, T*& val_ptr, T& val_placeholder)
{
    std::visit([&val_ptr, &val_placeholder](auto&& v)
    {
        using DecayedType = std::decay_t<decltype(v)>;
        if constexpr (std::is_pointer_v<DecayedType>)
            val_ptr = v;
        else if constexpr (std::is_same_v<DecayedType, T>)
            val_placeholder = v;
    }, value);
}

AttributeItem* Options::slider(
    const QString& name, 
    std::variant<int, int*> target, 
    std::variant<int, int*> min,
    std::variant<int, int*> max,
    std::variant<int, int*> step,
    std::function<void(int)> on_change)
{
    auto* item = attributeList->add(name, AttributeType::SLIDER_INT);// ->setRange(min, max, step);
    /*std::visit([item](auto&& v)
    {
        if constexpr (std::is_pointer_v<decltype(v)>)
            item->value_int_ptr = v;
        else if constexpr (std::is_same_v<decltype(v), int>)
            item->value_int = v;
    }, target);*/

    assign_ptr(target, item->value_int_ptr, item->value_int);
    assign_ptr(min, item->slider_int_min_ptr, item->slider_int_min);
    assign_ptr(max, item->slider_int_max_ptr, item->slider_int_max);
    assign_ptr(step, item->slider_int_step_ptr, item->slider_int_step);

    item->updateUIValue();

    /*if (std::holds_alternative<int*>(target))
        item->value_int_ptr = std::get<int*>(target);
    else
        item->value_int = std::get<int>(target);*/

    /*item->slider_int_step = step;
    item->slider_int_min = min;
    item->slider_int_max = max;

    //item->setValue(*target);
    item->value_int = target;
    item->int_change = [target, on_change](int v) {
        *target = v;
        if (on_change) on_change(v);
    };*/
    return item;
}

AttributeItem* Options::slider(
    const QString& name,
    std::variant<double, double*> target,
    std::variant<double, double*> min,
    std::variant<double, double*> max,
    std::variant<double, double*> step,
    std::function<void(double)> on_change)
{
    auto* item = attributeList->add(name, AttributeType::SLIDER_FLOAT);// ->setRange(min, max, step);
    assign_ptr(target, item->value_float_ptr, item->value_float);
    assign_ptr(min, item->slider_float_min_ptr, item->slider_float_min);
    assign_ptr(max, item->slider_float_max_ptr, item->slider_float_max);
    assign_ptr(step, item->slider_float_step_ptr, item->slider_float_step);
    
    item->updateUIValue();
    return item;
}

AttributeItem* Options::checkbox(
    const QString& name,
    std::variant<bool, bool*> target,
    std::function<void(bool)> on_change)
{
    auto* item = attributeList->add(name, AttributeType::CHECKBOX);// ->setRange(min, max, step);
    assign_ptr(target, item->value_bool_ptr, item->value_bool);

    item->updateUIValue();
    return item;
}

/*AttributeItem* Options::number(const QString& name, int min, int max, int step, std::function<void(int)> on_change)
{
    auto* item = attributeList->add(name, AttributeType::INPUT_INT)->setRange(min, max, step);
    item->int_change = on_change;
    return item;
}

AttributeItem* Options::number(const QString& name, double min, double max, double step, std::function<void(double)> on_change)
{
    auto* item = attributeList->add(name, AttributeType::INPUT_FLOAT)->setRange(min, max, step);
    item->float_change = on_change;
    return item;
}*/

AttributeItem* Options::combo(const QString& name, const std::vector<QString>& items, std::function<void(int)> on_change)
{
    auto* combo = attributeList->add(name, AttributeType::COMBOBOX);
    for (auto& item : items)
        combo->addComboItem(item);
    combo->int_change = on_change;
    return combo;
}



void Options::addSimListEntry(const QString& name)
{
    QStandardItem* item = new QStandardItem(name);
    list_model.appendRow(item);
}

void Options::clearAttributeList()
{
    attributeList->clear();
}

void Options::updateListUI()
{
    for (auto& item : attributeList->item_widgets)
    {
        item->updateUIValue();
    }
}

bool Options::getRecordChecked()
{
    return  ui.checkBox_record->isChecked();
}

Size Options::getRecordResolution()
{
    return {
        ui.spinBox_width->value(), 
        ui.spinBox_height->value() 
    };
}

QString Options::getRecordPath()
{
    return ui.lineEdit_output_path->text();
}
