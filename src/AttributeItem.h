#pragma once

#include <functional>
#include <variant>

#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSpacerItem>

#include <QLabel>
#include <QPushButton>
#include <QCheckBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QSlider>
#include <QComboBox>

enum AttributeType
{
    SLIDER_INT,
    SLIDER_FLOAT,
    INPUT_INT,
    INPUT_FLOAT,
    INPUT_STRING,
    CHECKBOX,
    BUTTON,
    COMBOBOX
};

class AttributeList;
class AttributeItem : public QWidget
{
    Q_OBJECT;

    QVBoxLayout* layout;

    AttributeType type;
    QWidget* input;
    QLabel* name_lbl;
    QLabel* val_lbl;
    bool manual_refresh;

public:

    QString name;
    //bool touched;

    AttributeList* attributeList;

    std::function<QString(double)> label_value;

    // Calculated automatically
    int slider_float_decimals = 1;

    // Placeholder values, ptr's use these if not explicitly set
    int value_int = 0;
    double value_float = 0.0;
    QString value_string = "";
    bool value_bool = false;
    int value_combo_selected = 0;

    double slider_float_step = 0.1;
    double slider_float_min = 0;
    double slider_float_max = 1;

    int slider_int_step = 1;
    int slider_int_min = 0;
    int slider_int_max = 100;

    // Pointers to change
    std::vector<int*> int_ptrs;
    std::vector<double*> float_ptrs;
    std::vector<bool*> bool_ptrs;
    QString* value_string_ptr;
    int* value_combo_selected_ptr;

    double* slider_float_step_ptr;
    double* slider_float_min_ptr;
    double* slider_float_max_ptr;

    int *slider_int_step_ptr;
    int *slider_int_min_ptr;
    int *slider_int_max_ptr;

    

    //double* slider_step_ptr;// = 0.1;
    //int* float_decimals_ptr;// = 1;
    //int float_decimals = 1;
   
    

    std::vector<std::pair<QString, std::function<void(void)>>> combo_items;

    explicit AttributeItem(
        const QString &name,
        AttributeType _type,
        bool manual_refresh,
        QWidget* parent = nullptr);

    std::function<void(int)>    int_change;
    std::function<void(double)> float_change;
    std::function<void(bool)>   bool_change;

    //void setValuePtr(int* value);
    void updateUIValue();
    //void forceRefreshPointers();

    std::vector<void*> getValuePointers();
    void removePointer(void* ptr);
    void removeAllPointers();
    //AttributeItem* updateValue(int value);

    //AttributeItem* updateValue(int value);
    //AttributeItem* updateValue(double value);
    //AttributeItem* updateRange(int min, int max, int step=1);
    //AttributeItem* updateRange(double min, double max, double step);

    /*struct AttributeItemSnapshot
    {
        QString name;
        std::vector<void*> ptrs;
    };

    AttributeItemSnapshot snapshot;
    void garbageTakePriorSnapshot()
    {
        snapshot.name = name;
        snapshot.ptrs = getValuePointers();
    }

    void garbageRemoveUnreferencedPointers()
    {
        std::vector<void*> current_ptrs = getValuePointers();
        for (void* ptr : snapshot.ptrs)
        {
            if (std::find(current_ptrs.begin(), current_ptrs.end(), ptr) != current_ptrs.end())
                removePointer(ptr);
        }
    }*/

    AttributeItem* addComboItem(QString text, std::function<void(void)> callback = nullptr);
    void setActiveComboItem(QString text);
    QString getActiveComboItem();
};