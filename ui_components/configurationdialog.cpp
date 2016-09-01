#include "configurationdialog.h"
#include "ui_configurationdialog.h"
#include "config.h"

#include <QFileDialog>

ConfigurationDialog::ConfigurationDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConfigurationDialog)
{
    inited = false;
    ui->setupUi(this);

    BubbleChartConfig *bcc = configuration->BubbleChart();
    ui->sbMaxChartTax->setValue(bcc->maxChartTaxes());
    ui->spDefaultVisibleChartTaxes->setValue(bcc->defaultVisibleChartTaxes());
    ui->sbDefaultMaxBubbleSize->setValue(bcc->defaultMaxBubbleSize());

    GraphNodeConfig *gnc = configuration->GraphNode();
    ui->sbNodeCircleSize->setValue(gnc->nodeCircleSize());
    ui->sbPlusSignSize->setValue(gnc->halfPlusSize()*2);
    ui->sbMaxNodeRadius->setValue(gnc->maxNodeRadius());
    ui->rbCurves->setChecked(gnc->edgeStyle() == EDGE_CURVE);

    ImportDataConfig *idc = configuration->ImportData();
    ui->sbMinBitscore->setValue(idc->minBitscore());
    ui->leGiToTaxMapPath->setText(idc->gi2taxmap());
    ui->sbTopPercent->setValue(idc->topPercent());

    inited = true;

    connect(ui->bGiToTaxMapPath, SIGNAL(clicked(bool)), this, SLOT(onGiToTaxMapPathClicked()));

    ui->leftList->setCurrentRow(0);
}

ConfigurationDialog::~ConfigurationDialog()
{
    delete ui;
}

void ConfigurationDialog::closeEvent(QCloseEvent *)
{
    //configuration->save();
}

void ConfigurationDialog::onConfigChanged()
{
    if ( !inited )
        return;
    BubbleChartConfig *bcc = configuration->BubbleChart();
    bcc->setmaxChartTaxes(ui->sbMaxChartTax->value());
    bcc->setdefaultVisibleChartTaxes(ui->spDefaultVisibleChartTaxes->value());
    bcc->setdefaultMaxBubbleSize(ui->sbDefaultMaxBubbleSize->value());

    GraphNodeConfig *gnc = configuration->GraphNode();
    gnc->setnodeCircleSize(ui->sbNodeCircleSize->value());
    gnc->sethalfPlusSize(ui->sbPlusSignSize->value()/2);
    gnc->setmaxNodeRadius(ui->sbMaxNodeRadius->value());
    gnc->setedgeStyle(ui->rbCurves->isChecked() ? EDGE_CURVE : EDGE_LINE);

    ImportDataConfig *idc = configuration->ImportData();
    idc->setminBitscore(ui->sbMinBitscore->value());
    idc->settopPercent(ui->sbTopPercent->value());
    idc->setgi2taxmap(ui->leGiToTaxMapPath->text());

    configuration->save();
}

void ConfigurationDialog::onGiToTaxMapPathClicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Select GI to taxonomy map file"),
                                                         ui->leGiToTaxMapPath->text(),
                                                          tr("Map files (*.bin *.map)"));
    if ( !fileName.isEmpty() )
        ui->leGiToTaxMapPath->setText(fileName);
}
