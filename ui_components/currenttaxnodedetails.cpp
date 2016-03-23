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
    qint32 id = node->getId();
    ui->lId->setText(QString::number(id));
    ui->lName->setText(node->getText());
    QStringList sl;
    BaseTaxNode *p = node->parent;

    if ( id > 1 )
    {
        QString ncbiLink = QString("<a href=\"http://www.ncbi.nlm.nih.gov/Taxonomy/Browser/wwwtax.cgi?mode=Info&id=%1\">Open in NCBI</a>")
                .arg(node->getId());
        ui->lNcbi->setText(ncbiLink);
    }
    else
    {
        ui->lNcbi->setText("");
    }

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
