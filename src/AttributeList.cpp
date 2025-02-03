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

AttributeItem* AttributeList::add(const QString& name, AttributeType type, bool manual_refresh)
{
    for (AttributeItem* item : item_widgets)
    {
        if (item->name == name)
            return item;
    }

    AttributeItem* item = new AttributeItem(name, type, manual_refresh);
    item->attributeList = this;
    layout->addWidget(item);
    item_widgets.push_back(item);
    return item;
}

void AttributeList::clear()
{
    for (auto* w : item_widgets)
    {
        layout->removeWidget(w);
        delete w;
    }
    item_widgets.clear();
}