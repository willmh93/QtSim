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

#include "AttributeList.h"

AttributeList::AttributeList(QWidget* parent)
{
    layout = new QVBoxLayout(this);
    layout->setSpacing(6);
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