#include "configurationdialog.h"
#include "ui_configurationdialog.h"
#include "config.h"

ConfigurationDialog::ConfigurationDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConfigurationDialog)
{
    inited = false;
    ui->setupUi(this);
    ui->leftList->addItem("Bubble charts");
    ui->leftList->addItem("Tree views");

    BubbleChartConfig *bcc = configuration->BubbleChart();
    ui->sbMaxChartTax->setValue(bcc->maxChartTaxes());
    ui->spDefaultVisibleChartTaxes->setValue(bcc->defaultVisibleChartTaxes());
    ui->sbDefaultMaxBubbleSize->setValue(bcc->defaultMaxBubbleSize());

    GraphNodeConfig *gnc = configuration->GraphNode();
    ui->sbNodeCircleSize->setValue(gnc->nodeCircleSize());
    ui->sbPlusSignSize->setValue(gnc->halfPlusSize()*2);
    ui->sbMaxNodeRadius->setValue(gnc->maxNodeRadius());

    inited = true;

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

    configuration->save();
}
