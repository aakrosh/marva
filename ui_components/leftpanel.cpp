#include "leftpanel.h"
#include "ui_leftpanel.h"

LeftPanel::LeftPanel(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LeftPanel)
{
    ui->setupUi(this);
    ui->leftSplitter->setSizes(QList<int>() << 4000 << 1000);
}

LeftPanel::~LeftPanel()
{
    delete ui;
}

TaxListWidget *LeftPanel::taxList() { return ui->taxList; }

CurrentTaxNodeDetails *LeftPanel::curNodeDetails() { return ui->curNode; }
