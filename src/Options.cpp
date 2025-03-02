/*
 * This file is part of QtSim
 *
 * Copyright (C) 2025 William Hemsworth
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "Options.h"
#include <QDir>

QString getEnvironmentVariable(const char* varName)
{
    // Use getenv on other platforms
    const char* value = std::getenv(varName);
    return value ? QString::fromLocal8Bit(value) : QString();
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

    ui.simTree->setSelectionMode(QAbstractItemView::SingleSelection);
    ui.simTree->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui.simTree->setModel(&model);

    icon_delegate = new DynamicIconDelegate(ui.simTree);
    ui.simTree->setItemDelegate(icon_delegate);

    //connect(ui.startBtn, &QPushButton::clicked, this, &Options::startClicked);
    //connect(ui.stopBtn, &QPushButton::clicked, this, &Options::stopClicked);
    connect(ui.simTree->selectionModel(), &QItemSelectionModel::selectionChanged, this, &Options::selectionChanged);
    connect(ui.simTree, &QTreeView::doubleClicked, this, &Options::doubleClickSim);
    connect(ui.spinBox_fps, &QSpinBox::valueChanged, this, &Options::onChangeFPS);
    //connect(ui.checkBox_windowCapture, &QCheckBox::checkStateChanged,)

    model.setHorizontalHeaderLabels({"Project Type"});
    rootItem = model.invisibleRootItem();

    ui.project_dir_input->setText(QDir::toNativeSeparators(getDesktopPath() + "/QtSims"));

    ui.frame_cacheOptions->hide();
}

Options::~Options()
{
}

void Options::addSimListEntry(const std::shared_ptr<ProjectInfo>& info)
{
    auto& path = info->path;

    // Start from the root and traverse the tree
    QStandardItem* parent = rootItem;  // Assuming `rootItem` is your top-level node

    //for (const QString& part : path)
    for (size_t part_index=0; part_index<path.size(); part_index++)
    {
        const QString& part = path[part_index];
        bool found = false;

        // Loop over current node children
        for (int i = 0; i < parent->rowCount(); ++i) 
        {
            // Did we find this part?
            QStandardItem* child = parent->child(i);
            if (child->text() == part) 
            {
                parent = child;
                found = true;
                break;
            }
        }

        if (!found) 
        {
            bool isLeaf = (part_index == path.size() - 1);

            // Create a new item if not found
            SimTreeItem* newItem = new SimTreeItem(part, isLeaf ? info : nullptr);
            parent->appendRow(newItem);
            parent = newItem;
        }
    }

    ui.simTree->expandAll();
}

void Options::selectionChanged(const QItemSelection& selected, const QItemSelection&)
{
    if (!selected.indexes().isEmpty())
    {
        QModelIndex index = selected.indexes().first();
        SimTreeItem* item = dynamic_cast<SimTreeItem*>(model.itemFromIndex(index));

        if (item->sim_info && item->sim_info->sim_uid >= 0)
        {
            emit onChooseProject(item->sim_info->sim_uid);
            return;
        }
    }
}

void Options::doubleClickSim(const QModelIndex& index)
{
    SimTreeItem* item = dynamic_cast<SimTreeItem*>(model.itemFromIndex(index));
    if (item->sim_info && item->sim_info->sim_uid >= 0)
    {
        emit onForceStartBeginProject(item->sim_info->sim_uid);
        return;
    }
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

void Options::refreshTreeUI()
{
    ui.simTree->update();
}

Size Options::getRecordResolution()
{
    return {
        ui.spinBox_width->value(), 
        ui.spinBox_height->value() 
    };
}

int Options::getRecordFPS()
{
    return ui.spinBox_fps->value();
}

bool Options::isWindowCapture()
{
    return ui.checkBox_windowCapture->isChecked();
}

QString Options::getProjectsDirectory()
{
    return QDir::toNativeSeparators(ui.project_dir_input->text());
}

void DynamicIconDelegate::paint(
    QPainter* painter, 
    const QStyleOptionViewItem& option, 
    const QModelIndex& index) const
{
    // Get the text from the model
    QString text = index.data(Qt::DisplayRole).toString();
    if (text.isEmpty()) return;



    // Define the rectangle for text
    QRect textRect = option.rect;
    //textRect.setLeft(option.rect.left() + 5); // Small left margin for readability


    if (option.state & QStyle::State_Selected) {
        painter->fillRect(option.rect, option.palette.highlight());
        painter->setPen(option.palette.highlightedText().color());
    }
    else {
        painter->setPen(option.palette.text().color()); // Reset to default text color
    }

    // Draw the text
    painter->drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, text);

    //QTreeView* treeView = qobject_cast<QTreeView*>(parent());

    const QAbstractItemModel* model = index.model();  // Get the model
    const QStandardItemModel* standardModel = qobject_cast<const QStandardItemModel*>(model);
    SimTreeItem* item = dynamic_cast<SimTreeItem*>(standardModel->itemFromIndex(index));

    std::shared_ptr<ProjectInfo>& sim_info = item->sim_info;

    if (sim_info && sim_info->sim_uid >= 0)
    {
        // Define icon properties
        int iconSize = 10; // Diameter of the circle
        int spacing = 8;    // Space between text and icon
        QColor iconColor;// = (index.row() % 2 == 0) ? Qt::red : Qt::blue; // Alternate colors

        switch (sim_info->state)
        {
        case ProjectInfo::INACTIVE: iconColor = Qt::darkGray; break;
        case ProjectInfo::ACTIVE: iconColor = Qt::green; break;
        case ProjectInfo::RECORDING: iconColor = Qt::red; break;
        }

        // Set up font metrics to measure text width
        QFontMetrics fm(option.font);
        int textWidth = fm.horizontalAdvance(text);

        // Define the rectangle for the icon (right after the text)
        QRect iconRect(textRect.left() + textWidth + spacing, option.rect.center().y() - iconSize / 2 + 2,
            iconSize, iconSize);


        // Draw the dynamically generated icon (a colored circle)
        painter->setRenderHint(QPainter::Antialiasing);
        painter->setBrush(iconColor);

        painter->setPen(QPen(QBrush({ 30,30,40 }), 2));
        painter->drawEllipse(iconRect);
    }

    // Reset painter brush to default after drawing
    painter->setPen(Qt::NoPen);
    painter->setBrush(Qt::NoBrush);
}

