#include "datagraphicsview.h"
#include "taxdataprovider.h"
#include "taxnodesignalsender.h"
#include "graphview.h"
#include "chartview.h"

#include <QPrinter>
#include <QPrintDialog>
#include <QPrintPreviewDialog>
#include <QFileDialog>

DataGraphicsView::DataGraphicsView(TaxDataProvider *_dataProvider, QWidget *parent):
    QGraphicsView(parent),
    curNode(NULL),
    taxDataProvider(_dataProvider),
    persistant(false)
{
    setAttribute(Qt::WA_DeleteOnClose);
    printAction = popupMenu.addAction("Print...");
    screenshotAction = popupMenu.addAction("Screenshot...");
    connect(printAction, SIGNAL(triggered(bool)), this, SLOT(print()));
    connect(screenshotAction, SIGNAL(triggered(bool)), this, SLOT(makeScreenshot()));

    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(const QPoint&)),this, SLOT(showContextMenu(const QPoint&)));
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

//=========================================================================
void DataGraphicsView::print()
{
    QPrinter printer(QPrinter::HighResolution);
    printer.setPageSize(QPrinter::A4);
    QPrintPreviewDialog preview(&printer);
    connect(&preview, SIGNAL(paintRequested(QPrinter *)), this, SLOT(renderToPrinter(QPrinter *)));
    preview.exec();
}

//=========================================================================
void DataGraphicsView::makeScreenshot()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save image as"), QString(), "*.png");
    if ( fileName.isEmpty() )
        return;
    if ( fileName.indexOf('.') < 0 )
        fileName.append(".png");

    QRectF rect = scene()->itemsBoundingRect();
    QImage image(rect.size().toSize(), QImage::Format_ARGB32);  // Create the image with the exact size of the shrunk scene
    image.fill(Qt::white);
    QPainter painter(&image);
    renderToPainter(&painter);
    image.save(fileName, "PNG");
}

//=========================================================================
void DataGraphicsView::renderToPrinter(QPrinter *printer)
{
    QPainter painter(printer);
    renderToPainter(&painter);
}

//=========================================================================
void DataGraphicsView::renderToPainter(QPainter *painter)
{
    painter->setRenderHint(QPainter::Antialiasing);
    scene()->render(painter, QRectF(), scene()->itemsBoundingRect());
}

//=========================================================================
void DataGraphicsView::showContextMenu(const QPoint &p)
{
    popupMenu.exec(mapToGlobal(p));
}

