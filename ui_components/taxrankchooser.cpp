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

void TaxRankChooser::setRank(TaxRank rank)
{
    int arsize = sizeof(ranks)/sizeof(TaxRank);
    for ( int r = 0; r < arsize; r++ )
    {
        if ( ranks[r] == rank )
        {
            ui->comboBox->setCurrentIndex(r);
            return;
        }
    }
}

void TaxRankChooser::onIndexChanged(int index)
{
    emit selectedTaxRankChanged(ranks[index]);
}

