#ifndef LABELEDDOUBLESPINBOX_H
#define LABELEDDOUBLESPINBOX_H

#include <QWidget>

namespace Ui {
class LabeledDoubleSpinBox;
}

class LabeledDoubleSpinBox : public QWidget
{
    Q_OBJECT

public:
    explicit LabeledDoubleSpinBox(QWidget *parent = 0);
    ~LabeledDoubleSpinBox();
    void setValue(qreal val);
    void setMaxValue(qreal val);
    void setReadOnly(bool ro);
    void setLabel(const char *l);
    qreal value();
    qreal maxValue();

private:
    Ui::LabeledDoubleSpinBox *ui;
    quint32 oldValue;

private slots:
    void onValueChanged(double newVal);

signals:
    valueChanged(quint32, quint32);
};

#endif // LABELEDDOUBLESPINBOX_H
