#include "bubblechartproperties.h"
#include "ui_bubblechartproperties.h"
#include "datasourcesmodel.h"
#include "bubblechartview.h"

BubbleChartProperties::BubbleChartProperties(QWidget *parent, BubbleChartParameters *_config, BlastTaxDataProviders *dp) :
    QDialog(parent),
    config(_config),
    ui(new Ui::BubbleChartProperties)
{
    ui->setupUi(this);
    ui->lvDataSources->setModel(new DataSourcesModel(this, dp));
    setValues();
    connect(ui->horizontalSlider, SIGNAL(valueChanged(int)), this, SLOT(onBubbleMaxSizeSliderValueChanged(int)));
    connect(ui->cbShowTitle, SIGNAL(toggled(bool)), this, SLOT(onShowTitleToggled(bool)));
    connect(ui->lvDataSources->model(), SIGNAL(dataChanged(QModelIndex,QModelIndex,QVector<int>)), this, SLOT(onDataSourceCheckBoxTriggered(QModelIndex,QModelIndex,QVector<int>)));
}

void BubbleChartProperties::setValues()
{
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

void BubbleChartProperties::onDataSourceCheckBoxTriggered(QModelIndex start, QModelIndex, QVector<int>)
{
    emit dataSourceVisibilityChanged(start.row());
}
