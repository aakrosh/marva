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
    switch ( gnc->showTitle() )
    {
        case SHOW_TITLE_ALL:   ui->rbShowTitleAll->setChecked(true);  break;
        case SHOW_TITLE_MAIN:  ui->rbShowTitleMain->setChecked(true); break;
        case SHOW_TITLE_NONE:  ui->rbShowTitleNone->setChecked(true); break;
    }
    ui->sbMaxTitleLen->setValue(gnc->nodeTitleLen());

    ImportDataConfig *idc = configuration->ImportData();
    ui->sbMinBitscore->setValue(idc->minBitscore());
    ui->leGiToTaxMapPath->setText(idc->gi2taxmap());
    ui->sbTopPercent->setValue(idc->topPercent());
    ui->sbMaxEValue->setValue(idc->maxEValue());

    inited = true;

    connect(ui->bGiToTaxMapPath, SIGNAL(clicked(bool)), this, SLOT(onGiToTaxMapPathClicked()));

    InitializationConfig *ic = configuration->Initialization();
    ui->leTaxTree->setText(ic->taxTreePath());
    ui->leTaxMap->setText(ic->taxMapPath());
    connect(ui->bTaxTree, SIGNAL(clicked(bool)), this, SLOT(onTaxTreePathClicked()));
    connect(ui->bTaxMap, SIGNAL(clicked(bool)), this, SLOT(onTaxMapPathClicked()));


    ui->leftList->setCurrentRow(0);
}

ConfigurationDialog::~ConfigurationDialog()
{
    delete ui;
}

void ConfigurationDialog::closeEvent(QCloseEvent *)
{
    onConfigChanged();
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
    int st = SHOW_TITLE_MAIN;
    if ( ui->rbShowTitleAll->isChecked() )
        st = SHOW_TITLE_ALL;
    else if ( ui->rbShowTitleNone->isChecked() )
        st = SHOW_TITLE_NONE;
    gnc->setshowTitle(st);
    gnc->setnodeTitleLen(ui->sbMaxTitleLen->value());

    ImportDataConfig *idc = configuration->ImportData();
    idc->setminBitscore(ui->sbMinBitscore->value());
    idc->settopPercent(ui->sbTopPercent->value());
    idc->setmaxEValue(ui->sbMaxEValue->value());
    idc->setgi2taxmap(ui->leGiToTaxMapPath->text());

    InitializationConfig *ie = configuration->Initialization();
    ie->settaxMapPath(ui->leTaxMap->text());
    ie->settaxTreePath(ui->leTaxTree->text());

    configuration->save();

    emit configChanged();
}

void ConfigurationDialog::onGiToTaxMapPathClicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Select GI to taxonomy map file"),
                                                          ui->leGiToTaxMapPath->text(),
                                                          tr("Map files (*.bin *.map)"));
    if ( !fileName.isEmpty() )
        ui->leGiToTaxMapPath->setText(fileName);
}

void ConfigurationDialog::onTaxTreePathClicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Select taxonomy tree file"),
                                                          ui->leTaxTree->text(),
                                                          tr("Tree files (*.tre *.tree *.txt);;All files(*)"));
    if ( !fileName.isEmpty() )
        ui->leTaxTree->setText(fileName);
}
void ConfigurationDialog::onTaxMapPathClicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Select taxonomy map file"),
                                                          ui->leTaxMap->text(),
                                                          tr("Map files (*.map *.txt);;All files(*)"));
    if ( !fileName.isEmpty() )
        ui->leTaxMap->setText(fileName);
}
