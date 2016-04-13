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
    if ( node == NULL )
        return;
    qint32 id = node->getId();
    ui->lId->setText(QString::number(id));

    if ( id > 1 )
    {
        QString text = node->getText();
        if ( text.isEmpty() )
            text = QString::number(id);
        QString ncbiLink = QString("<a href=\"http://www.ncbi.nlm.nih.gov/Taxonomy/Browser/wwwtax.cgi?mode=Info&id=%1\">%2</a>")
                .arg(node->getId())
                .arg(text);
        ui->lName->setText(ncbiLink);
    }
    else
    {
        ui->lName->setText(node->getText());
    }

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
