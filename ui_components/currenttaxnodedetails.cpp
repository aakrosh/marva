#include "currenttaxnodedetails.h"
#include "ui_currenttaxnodedetails.h"
#include "base_tax_node.h"
#include "blast_data.h"

//=========================================================================
CurrentTaxNodeDetails::CurrentTaxNodeDetails(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CurrentTaxNodeDetails)
{
    ui->setupUi(this);
}

//=========================================================================
CurrentTaxNodeDetails::~CurrentTaxNodeDetails()
{
    delete ui;
}

//=========================================================================
void CurrentTaxNodeDetails::onCurrentNodeChanged(BaseTaxNode *node)
{
    ui->lId->setText(QString::number(node->getId()));
    ui->lName->setText(node->getText());
    QStringList sl;
    BaseTaxNode *p = node->parent;

    while ( p != NULL )
    {
        sl.insert(0, p->getText());
        p = p->parent;
    }
    sl.append(node->getText());
    ui->lPath->setText(sl.join("\n"));

    BlastTaxNode *bnode = dynamic_cast<BlastTaxNode *>(node);
    if ( bnode != NULL )
    {
        ui->lReads->setText(QString::number(bnode->reads));
    }
    else
    {
        ui->lReads->setText("-");
    }
}
