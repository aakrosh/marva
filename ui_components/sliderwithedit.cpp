#include "sliderwithedit.h"
#include "ui_sliderwithedit.h"

SliderWithEdit::SliderWithEdit(QWidget *parent) :
    QWidget(parent),
    oldValue(-1),
    ui(new Ui::SliderWithEdit)
{
    ui->setupUi(this);
}

SliderWithEdit::~SliderWithEdit()
{
    delete ui;
}

int SliderWithEdit::maxValue()
{
    return ui->spinBox->maximum();
}

int SliderWithEdit::value()
{
    return ui->spinBox->value();
}

void SliderWithEdit::setLabel(QString label)
{
    ui->label->setText(label);
}

void SliderWithEdit::setMaxValue(int maxVal)
{
    ui->spinBox->setMaximum(maxVal);
    ui->horizontalSlider->setMaximum(maxVal);
}

void SliderWithEdit::setMinValue(int minVal)
{
    ui->spinBox->setMinimum(minVal);
    ui->horizontalSlider->setMinimum(minVal);
}

void SliderWithEdit::setValue(int val)
{
    ui->spinBox->setValue(val); //slider will be updated by signal from spinbox
}

void SliderWithEdit::onValueChanged(int val)
{
    if ( val == oldValue )
        return;
    emit valueChanged((quint32)oldValue, (quint32)val);
    oldValue = val;
}


