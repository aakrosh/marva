#include "currenttaxnodedetails.h"
#include "ui_currenttaxnodedetails.h"
#include "base_tax_node.h"
#include "blast_data.h"
#include "colors.h"
#include "taxnodesignalsender.h"
#include "taxdataprovider.h"

//=========================================================================
CurrentTaxNodeDetails::CurrentTaxNodeDetails(QWidget *parent) :
    QWidget(parent),
    curNodeId(-1),
    dataProvider(NULL),
    ui(new Ui::CurrentTaxNodeDetails)
{
    ui->setupUi(this);
    TaxNodeSignalSender *tnss = getTaxNodeSignalSender(NULL);
    connect(tnss, SIGNAL(colorChanged(BaseTaxNode*)), this, SLOT(onColorChanged(BaseTaxNode*)));
}

//=========================================================================
CurrentTaxNodeDetails::~CurrentTaxNodeDetails()
{
    delete ui;
}

//=========================================================================
void CurrentTaxNodeDetails::setTaxDataProvider(TaxDataProvider *tdp)
{
    dataProvider = tdp;
    refresh();
}

//=========================================================================
void CurrentTaxNodeDetails::refresh()
{
    if ( curNodeId < 0 )
        return;
    ui->lId->setText(QString::number(curNodeId));
    if ( dataProvider == NULL )
        return;
    quint32 index = dataProvider->indexOf(curNodeId);
    if ( index == quint32(-1) )
        return;
    quint32 reads = dataProvider->reads(index);
    quint32 sum = dataProvider->sum(index);
    BaseTaxNode *bNode = dataProvider->taxNode(index);

    QString rank;
    TaxRank tr = taxMap.value(curNodeId)->getRank();
    switch ( tr )
    {
        case TR_DOMAIN: rank = "Domain"; break;
        case TR_KINGDOM: rank = "Kingdom"; break;
        case TR_PHYLUM: rank = "Phylum"; break;
        case TR_CLASS: rank = "Class"; break;
        case TR_ORDER: rank = "Order"; break;
        case TR_FAMILY: rank = "Family"; break;
        case TR_VARIETAS: rank = "Genus"; break;
        case TR_SPECIES: rank = "Species"; break;
        default: break;
    }
    rank.append(QString("(%1)").arg((qint32)tr));
    ui->lRank->setText(rank);

    if ( curNodeId > 1 )
    {
        QString text = bNode->getText();
        if ( text.isEmpty() )
            text = QString::number(curNodeId);
        QString ncbiLink = QString("<a href=\"http://www.ncbi.nlm.nih.gov/Taxonomy/Browser/wwwtax.cgi?mode=Info&id=%1\">%2</a>")
                .arg(curNodeId)
                .arg(text);
        ui->lName->setText(ncbiLink);
    }
    else
    {
        ui->lName->setText(bNode->getText());
    }

    TreeTaxNode *ttn = dynamic_cast<TreeTaxNode *>(bNode);
    QStringList sl;
    if ( ttn != NULL )
    {
        TreeTaxNode *p = ttn->parent;
        while ( p != NULL )
        {
            sl.insert(0, p->getText());
            p = p->parent;
        }
        sl.append(bNode->getText());
    }
    ui->lPath->setText(sl.join("\n"));

    ui->lReads->setText(reads == 0 ? QString("-") : QString::number(reads));
    ui->lSum->setText(sum == 0 ? QString("-") : QString::number(sum));

    onColorChanged(bNode);
}

//=========================================================================
void CurrentTaxNodeDetails::onCurrentNodeChanged(BaseTaxNode *node)
{
    if ( node == NULL || node->getId() == curNodeId )
        return;
    curNodeId = node->getId();
    refresh();
}

//=========================================================================
void CurrentTaxNodeDetails::onColorChanged(BaseTaxNode *node)
{
    if ( node->getId() == curNodeId )
    {
        QColor col = colors->getColor(curNodeId);
        if(col.isValid())
        {
            QString qss = QString("background-color: %1").arg(col.name());
            ui->bColor->setStyleSheet(qss);
        }
    }
}

//=========================================================================
void CurrentTaxNodeDetails::onBColorClicked()
{
    colors->pickColor(curNodeId);
}
