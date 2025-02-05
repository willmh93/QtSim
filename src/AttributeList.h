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

    explicit AttributeList(QWidget *parent=nullptr);

    void clearItems();
    AttributeItem* addItem(const QString& name, AttributeType type, bool manual_refresh);
    AttributeItem *getItem(QString name);

    //void forceRefreshPointers()
    //{
    //    for (auto *item : item_widgets)
    //        item->forceRefreshPointers();
    //}

    QSize sizeHint() const override {
        return layout->sizeHint();
    }
};