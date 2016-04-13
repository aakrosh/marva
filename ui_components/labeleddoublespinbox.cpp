#include "labeleddoublespinbox.h"
#include "ui_labeleddoublespinbox.h"

LabeledDoubleSpinBox::LabeledDoubleSpinBox(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LabeledDoubleSpinBox),
    oldValue(0)
{
    ui->setupUi(this);
    connect(ui->sb, SIGNAL(valueChanged(double)), this, SLOT(onValueChanged(double)));
}

LabeledDoubleSpinBox::~LabeledDoubleSpinBox()
{
    delete ui;
}

void LabeledDoubleSpinBox::setValue(qreal val)
{
    ui->sb->setValue(val);
}

void LabeledDoubleSpinBox::setMaxValue(qreal val)
{
    ui->sb->setMaximum(val);
}

void LabeledDoubleSpinBox::setReadOnly(bool ro)
{
    ui->sb->setReadOnly(ro);
}

void LabeledDoubleSpinBox::setLabel(const char *l)
{
    ui->label->setText(l);
}

qreal LabeledDoubleSpinBox::value() { return ui->sb->value(); }

qreal LabeledDoubleSpinBox::maxValue() { return ui->sb->maximum(); }

void LabeledDoubleSpinBox::onValueChanged(double newVal)
{
    emit valueChanged((quint32) oldValue, (quint32)newVal);
    oldValue = newVal;
}
