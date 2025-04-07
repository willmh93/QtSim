#include "options.h"
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
    //attributeList = ui.attributeList;
    imOptions = ui.imOptions;

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

    //ui.simTree->expandAll();
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

void Options::setCurrentProject(Project* _project)
{
    this->imOptions->setCurrentProject(_project);
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

