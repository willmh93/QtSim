#include "AttributeList.h"

AttributeList::AttributeList(QWidget* parent)
{
    layout = new QVBoxLayout(this);
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setAlignment(Qt::AlignTop);  // Ensure widgets stack at the top
    layout->setSizeConstraint(QLayout::SetMinAndMaxSize);
    setLayout(layout);
}

AttributeItem* AttributeList::addItem(const QString& name, AttributeType type, bool manual_refresh)
{
    // If an item with this name already exists, return it
    for (AttributeItem* item : item_widgets)
    {
        if (item->name == name)
            return item;
    }

    // New item
    AttributeItem* item = new AttributeItem(name, type, manual_refresh);
    item->attributeList = this;
    layout->addWidget(item);
    item_widgets.push_back(item);
    return item;
}

AttributeItem* AttributeList::getItem(QString name)
{
    for (AttributeItem* item : item_widgets)
    {
        if (item->name == name)
            return item;
    }
    return nullptr;
}

void AttributeList::clearItems()
{
    for (auto* w : item_widgets)
    {
        layout->removeWidget(w);
        delete w;
    }
    item_widgets.clear();
}