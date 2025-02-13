#include "AttributeItem.h"
#include "AttributeList.h"

int getDecimalPlaces(double step)
{
    int decimalPlaces = 0;

    // Handle small tolerances for floating-point errors
    while (std::fmod(step, 1.0) != 0.0) {
        step *= 10;
        decimalPlaces++;
        if (decimalPlaces > 15) break; // Prevent infinite loop for extremely small values
    }

    return decimalPlaces;
}


AttributeItem::AttributeItem(
    const QString& _name,
    AttributeType _type,
    bool _manual_refresh,
    QWidget* parent)
{
    name = _name;
    type = _type;
    manual_refresh = _manual_refresh;

    label_value = [this](double v)
    {
        return QString::number(value_float, 'f', slider_float_decimals);
    };

    // Default values
    //value_int_ptr = &value_int;
    //value_float_ptr.push_back(&value_float);
    //value_bool_ptr = &value_bool;
    value_string_ptr = &value_string;
    value_combo_selected_ptr = &value_combo_selected;

    slider_float_step_ptr = &slider_float_step;
    slider_float_min_ptr = &slider_float_min;
    slider_float_max_ptr = &slider_float_max;

    slider_int_step_ptr = &slider_int_step;
    slider_int_min_ptr = &slider_int_min;
    slider_int_max_ptr = &slider_int_max;



    // Add Layout to Frame
    layout = new QVBoxLayout(this);
    layout->setSpacing(0);
    layout->setContentsMargins(0,0,0,0);

    // Create frame
    QFrame* frame = new QFrame(this);
    frame->setFrameShape(QFrame::Box);
    frame->setFrameShadow(QFrame::Raised);
    frame->setStyleSheet("background: #292942;");
    frame->setLineWidth(1);

    // Create inner layout for components
    QVBoxLayout* frameLayout = new QVBoxLayout(frame);
    frameLayout->setContentsMargins(5,5,5,5);

    name_lbl = new QLabel(this);
    name_lbl->setText(name);
    frameLayout->addWidget(name_lbl);
    frameLayout->setContentsMargins(5,5,5,5);
    //frameLayout->addSpacing(1);

    switch (type)
    {
        case AttributeType::SLIDER_INT:
        {
            // Make horizontal wrapper layout for the input
            QHBoxLayout* wrapper = new QHBoxLayout(this);
            wrapper->setSpacing(10);
            frameLayout->addLayout(wrapper);

            // Place input items inside the wrapper layout
            QSlider* slider = new QSlider(Qt::Orientation::Horizontal, this);
            val_lbl = new QLabel(this);
            wrapper->addWidget(slider, 1);
            wrapper->addWidget(val_lbl, 0);

            input = slider;

            // Set input value
            slider->setValue(value_int);
            //val_lbl->setText(QString::number(value_int));

            // Handle change
            connect(slider, &QSlider::valueChanged, this, [this](int v)
            {
                // Update value
                value_int = v;
                
                if (!manual_refresh)
                {
                    for (int*& ptr : int_ptrs)
                        *ptr = v;

                    // Invoke callback
                    if (float_change)
                        float_change(v);
                }

                updateUIValue();
            });
        }
        break;

        case AttributeType::SLIDER_FLOAT:
        {
            // Make horizontal wrapper layout for the input
            QHBoxLayout* h_layout = new QHBoxLayout(this);
            h_layout->setSpacing(10);
            frameLayout->addLayout(h_layout);

            // Place input items inside the wrapper layout
            QSlider* slider = new QSlider(Qt::Orientation::Horizontal, this);
            val_lbl = new QLabel(this);
            h_layout->addWidget(slider, 1);
            h_layout->addWidget(val_lbl, 0);

            input = slider;

            // Set input text
            slider->setValue(value_float / *slider_float_step_ptr);

            // Handle change
            connect(slider, &QSlider::valueChanged, this, [this](int v)
            {
                double f = ((double)v) * *slider_float_step_ptr;

                // Update values
                value_float = f;
                if (!manual_refresh)
                {
                    for (double*& ptr : float_ptrs)
                        *ptr = f;

                    // Invoke callback
                    if (float_change)
                        float_change(f);
                }

                updateUIValue();
            });
        }
        break;

        case AttributeType::INPUT_INT:
        {
            QSpinBox* spinBox = new QSpinBox(this);
            input = spinBox;

            // Set input / value label text
            //spinBox->setValue(value_float);
            frameLayout->addWidget(spinBox);

            // Handle change
            connect(spinBox, &QSpinBox::valueChanged, this, [this](int v)
            {
                // Update value
                //value_float = v;

                // Invoke callback
                if (float_change)
                    float_change(v);

                updateUIValue();
            });
        }
        break;

        case AttributeType::INPUT_FLOAT:
        {
            QDoubleSpinBox* spinBox = new QDoubleSpinBox(this);
            input = spinBox;

            // Set input / value label text
            //spinBox->setValue(value_float);
            frameLayout->addWidget(spinBox);

            // Handle change
            connect(spinBox, &QDoubleSpinBox::valueChanged, this, [this](double v)
            {
                // Update value
                //value_float = v;

                // Invoke callback
                if (float_change)
                    float_change(v);

                updateUIValue();
            });
        }
        break;

        case AttributeType::CHECKBOX:
        {
            QCheckBox* checkBox = new QCheckBox(this);
            input = checkBox;

            // Set input / value label text
            //spinBox->setValue(value_float);
            frameLayout->setDirection(QBoxLayout::LeftToRight);
            frameLayout->addStretch(1);
            frameLayout->addWidget(checkBox);

            // Handle change
            connect(checkBox, &QCheckBox::toggled, this, [this](bool b)
            {
                // Update value
                value_bool = b;

                if (!manual_refresh)
                {
                    for (bool*& ptr : bool_ptrs)
                        *ptr = b;

                    // Invoke callback
                    if (bool_change)
                        bool_change(b);
                }

                updateUIValue();
            });
        }
        break;

        case AttributeType::BUTTON:
        {

        }
        break;

        case AttributeType::COMBOBOX:
        {
            QComboBox* comboBox = new QComboBox(this);
            input = comboBox;

            // Set input / value label text
            //spinBox->setValue(value_float);
            frameLayout->addWidget(comboBox);

            // Handle change
            connect(comboBox, &QComboBox::currentIndexChanged, this, [this](int i)
            {
                // Update value
                //value_combo_selected = i;

                // Invoke callbacks
                if (int_change)
                    int_change(i);

                if (combo_items[i].second)
                    combo_items[i].second();
            });
        }
        break;
    }

    input->setContentsMargins(2,2,2,2);
    layout->addWidget(frame);
    //setLayout(layout);
}

/*void AttributeItem::forceRefreshPointers()
{
    switch (type)
    {
    case AttributeType::SLIDER_INT:
    {
    }
    break;
    case AttributeType::INPUT_INT:
    {
    }
    break;

    case AttributeType::SLIDER_FLOAT:
    {
        for (double*& ptr : value_float_ptr)
            *ptr = value_float;
    }
    break;

    case AttributeType::INPUT_FLOAT:
    {
    }
    break;

    case AttributeType::CHECKBOX:
    {
    }
    break;
    }
}*/

std::vector<void*> AttributeItem::getValuePointers()
{
    switch (type)
    {
        case AttributeType::SLIDER_INT:
        case AttributeType::INPUT_INT:
        {
            return std::vector<void*>(int_ptrs.begin(), int_ptrs.end());
        }
        break;

        case AttributeType::SLIDER_FLOAT:
        case AttributeType::INPUT_FLOAT:
        {
            return std::vector<void*>(float_ptrs.begin(), float_ptrs.end());
        }
        break;

        case AttributeType::CHECKBOX:
        {
            return std::vector<void*>(bool_ptrs.begin(), bool_ptrs.end());
        }
        break;
    }
    return {};
}

void AttributeItem::removePointer(void* ptr)
{
    switch (type)
    {
        case AttributeType::SLIDER_INT:
        case AttributeType::INPUT_INT:
        {
            int_ptrs.erase(std::find(int_ptrs.begin(), int_ptrs.end(), ptr));
        }
        break;

        case AttributeType::SLIDER_FLOAT:
        case AttributeType::INPUT_FLOAT:
        {
            float_ptrs.erase(std::find(float_ptrs.begin(), float_ptrs.end(), ptr));
        }
        break;

        case AttributeType::CHECKBOX:
        {
            bool_ptrs.erase(std::find(bool_ptrs.begin(), bool_ptrs.end(), ptr));
        }
        break;
    }
}

void AttributeItem::removeAllPointers()
{
    std::vector<void*> ptrs = getValuePointers();
    for (void* ptr : ptrs)
        removePointer(ptr);
}

void AttributeItem::updateUIValue()
{
    switch (type)
    {
    case AttributeType::SLIDER_INT:
    {
        QSlider* slider = (QSlider*)input;

        slider->setSingleStep(*slider_int_step_ptr);

        // Block value callback while setting range to avoid overriding value
        slider->blockSignals(true);
        slider->setRange(*slider_int_min_ptr, *slider_int_max_ptr);
        slider->blockSignals(false);

        QString users_txt = QString(" (%1 users)").arg(int_ptrs.size());

        slider->setValue(value_int);
        name_lbl->setText(name + users_txt);
        val_lbl->setText(QString::number(value_int));
    }
    break;
    case AttributeType::INPUT_INT:
    {
        QSpinBox* slider = (QSpinBox*)input;
        slider->setValue(value_int);
    }
    break;

    case AttributeType::SLIDER_FLOAT:
    {
        QSlider* slider = (QSlider*)input;

        int iStep = (int)(1.0 / *slider_float_step_ptr);

        slider->setSingleStep(iStep);

        // Block value callback while setting range to avoid overriding value
        slider->blockSignals(true);
        slider->setRange(
            *slider_float_min_ptr / *slider_float_step_ptr,
            *slider_float_max_ptr / *slider_float_step_ptr
        );
        slider->blockSignals(false);


        slider_float_decimals = getDecimalPlaces(*slider_float_step_ptr);

        QString users_txt = QString(" (%1 users)").arg(float_ptrs.size());

        slider->setValue(value_float / *slider_float_step_ptr);
        name_lbl->setText(name + users_txt);
        val_lbl->setText(label_value(value_float));
    }
    break;
    case AttributeType::INPUT_FLOAT:
    {
        QSpinBox* spinBox = (QSpinBox*)input;
        //spinBox->setValue(*value_float_ptr);
        spinBox->setValue(value_float);
    }
    break;
    case AttributeType::CHECKBOX:
    {
        QCheckBox* checkBox = (QCheckBox*)input;

        QString users_txt = QString(" (%1 users)").arg(bool_ptrs.size());
        name_lbl->setText(name + users_txt);
        checkBox->setChecked(value_bool);
    }
    break;
    }
}



/*AttributeItem* AttributeItem::updateValue()
{
    //value_int = value;

    switch (type)
    {
    case AttributeType::SLIDER_INT:
    {
        QSlider* slider = (QSlider*)input;
        slider->setValue(value);
        val_lbl->setText(QString::number(value));
    }
    break;
    case AttributeType::INPUT_INT:
    {
        QSpinBox* spinBox = (QSpinBox*)input;
        spinBox->setValue(value);
    }
    break;
    }
    return this;
}

AttributeItem* AttributeItem::setValue(double value)
{
    // Update value
    value_float = value;

    switch (type)
    {
    case AttributeType::SLIDER_FLOAT:
    {
        QSlider* slider = (QSlider*)input;
        slider->setValue(value / slider_step);
        val_lbl->setText(QString::number(value, 'f', float_decimals));
    }
    break;
    case AttributeType::INPUT_FLOAT:
    {
        QSpinBox* spinBox = (QSpinBox*)input;
        spinBox->setValue(value);
    }
    break;
    }
    return this;
}

AttributeItem* AttributeItem::setRange(int min, int max, int step)
{
    switch (type)
    {
    case AttributeType::SLIDER_INT:
    {
        QSlider* slider = (QSlider*)input;
        slider->setSingleStep(step);
        slider->setRange(min, max);
    }
    break;
    case AttributeType::INPUT_INT:
    {
        QSpinBox* spinBox = (QSpinBox*)input;
        spinBox->setSingleStep(step);
        spinBox->setRange(min, max);
    }
    break;
    }
    return this;
}

AttributeItem* AttributeItem::setRange(double min, double max, double step)
{
    switch (type)
    {
    case AttributeType::SLIDER_FLOAT:
    {
        slider_step = step;
        float_decimals = getDecimalPlaces(step);

        QSlider* slider = (QSlider*)input;
        int iStep = (int)(1.0 / step);
        slider->setSingleStep(iStep);
        slider->setRange(min / step, max / step);
    }
    break;
    case AttributeType::INPUT_FLOAT:
    {
        //slider_step = step;
        //float_decimals = getDecimalPlaces(step);

        QDoubleSpinBox* spinBox = (QDoubleSpinBox*)input;
        spinBox->setSingleStep(step);
        spinBox->setRange(min, max);
    }
    break;
    }
    return this;
}*/

AttributeItem* AttributeItem::addComboItem(QString text, std::function<void(void)> callback)
{
    QComboBox* comboBox = (QComboBox*)input;
    combo_items.push_back(std::pair(text, [this, callback]()
    {
        if (callback)
            callback();

        for (auto& item : attributeList->item_widgets)
        {
            item->updateUIValue();
        }
    }));
    comboBox->addItem(text);
    return this;
}

void AttributeItem::setActiveComboItem(QString text)
{
    for (size_t i = 0; i < combo_items.size(); i++)
    {
        if (combo_items[i].first == text)
        {
            QComboBox* comboBox = (QComboBox*)input;
            comboBox->setCurrentIndex((int)i);
            combo_items[i].second();
        }
    }
}

QString AttributeItem::getActiveComboItem()
{
    QComboBox* comboBox = (QComboBox*)input;
    return comboBox->currentText();
}
