#include "taxrankchooser.h"
#include "ui_taxrankchooser.h"

static TaxRank ranks[] = {TR_NORANK, TR_KINGDOM, TR_PHYLUM, TR_CLASS, TR_ORDER, TR_FAMILY, TR_VARIETAS, TR_SPECIES};

TaxRankChooser::TaxRankChooser(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TaxRankChooser)
{
    ui->setupUi(this);    
}

TaxRankChooser::~TaxRankChooser()
{
    delete ui;
}

TaxRank TaxRankChooser::rank()
{
    return ranks[ui->comboBox->currentIndex()];
}

void TaxRankChooser::onIndexChanged(int index)
{
    emit selectedTaxRankChanged(ranks[index]);
}

