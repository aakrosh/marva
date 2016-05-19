#include "bubblechartproperties.h"
#include "ui_bubblechartproperties.h"

BubbleChartProperties::BubbleChartProperties(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::BubbleChartProperties)
{
    ui->setupUi(this);
    connect(ui->horizontalSlider, SIGNAL(valueChanged(int)), this, SLOT(onBubbleMaxSizeSliderValueChanged(int)));
}

void BubbleChartProperties::setValues(quint32 value, quint32 maxValue)
{
    if ( maxValue != 0)
        ui->horizontalSlider->setMaximum(maxValue);
    ui->horizontalSlider->setValue(value);
}

BubbleChartProperties::~BubbleChartProperties()
{
    delete ui;
}

void BubbleChartProperties::onBubbleMaxSizeSliderValueChanged(int val)
{
    emit maxBubbleSizeChanged(val);
}
