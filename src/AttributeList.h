#pragma once

#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include "AttributeItem.h"

class AttributeList : public QWidget
{
    Q_OBJECT;

    QVBoxLayout* layout;

public:

    std::vector<AttributeItem*> item_widgets;
    //std::function<void(void)> on_update = nullptr;

    explicit AttributeList(QWidget *parent=nullptr);

    AttributeItem* add(const QString& name, AttributeType type, bool manual_refresh);
    void clear();

    AttributeItem *getItem(QString name)
    {
        for (AttributeItem* item : item_widgets)
        {
            if (item->name == name)
                return item;
        }
        return nullptr;
    }

    void forceRefreshPointers()
    {
        for (auto *item : item_widgets)
            item->forceRefreshPointers();
    }

    QSize sizeHint() const override {
        return layout->sizeHint();
    }
};