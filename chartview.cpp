#include "chartview.h"
#include "graph_node.h"
#include "taxnodesignalsender.h"
#include "ui_components/bubblechartproperties.h"
#include <math.h>

#include <QKeyEvent>
#include <QDebug>
#include <QGraphicsTextItem>
#include <QTextDocument>
#include <QGraphicsItem>
#include <QGraphicsSceneMouseEvent>
#include <QMenu>


#define MAX_NODE_SIZE 60
#define MAX_CHART_TAXES 20
//=========================================================================
void ChartView::setChartRectSize(int w, int h)
{
    chartRect = QRectF(0, 0, w, h);
    scene()->setSceneRect(-2*MARGIN, -MARGIN, chartRect.width()+3*MARGIN, chartRect.height()+2*MARGIN);
}

//=========================================================================
ChartView::ChartView(BlastTaxDataProviders *_dataProviders, QWidget *parent)
    : DataGraphicsView(NULL, parent),
      maxNodeSize(MAX_NODE_SIZE)
{
    setWindowTitle(tr("Gene chart"));

    taxDataProvider = new ChartDataProvider(_dataProviders, this);

    connect((ChartDataProvider *)taxDataProvider, SIGNAL(taxVisibilityChanged(quint32)), this, SLOT(onTaxVisibilityChanged(quint32)));
    connect((ChartDataProvider *)taxDataProvider, SIGNAL(cacheUpdated()), this, SLOT(onDataChanged()));

    QGraphicsScene *s = new QGraphicsScene(this);
    s->setItemIndexMethod(QGraphicsScene::NoIndex);
    setCacheMode(CacheBackground);
    setViewportUpdateMode(BoundingRectViewportUpdate);
    setRenderHint(QPainter::Antialiasing);
    setTransformationAnchor(AnchorUnderMouse);
    setMinimumSize(400, 200);
    setAlignment(Qt::AlignCenter);
    setScene(s);

    PrepareScene();
    setChartRectSize(800, 800);
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(const QPoint&)),
        this, SLOT(showContextMenu(const QPoint&)));

    propertiesAction = popupMenu.addAction("Properties");

    connect(propertiesAction, SIGNAL(triggered(bool)), this, SLOT(showPropertiesDialog()));

    showChart();
}

//=========================================================================
ChartView::~ChartView()
{
    scene()->clear();
}

//=========================================================================
void ChartView::resizeEvent(QResizeEvent *e)
{
    QSize s = e->size();
    setChartRectSize(s.width()*0.8, s.height()*0.8);
    if ( dataProvider() != NULL )
        showChart();
}

//=========================================================================
void ChartView::onCurrentNodeChanged(BaseTaxNode *node)
{
    if ( curNode == node )
        return;
    BaseTaxNode *oldCurNode = curNode;
    curNode = node;
    foreach(QGraphicsItem *item, scene()->items())
    {
        if ( item->type() == GraphNode::Type )
        {
            ChartGraphNode *n = (ChartGraphNode *)item;
            if ( oldCurNode != NULL )
            {
                if ( n->getTaxNode()->getId() == oldCurNode->getId() )
                    n->update();
            }
            if ( curNode != NULL )
            {
                if ( n->getTaxNode()->getId() == curNode->getId() )
                    n->update();
            }
        }
    }
    if ( curNode != NULL && oldCurNode != NULL && curNode->getId() == oldCurNode->getId() )
        return;
    qint32 index = curNode == NULL ? -1 : dataProvider()->indexOf(curNode->getId());
    qint32 old_index = oldCurNode == NULL ? -1 : dataProvider()->indexOf(oldCurNode->getId());
    if ( index >= 0 )
    {
        verticalLegend[index]->setDefaultTextColor(Qt::red);
        verticalLegend[index]->update();
    }
    if ( old_index >= 0 )
    {
        verticalLegend[old_index]->setDefaultTextColor(Qt::black);
        verticalLegend[old_index]->update();
    }
}

//=========================================================================
void ChartView::onTaxVisibilityChanged(quint32 index)
{
    ChartDataProvider *chartDataProvider = dataProvider();
    const BlastTaxNodes &btns = chartDataProvider->data.at(index).tax_nodes;
    for ( int i = 0; i < btns.size(); i++ )
    {
        BlastTaxNode *node = btns.at(i);
        if ( node == NULL )
            continue;
        if ( chartDataProvider->data.at(index).checked )
        {
            ChartGraphNode *gnode = new ChartGraphNode(this, node);
            node->gnode = gnode;
            gnode->updateToolTip();
            scene()->addItem(gnode);
        }
        else
        {
            ChartGraphNode *gnode = (ChartGraphNode *) node->getGnode();//getGNode(node);
            if ( gnode != NULL )
                delete gnode;
        }
    }
    verticalLegend[index]->setVisible(chartDataProvider->data.at(index).checked);
    showChart();
}

//=========================================================================
void ChartView::onDataChanged()
{
    for ( int j = 0; j < verticalLegend.size(); j++)
    {
        if ( !verticalLegend[j]->toPlainText().isEmpty() )
            continue;
        const BlastTaxNodes &btns = dataProvider()->data.at(j).tax_nodes;
        for ( int i = 0 ; i < btns.size(); i++ )
        {
            if ( btns[i] == NULL )
                continue;
            QString txt = btns[i]->getText();
            QString stxt = txt;
            if ( stxt.length() > 30 )
            {
                stxt.truncate(27);
                stxt.append("...");
            }
            verticalLegend[j]->setPlainText(stxt);
            verticalLegend[j]->setToolTip(txt);
            QPointF pos = verticalLegend[j]->pos();
            pos.setX(pos.x() - verticalLegend[j]->boundingRect().width());
            verticalLegend[j]->setPos(pos);
            verticalLegend[j]->update();
            break;
        }
    }
}

//=========================================================================
void ChartView::showContextMenu(const QPoint &pos)
{
    QPoint globalPos = mapToGlobal(pos);
    popupMenu.exec(globalPos);
}

//=========================================================================
void ChartView::changeMaxBubbleSize(int value)
{
    maxNodeSize = value;
    showChart();
}

//=========================================================================
void ChartView::showChart()
{
    if ( dataProvider()->data.size() == 0 )
        return;
    quint32 sheight = (int)this->sceneRect().height()*0.8;
    quint32 swidth = (int)this->sceneRect().height()*0.8;
    quint32 tsize = dataProvider()->data.size();
    quint32 maxBubbleSize = qMin(maxNodeSize, sheight/tsize);
    quint32 columnWidth = qMin(maxNodeSize, swidth/dataProvider()->providers->count());
    ChartDataProvider *chartDataProvider = dataProvider();
    int rnum = 0;
    for ( int j = 0; j < chartDataProvider->data.count(); j++)
    {
        const BlastTaxNodes &btns = chartDataProvider->data.at(j).tax_nodes;
        if ( !chartDataProvider->data.at(j).checked )
            continue;
        for ( int i = 0; i < btns.size(); i++ )
        {
            qreal x1 = i*columnWidth;
            BlastTaxNode *node = btns.at(i);
            if ( node == NULL )
                continue;
            ChartGraphNode *gnode = (ChartGraphNode*)node->getGnode();
            Q_ASSERT_X(gnode != NULL, "showChart", "GraphNode must be created here");
            gnode->setMaxNodeSize(maxNodeSize);
            gnode->setPos(chartRect.x()+x1+columnWidth/2, chartRect.y()+(rnum)*maxBubbleSize+maxBubbleSize/2);
        }
        verticalLegend[j]->setPos(chartRect.x()-verticalLegend[j]->boundingRect().width(), rnum*maxBubbleSize);
        ++rnum;
    }
    quint32 rectHeight = rnum*maxBubbleSize + maxBubbleSize/2;
    chartRectGI->setRect(chartRect.x(), chartRect.y(), dataProvider()->providers->count()*columnWidth, rectHeight);
    header->setPos(0, 10-MARGIN);
    header->setTextWidth(dataProvider()->providers->count()*columnWidth);
    for ( int i = 0; i < chartDataProvider->providers->size(); i++ )
    {
        qreal x1 = i*columnWidth;
        grid.at(i)->setRect(chartRect.x()+x1, chartRect.y(), columnWidth, rectHeight);
        horizontalLegend.at(i)->setPos(chartRect.x()+x1+columnWidth/2, chartRect.y()+rectHeight+5);
    }
}

//=========================================================================
void ChartView::setHeader(QString fileName)
{
    int idx = fileName.lastIndexOf('/');
    fileName = fileName.mid(idx+1);
    idx = fileName.indexOf('.');
    fileName = fileName.left(idx);
    header = scene()->addText(fileName);
    header->document()->setDefaultTextOption(QTextOption(Qt::AlignCenter | Qt::AlignVCenter));
    header->setFlags(QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsFocusable | QGraphicsItem::ItemIsMovable);
    header->setTextInteractionFlags(Qt::TextEditorInteraction);
    QFont font;
    font.setPixelSize(25);
    font.setBold(true);
    header->setFont(font);
}

//=========================================================================
void ChartView::PrepareScene()
{
    QPen peng(Qt::black);
    peng.setWidth(1.5);
    QBrush brushg(Qt::transparent);
    chartRectGI = scene()->addRect(chartRect, peng, brushg);
    setHeader(/*chartData.header*/ "Chart header");
    ChartDataProvider *dp = dataProvider();
    if ( dp->data.size() == 0 )
        return;
    QPen pen(Qt::lightGray);
    pen.setWidth(0.2);
    QBrush brush(Qt::transparent);
    qint32 swidth = (int)this->sceneRect().height()*0.8;
    quint32 columnWidth = qMin(MAX_NODE_SIZE, swidth/dp->providers->count());
    grid.clear();
    for ( int i = 0; i < dp->providers->size(); i++ )
    {
        qreal x1 = i*columnWidth;
        grid.append(scene()->addRect(chartRect.x()+x1, chartRect.y(), columnWidth, chartRect.height(), pen, brush));
        for ( int j = 0; j < dp->data.count(); j++)
        {
                const BlastTaxNodes &btns = dp->data.at(j).tax_nodes;
                BlastTaxNode *node = btns.at(i);
                if ( node != NULL )
                {
                    if ( node == NULL )
                        continue;
                    quint32 reads = node->reads;
                    if ( reads == 0 )
                        continue;
                    ChartGraphNode *gnode = new ChartGraphNode(this, node);
                    node->gnode = gnode;
                    gnode->updateToolTip();
                    scene()->addItem(gnode);
                }
        }
        // Create horizontal axe labels
        BlastTaxDataProvider *p = dp->providers->at(i);
        QString stxt = p->name;
        if ( stxt.length() > 30 )
        {
            stxt.truncate(27);
            stxt.append("...");
        }
        QGraphicsTextItem *item = scene()->addText(stxt);
        item->setToolTip(p->name);
        item->setRotation(45);
        horizontalLegend.append(item);
    }
    for ( int j = 0; j < dp->data.count(); j++)
    {
        const BlastTaxNodes &btns = dp->data.at(j).tax_nodes;
        for ( int i = 0 ; i < btns.size(); i++ )
        {
            if ( btns[i] == NULL )
                continue;
            QString txt = btns[i]->getText();
            QString stxt = txt;
            if ( stxt.length() > 30 )
            {
                stxt.truncate(27);
                stxt.append("...");
            }
            QGraphicsTextItem *item = scene()->addText(stxt);
            item->setToolTip(txt);
            item->installEventFilter(this);
            verticalLegend.append(item);
            break;
        }
    }
}

//=========================================================================
ChartGraphNode *ChartView::getGNode(BlastTaxNode *node)
{
    foreach(QGraphicsItem *item, scene()->items())
    {
        if ( item->type() == GraphNode::Type )
        {
            ChartGraphNode *n = (ChartGraphNode *)item;
            if ( n->getTaxNode() == node )
                return n;
        }
    }
    return NULL;
}

//=========================================================================
void ChartView::goUp()
{
    BaseTaxNode *cNode = currentNode();
    if ( cNode == NULL )
        return;
    qint32 index = dataProvider()->indexOf(cNode->getId());
    if ( index <= 0 )
        return;
    TaxNodeSignalSender *tnss = getTaxNodeSignalSender(dataProvider()->taxNode(--index));
    tnss->makeCurrent();
}

//=========================================================================
void ChartView::goDown()
{
    BaseTaxNode *cNode = currentNode();
    if ( cNode == NULL )
        return;
    quint32 index = dataProvider()->indexOf(cNode->getId());
    if ( ++index == dataProvider()->count() )
        return;
    TaxNodeSignalSender *tnss = getTaxNodeSignalSender(dataProvider()->taxNode(index));
    tnss->makeCurrent();
}

//=========================================================================
void ChartView::showPropertiesDialog()
{
    BubbleChartProperties *propertiesDialog = new BubbleChartProperties(this);
    connect(propertiesDialog, SIGNAL(maxBubbleSizeChanged(int)), this, SLOT(changeMaxBubbleSize(int)));
    propertiesDialog->setValues(maxNodeSize, maxNodeSize*2);
    propertiesDialog->exec();
}

//=========================================================================
void ChartView::keyPressEvent(QKeyEvent *event)
{
    switch( event->key() )
    {
        case Qt::Key_Up:
            goUp();
            return;
        case Qt::Key_Down:
            goDown();
            return;
    }
    QGraphicsView::keyPressEvent(event);
}

//=========================================================================
bool ChartView::eventFilter(QObject *object, QEvent *event)
{
    if ( event->type() == QEvent::GraphicsSceneMousePress )
    if ( object->metaObject() == &QGraphicsTextItem::staticMetaObject )
    {
        QGraphicsSceneMouseEvent *mevent = (QGraphicsSceneMouseEvent *)event;
        if ( mevent->scenePos().y() <= verticalLegend.last()->y()+10 )
        {
            QGraphicsTextItem *item = (QGraphicsTextItem*)object;
            qint32 index = verticalLegend.indexOf(item);
            if ( index >= 0 )
            {
                TaxNodeSignalSender *tnss = getTaxNodeSignalSender(dataProvider()->taxNode(index));
                tnss->makeCurrent();
            }
        }
    }
    return DataGraphicsView::eventFilter(object, event);
}

//=========================================================================
//************************************************************************
//=========================================================================
ChartDataProvider::ChartDataProvider(BlastTaxDataProviders *_providers, QObject *parent)
    : TaxDataProvider(parent),
      providers(_providers)
{
    for ( int i = 0; i < providers->size(); i++ )
        providers->at(i)->addParent();
    updateCache(false);
}

//=========================================================================
ChartDataProvider::~ChartDataProvider()
{
    foreach(BlastTaxDataProvider *p, *providers)
        p->removeParent();
    for ( int i = 0; i < data.count(); i++ )
    {
        BlastTaxNodes &nodes = data[i].tax_nodes;
        for ( int j = 0; j < nodes.count(); j++ )
            delete nodes[j];
    }
}

//=========================================================================
quint32 ChartDataProvider::count()
{
    return data.count();
}

//=========================================================================
QString ChartDataProvider::text(quint32 index)
{
    foreach (BlastTaxNode *tn, data[index].tax_nodes)
    {
        if ( tn != NULL )
            return tn->tNode->getText();
    }
    return QString();
}

//=========================================================================
qint32 ChartDataProvider::id(quint32 index)
{
    foreach (BlastTaxNode *tn, data[index].tax_nodes)
    {
        if ( tn != NULL )
            return tn->getId();
    }
    return 0;
}

//=========================================================================
quint32 ChartDataProvider::reads(quint32 index)
{
    quint32 res = 0;
    foreach (BlastTaxNode *tn, data[index].tax_nodes)
    {
        if ( tn != NULL )
            res += tn->reads;
    }
    return res;
}

//=========================================================================
quint32 ChartDataProvider::readsById(quint32 id)
{
    quint32 index = indexOf(id);
    return reads(index);
}

//=========================================================================
static ChartDataProvider *cdp;
bool readsLessThan(const IdBlastTaxNodesPair &x1, const IdBlastTaxNodesPair &x2)
{
    return x1.reads() < x2.reads();
}

//=========================================================================
void ChartDataProvider::updateCache(bool ids_only)
{
    if ( !ids_only )
    {
        maxreads = 0;
        for ( int i = 0; i < providers->size(); i++ )
        {
            BlastTaxDataProvider *p = providers->at(i);
            for ( quint32 j = 0; j < p->count(); j++ )
            {
                if ( p->checkState(j) == Qt::Checked )
                {
                    quint32 reads = p->reads(j);
                    if ( reads == 0 )
                        continue;
                    quint32 taxid = p->id(j);
                    qint32 ind = indexOf(taxid);
                    if ( ind == -1 )
                        data.append(IdBlastTaxNodesPair(taxid));
                    else
                        data[ind].tax_nodes.clear();
                    if ( reads > maxreads )
                        maxreads = reads;
                }
            }
        }

        for ( int i = 0; i < providers->size(); i++ )
        {
            BlastTaxDataProvider *p = providers->at(i);
            for ( int j = 0; j < data.count(); j++)
            {
                int id = data[j].id;
                qint32 index = p->indexOf(id);
                if ( index >= 0 )
                    data[j].tax_nodes.append(((BlastTaxNode *)p->taxNode(index))->clone());
                else
                    data[j].tax_nodes.append(NULL);
            }
        }

        if ( data.size() > MAX_CHART_TAXES )
        {
            cdp = this;
            qSort(data.begin(), data.end(), readsLessThan);
            while ( data.size() > MAX_CHART_TAXES )
                data.removeFirst();
        }
    }
    emit cacheUpdated();
}

//=========================================================================
QColor ChartDataProvider::color(int index)
{
    return taxColorSrc.getColor(data[index].id);
}

//=========================================================================
void ChartDataProvider::sort(int /*column*/, Qt::SortOrder /*order*/)
{
    // TODO:
}

//=========================================================================
quint32 ChartDataProvider::getMaxReads()
{
    return maxreads;
}

//=========================================================================
bool ChartDataProvider::contains(quint32 id)
{
    for( int i = 0; i < data.size(); i++ )
        if ( data[i].id == id )
            return true;
    return false;
}

//=========================================================================
quint32 ChartDataProvider::indexOf(qint32 id)
{
    for( int i = 0; i < data.size(); i++ )
        if ( data[i].id == (quint32)id )
            return i;
    return -1;
}

//=========================================================================
QVariant ChartDataProvider::checkState(int index)
{
    return data[index].checked ? Qt::Checked : Qt::Unchecked;
}

//=========================================================================
void ChartDataProvider::setCheckedState(int index, QVariant val)
{
    data[index].checked = val == Qt::Checked;
    maxreads = 0;
    for ( qint32 i = 0; i < data.count(); i++ )
    {
        IdBlastTaxNodesPair &di = data[i];
        if ( !di.checked )
            continue;
        for ( int j = 0; j < di.tax_nodes.size(); j++ )
        {
            BlastTaxNode *node = di.tax_nodes[j];
            if ( node == NULL )
                continue;
            quint32 reads = node->reads;
            if ( reads > maxreads )
                 maxreads = reads;
        }
    }
    emit taxVisibilityChanged(index);
}

//=========================================================================
BaseTaxNode *ChartDataProvider::taxNode(quint32 index)
{
    BlastTaxNodes btns = data[index].tax_nodes;
    for ( int i = 0; i < btns.count(); i++ )
    {
        BlastTaxNode *btn = btns[i];
        if ( btn != NULL )
            return btn;
    }
    return NULL;
}

//=========================================================================
//*************************************************************************
//=========================================================================
quint32 IdBlastTaxNodesPair::reads() const
{
    quint32 res = 0;
    foreach (BlastTaxNode *n, tax_nodes )
    {
        if ( n != NULL )
            res += n->reads;
    }
    return res;
}
