#include "datagraphicsview.h"
#include "taxdataprovider.h"
#include "taxnodesignalsender.h"

DataGraphicsView::DataGraphicsView(TaxDataProvider *_dataProvider, QWidget *parent):
    QGraphicsView(parent),
    curNode(NULL),
    taxDataProvider(_dataProvider)
{
    setAttribute(Qt::WA_DeleteOnClose);
}

//=========================================================================
void DataGraphicsView::setCurrentNode(BaseTaxNode *node)
{
    if ( curNode == node )
        return;
    TaxNodeSignalSender *tnss = getTaxNodeSignalSender(node);
    tnss->makeCurrent();
}

