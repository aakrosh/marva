#include "datagraphicsview.h"
#include "taxdataprovider.h"
#include "taxnodesignalsender.h"
#include "graphview.h"
#include "chartview.h"

DataGraphicsView::DataGraphicsView(TaxDataProvider *_dataProvider, QWidget *parent):
    QGraphicsView(parent),
    curNode(NULL),
    taxDataProvider(_dataProvider),
    persistant(false)
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

//=========================================================================
DataGraphicsView *DataGraphicsView::createViewByType(QWidget *parent, QString &type)
{
    if ( type == "BlastGraphView" )
        return new BlastGraphView(NULL, parent, NULL);
    if ( type == "ChartView" )
        return new ChartView(NULL, parent);
    return NULL;
}

