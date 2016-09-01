#include "taxrankchooser.h"
#include "ui_taxrankchooser.h"

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

void TaxRankChooser::onIndexChanged(int index)
{
    static TaxRank ranks[] = {TR_NORANK, TR_KINGDOM, TR_PHYLUM, TR_CLASS, TR_ORDER, TR_FAMILY, TR_VARIETAS, TR_SPECIES};
    emit selectedTaxRankChanged(ranks[index]);
}

