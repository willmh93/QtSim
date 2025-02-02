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

    AttributeItem* add(const QString& name, AttributeType type);
    void clear();

    QSize sizeHint() const override {
        return layout->sizeHint();
    }
};