#include "bubblechartproperties.h"
#include "ui_bubblechartproperties.h"

BubbleChartProperties::BubbleChartProperties(QWidget *parent, BubbleChartConfig *_config) :
    QDialog(parent),
    config(_config),
    ui(new Ui::BubbleChartProperties)
{
    ui->setupUi(this);
    setValues();
    connect(ui->horizontalSlider, SIGNAL(valueChanged(int)), this, SLOT(onBubbleMaxSizeSliderValueChanged(int)));
    connect(ui->cbShowTitle, SIGNAL(toggled(bool)), this, SLOT(onShowTitleToggled(bool)));
}

void BubbleChartProperties::setValues()
{
    if ( config->maxBubbleSize != 0)
        ui->horizontalSlider->setMaximum(config->maxBubbleSize);
    ui->horizontalSlider->setValue(config->bubbleSize);
    ui->cbShowTitle->setChecked(config->showTitle);
}

BubbleChartProperties::~BubbleChartProperties()
{
    delete ui;
}

void BubbleChartProperties::onBubbleMaxSizeSliderValueChanged(int val)
{
    config->bubbleSize = val;
    emit maxBubbleSizeChanged(val);
}

void BubbleChartProperties::onShowTitleToggled(bool val)
{
    config->showTitle = val;
    emit showTitleToggled(val);
}
