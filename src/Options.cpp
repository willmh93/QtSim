#include "Options.h"
//#include "helpers.h"
#include <QDir>

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

    ui.lineEdit_output_path->setText(QDir::toNativeSeparators(getDesktopPath() + "/output.mp4"));

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
        // Check if the target is a pointer, or a hardcoded value
        using DecayedType = std::decay_t<decltype(v)>;
        if constexpr (std::is_pointer_v<DecayedType>)
        {
            // If ptr passed, add pointer to list to update on change
            if (!keep_existing_value)
                val_placeholder = *v; // Set placeholder to current pointer value
            else
                *v = val_placeholder;
            val_ptr.push_back(v);
        }
        else if constexpr (std::is_same_v<DecayedType, T>)
        {
            if (!keep_existing_value)
                val_placeholder = v; // If value passed, set placeholder value to target
        }
    }, target);
}



AttributeItem* Options::realtime_slider(
    QString name, 
    IntVar target, 
    IntVar min, IntVar max, IntVar step,
    IntCallback changed)
{
    auto* item = attributeList->getItem(name);
    bool keep_existing_value = (item != nullptr);
    if (!item)
        item = attributeList->addItem(name, AttributeType::SLIDER_INT, false);
    //item->touched = true;

    assign_ptr(target, item->int_ptrs, item->value_int, keep_existing_value);
    assign_ptr(min, item->slider_int_min_ptr, item->slider_int_min);
    assign_ptr(max, item->slider_int_max_ptr, item->slider_int_max);
    assign_ptr(step, item->slider_int_step_ptr, item->slider_int_step);

    item->updateUIValue();
    return item;
}

AttributeItem* Options::starting_slider(
    QString name,
    IntVar target,
    IntVar min, IntVar max, IntVar step,
    IntCallback changed)
{
    auto* item = attributeList->getItem(name);
    bool keep_existing_value = (item != nullptr);
    if (!item)
        item = attributeList->addItem(name, AttributeType::SLIDER_INT, true);
    //item->touched = true;

    assign_ptr(target, item->int_ptrs, item->value_int, keep_existing_value);
    assign_ptr(min, item->slider_int_min_ptr, item->slider_int_min);
    assign_ptr(max, item->slider_int_max_ptr, item->slider_int_max);
    assign_ptr(step, item->slider_int_step_ptr, item->slider_int_step);

    item->updateUIValue();
    return item;
}

///

AttributeItem* Options::realtime_slider(
    QString name, 
    DoubleVar target, 
    DoubleVar min, DoubleVar max, DoubleVar step, 
    DoubleCallback changed)
{
    auto* item = attributeList->getItem(name);
    bool keep_existing_value = (item != nullptr);
    if (!item)
        item = attributeList->addItem(name, AttributeType::SLIDER_FLOAT, false);
    //item->touched = true;

    assign_ptr(target, item->float_ptrs, item->value_float, keep_existing_value);

    assign_ptr(min, item->slider_float_min_ptr, item->slider_float_min);
    assign_ptr(max, item->slider_float_max_ptr, item->slider_float_max);
    assign_ptr(step, item->slider_float_step_ptr, item->slider_float_step);
    
    item->updateUIValue();
    return item;
}

AttributeItem* Options::starting_slider(
    QString name,
    DoubleVar target,
    DoubleVar min, DoubleVar max,
    DoubleVar step,
    DoubleCallback changed)
{
    auto* item = attributeList->getItem(name);
    bool keep_existing_value = (item != nullptr);
    if (!item)
        item = attributeList->addItem(name, AttributeType::SLIDER_FLOAT, true);
    //item->touched = true;

    //assign_ptr(target, item->value_float_ptr, item->value_float);
    assign_ptr(target, item->float_ptrs, item->value_float, keep_existing_value);

    assign_ptr(min, item->slider_float_min_ptr, item->slider_float_min);
    assign_ptr(max, item->slider_float_max_ptr, item->slider_float_max);
    assign_ptr(step, item->slider_float_step_ptr, item->slider_float_step);

    item->updateUIValue();
    return item;
}

///

AttributeItem* Options::realtime_checkbox(QString name, BoolVar target, BoolCallback on_change)
{
    auto* item = attributeList->getItem(name);
    bool keep_existing_value = (item != nullptr);
    if (!item)
        item = attributeList->addItem(name, AttributeType::CHECKBOX, false);
    item->bool_change = on_change;

    assign_ptr(target, item->bool_ptrs, item->value_bool, keep_existing_value);

    item->updateUIValue();
    return item;
}

AttributeItem* Options::starting_checkbox(QString name, BoolVar target, BoolCallback on_change)
{
    auto* item = attributeList->getItem(name);
    bool keep_existing_value = (item != nullptr);
    if (!item)
        item = attributeList->addItem(name, AttributeType::CHECKBOX, true);
    item->bool_change = on_change;

    assign_ptr(target, item->bool_ptrs, item->value_bool, keep_existing_value);

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
    auto* combo = attributeList->addItem(name, AttributeType::COMBOBOX, false);
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
    attributeList->clearItems();
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
