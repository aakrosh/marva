#include "bubblechartproperties.h"
#include "ui_bubblechartproperties.h"
#include "datasourcesmodel.h"
#include "bubblechartview.h"
#include "config.h"

BubbleChartProperties::BubbleChartProperties(QWidget *parent, BubbleChartParameters *_config, BlastTaxDataProviders *dp) :
    QDialog(parent),
    config(_config),
    ui(new Ui::BubbleChartProperties)
{
    ui->setupUi(this);
    DataSourcesModel *model = new DataSourcesModel(this, dp);
    connect(model, SIGNAL(rowMoved(int, int)), this, SLOT(onDataSourceRowsMoved(int, int)));
    //connect(model, SIGNAL(rowsMoved(const QModelIndex &, int, int, const QModelIndex &, int)), this, SLOT(onDataSourceRowsMoved(int, int)));
    ui->lvDataSources->setModel(model);
    ui->lvDataSources->viewport()->setAcceptDrops(true);
    ui->lvDataSources->setDropIndicatorShown(true);
    ui->lvDataSources->setDragDropMode(QAbstractItemView::InternalMove);
    ui->lvDataSources->setDragEnabled(true);
    ui->lvDataSources->setAcceptDrops(true);

    setValues();
    connect(ui->horizontalSlider, SIGNAL(valueChanged(int)), this, SLOT(onBubbleMaxSizeSliderValueChanged(int)));
    connect(ui->cbShowTitle, SIGNAL(toggled(bool)), this, SLOT(onShowTitleToggled(bool)));
    connect(ui->lvDataSources->model(), SIGNAL(dataChanged(QModelIndex,QModelIndex,QVector<int>)), this, SLOT(onDataSourceCheckBoxTriggered(QModelIndex,QModelIndex,QVector<int>)));
    connect(ui->rbLinear, SIGNAL(toggled(bool)), this, SLOT(onBubbleSizeCalcMethodChanged(bool)));
    connect(ui->horIntervalSlider, SIGNAL(valueChanged(int)), this, SLOT(onHorIntervalChanged(int)));
    connect(ui->vertIntervalSlider, SIGNAL(valueChanged(int)), this, SLOT(onVertIntervalChanged(int)));
    connect(ui->bubbleTransparancySlider, SIGNAL(valueChanged(int)), this, SLOT(onTransparancyChanged(int)));
    connect(ui->cbNormalized, SIGNAL(toggled(bool)), this, SLOT(onNormalizedToggled(bool)));
    connect(ui->cbShowGrid, SIGNAL(toggled(bool)), this, SLOT(onShowGridToggled(bool)));
}

void BubbleChartProperties::setValues()
{
    ui->horizontalSlider->setValue(config->bubbleSize);
    ui->cbShowTitle->setChecked(config->showTitle);
    bool isLinear = config->calcMethod == METHOD_LINEAR;
    ui->rbLinear->setChecked(isLinear);
    ui->rbSqrt->setChecked(!isLinear);
    ui->horIntervalSlider->setValue(config->horInterval);
    ui->vertIntervalSlider->setValue(config->vertInterval);
    ui->bubbleTransparancySlider->setValue(config->bubbleTransparancy);
    ui->cbNormalized->setChecked(config->normalized);
    ui->cbShowGrid->setChecked(config->showGrid);
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

void BubbleChartProperties::onHorIntervalChanged(int val)
{
    config->horInterval = val;
    emit horIntervalChanged(val);
}

void BubbleChartProperties::onVertIntervalChanged(int val)
{
    config->vertInterval = val;
    emit vertIntervalChanged(val);
}

void BubbleChartProperties::onTransparancyChanged(int val)
{
    config->bubbleTransparancy = val;
    emit bubbleTransparancyChanged(val);
}

void BubbleChartProperties::onShowTitleToggled(bool val)
{
    config->showTitle = val;
    emit showTitleToggled(val);
}

void BubbleChartProperties::onNormalizedToggled(bool val)
{
    config->normalized = val;
    emit normalizedChanged(val);
}

void BubbleChartProperties::onShowGridToggled(bool val)
{
    config->showGrid = val;
    emit showGridChanged(val);
}

void BubbleChartProperties::onDataSourceCheckBoxTriggered(QModelIndex start, QModelIndex, QVector<int>)
{
    emit dataSourceVisibilityChanged(start.row());
}

void BubbleChartProperties::onBubbleSizeCalcMethodChanged(bool isLinear)
{
    config->calcMethod = isLinear ? METHOD_LINEAR : METHOD_SQRT;
    emit bubbleSizeCalcMethodChanged(config->calcMethod);
}

void BubbleChartProperties::onDataSourceRowsMoved(int start, int row)
{
    emit dataSourceMoved(start, row);
}
