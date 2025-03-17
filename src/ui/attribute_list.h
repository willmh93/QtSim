#pragma once

#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include "attribute_item.h"

class AttributeList : public QWidget
{
    Q_OBJECT;

    QVBoxLayout* layout;

public:

    std::vector<AttributeItem*> item_widgets;

    explicit AttributeList(QWidget *parent=nullptr);

    void clearItems();
    void removeItem(const QString &name);

    AttributeItem* addItem(const QString& name, AttributeType type, bool manual_refresh);
    AttributeItem *getItem(QString name);

    QSize sizeHint() const override {
        return layout->sizeHint();
    }
};