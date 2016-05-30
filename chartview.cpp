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
#include <QJsonObject>
#include <QJsonArray>


#define MAX_CHART_TAXES 40
#define MAX_VISIBLE_CHART_TAXES 20

//=========================================================================
ChartView::ChartView(BlastTaxDataProviders *_dataProviders, QWidget *parent)
    : DataGraphicsView(NULL, parent)
{
    setWindowTitle(tr("Gene chart"));

    if ( _dataProviders == NULL )
        _dataProviders = new BlastTaxDataProviders();

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

    setChartRectSize(800, 800);

    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(const QPoint&)),this, SLOT(showContextMenu(const QPoint&)));
    propertiesAction = popupMenu.addAction("Properties");
    connect(propertiesAction, SIGNAL(triggered(bool)), this, SLOT(showPropertiesDialog()));

    if ( _dataProviders->size() > 0 )
    {
        prepareScene();
        showChart();
    }
}

//=========================================================================
ChartView::~ChartView()
{
    scene()->clear();
}

//=========================================================================
void ChartView::setChartRectSize(int w, int h)
{
    chartRect = QRectF(0, 0, w, h);
    scene()->setSceneRect(-2*MARGIN, config.showTitle ? -MARGIN : 0, chartRect.width()+3*MARGIN, chartRect.height()+MARGIN+(config.showTitle ? MARGIN : 0));
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
void ChartView::prepareScene()
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
    quint32 swidth = (int)this->sceneRect().height()*0.8;
    quint32 columnWidth = qMin(config.bubbleSize, swidth/dp->providers->count());
    grid.clear();
    for ( int i = 0; i < dp->providers->size(); i++ )
    {
        qreal x1 = i*columnWidth;
        grid.append(scene()->addRect(chartRect.x()+x1, chartRect.y(), columnWidth, chartRect.height(), pen, brush));
        for ( int j = 0; j < dp->data.count(); j++)
        {
                if ( !dp->data.at(j).checked )
                    continue;
                const BlastTaxNodes &btns = dp->data.at(j).tax_nodes;
                BlastTaxNode *node = btns.at(i);
                if ( node != NULL )
                {
                    if ( node == NULL )
                        continue;
                    quint32 reads = node->reads;
                    if ( reads == 0 )
                        continue;
                    CreateGraphNode(node);
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
            item->setVisible(dp->data.at(j).checked);
            verticalLegend.append(item);
            break;
        }
    }
}

//=========================================================================
void ChartView::showChart()
{
    if ( dataProvider()->data.size() == 0 )
        return;
    quint32 sheight = (int)this->sceneRect().height()*0.8;
    quint32 swidth = (int)this->sceneRect().height()*0.8;
    quint32 tsize = dataProvider()->visibleTaxNumber();
    quint32 maxBubbleSize = tsize == 0 ? 0 : qMin(config.bubbleSize, sheight/tsize);
    quint32 columnWidth = qMin(config.bubbleSize, swidth/dataProvider()->providers->count());
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
            gnode->setMaxNodeSize(config.bubbleSize);
            gnode->setPos(chartRect.x()+x1+columnWidth/2, chartRect.y()+(rnum)*maxBubbleSize+maxBubbleSize/2);
        }
        verticalLegend[j]->setPos(chartRect.x()-verticalLegend[j]->boundingRect().width(), rnum*maxBubbleSize);
        ++rnum;
    }
    quint32 rectHeight = rnum*maxBubbleSize + maxBubbleSize/2;
    chartRectGI->setRect(chartRect.x(), chartRect.y(), dataProvider()->providers->count()*columnWidth, rectHeight);
    if ( header->isVisible() )
    {
        header->setPos(0, 10-MARGIN);
        header->setTextWidth(dataProvider()->providers->count()*columnWidth);
    }
    for ( int i = 0; i < chartDataProvider->providers->size(); i++ )
    {
        qreal x1 = i*columnWidth;
        grid.at(i)->setRect(chartRect.x()+x1, chartRect.y(), columnWidth, rectHeight);
        horizontalLegend.at(i)->setPos(chartRect.x()+x1+columnWidth/2, chartRect.y()+rectHeight+5);
    }
}


//=========================================================================
void ChartView::setVerticalLegendSelected(qint32 index, bool selected)
{
    if ( index >= 0 )
    {
        verticalLegend[index]->setDefaultTextColor(selected ? Qt::red : Qt::black);
        verticalLegend[index]->update();
    }
}

//=========================================================================
void ChartView::setVerticalLegentColor(BaseTaxNode *node, bool selected)
{
    qint32 index = node == NULL ? -1 : dataProvider()->indexOf(node->getId());
    setVerticalLegendSelected(index, selected);
}

//=========================================================================
void ChartView::compareNodesAndUpdate(ChartGraphNode *chartGraphNode, BaseTaxNode *refNode)
{
    if ( refNode != NULL )
    {
        if ( chartGraphNode->getTaxNode()->getId() == refNode->getId() )
            chartGraphNode->update();
    }
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
            compareNodesAndUpdate(n, oldCurNode);
            compareNodesAndUpdate(n, curNode);
        }
    }
    if ( curNode != NULL && oldCurNode != NULL && curNode->getId() == oldCurNode->getId() )
        return;
    setVerticalLegentColor(curNode, true);
    setVerticalLegentColor(oldCurNode, false);
}

//=========================================================================
void ChartView::CreateGraphNode(BlastTaxNode *node)
{
    ChartGraphNode *gnode = new ChartGraphNode(this, node);
    node->gnode = gnode;
    gnode->updateToolTip();
    scene()->addItem(gnode);
}

//=========================================================================
void ChartView::toJson(QJsonObject &json) const
{
    json["Type"] = QString("ChartView");
    json["Header"] = header->toPlainText();
    json["MaxNodeSize"] = qint32(config.bubbleSize);
    json["ShowTitle"] = config.showTitle;
    QJsonObject jDp;
    dataProvider()->toJson(jDp);
    json["Dp"] = jDp;
}

//=========================================================================
void ChartView::fromJson(QJsonObject &json)
{
    QJsonObject jDp = json["Dp"].toObject();
    dataProvider()->fromJson(jDp);
    prepareScene();
    header->setPlainText(json["Header"].toString());
    config.bubbleSize = json["MaxNodeSize"].toInt();
    config.maxBubbleSize = config.bubbleSize*2;
    config.showTitle = json["ShowTitle"].toBool();
    showChart();
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
            CreateGraphNode(node);
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
    QGraphicsItem *item = scene()->itemAt(mapToScene(pos), QTransform());
    ChartGraphNode *cgn = dynamic_cast<ChartGraphNode *>(item);
    QMenu *newMenu = new QMenu(this);
    if ( cgn != NULL )
    {
        QAction *hideCurTax = newMenu->addAction("Hide");
        connect(hideCurTax, SIGNAL(triggered(bool)), this, SLOT(hideCurrentTax()));
    }
    else
    {
        QGraphicsTextItem *gti = dynamic_cast<QGraphicsTextItem *>(item);
        for ( int i = 0; i < verticalLegend.count(); i++ )
        {
            if ( gti == verticalLegend[i] )
            {
                TaxNodeSignalSender *tnss = getTaxNodeSignalSender(dataProvider()->taxNode(i));
                tnss->makeCurrent();
                QAction *hideCurTax = newMenu->addAction("Hide");
                connect(hideCurTax, SIGNAL(triggered(bool)), this, SLOT(hideCurrentTax()));
                break;
            }
        }
    }
    newMenu->addActions(popupMenu.actions());
    newMenu->exec(mapToGlobal(pos));
}

//=========================================================================
void ChartView::changeMaxBubbleSize(int)
{
    showChart();
}

//=========================================================================
void ChartView::toggleTitleVisibility(bool visible)
{
    QSize s = size();
    header->setVisible(visible);
    setChartRectSize(s.width()*0.8, s.height()*0.8);
    showChart();
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
    header->setVisible(config.showTitle);
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
    BubbleChartProperties *propertiesDialog = new BubbleChartProperties(this, &config);
    connect(propertiesDialog, SIGNAL(maxBubbleSizeChanged(int)), this, SLOT(changeMaxBubbleSize(int)));
    connect(propertiesDialog, SIGNAL(showTitleToggled(bool)), this, SLOT(toggleTitleVisibility(bool)));
    propertiesDialog->exec();
}

//=========================================================================
void ChartView::hideCurrentTax()
{
    TaxNodeSignalSender *tnss = getTaxNodeSignalSender(curNode);
    tnss->VisibilityChanged(false);
    //dataProvider()->setCheckedState(dataProvider()->indexOf(curNode->getId()), Qt::Unchecked);
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
//=========================================================================
ChartDataProvider::ChartDataProvider(BlastTaxDataProviders *_providers, QObject *parent)
    : TaxDataProvider(parent),
      providers(_providers)
{
    name = "Bubble chart";
    addParentToAllDataProviders();
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
void ChartDataProvider::addParentToAllDataProviders()
{
    for ( int i = 0; i < providers->size(); i++ )
        providers->at(i)->addParent();
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

        cdp = this;
        qSort(data.begin(), data.end(), readsLessThan);
        while ( data.size() > MAX_CHART_TAXES )
            data.removeFirst();
        quint32 dsize = data.size();
        for ( quint32 i = MAX_VISIBLE_CHART_TAXES; i < dsize; i++ )
            data[dsize-i-1].checked = false;
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
    bool checked = val == Qt::Checked;
    if ( checked == data[index].checked )
        return;
    data[index].checked = checked;
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
quint32 ChartDataProvider::visibleTaxNumber()
{
    quint32 vbnum = 0;
    for ( int i = 0 ; i  < data.size(); i++ )
        if ( data.at(i).checked )
            vbnum++;
    return vbnum;
}

//=========================================================================
void ChartDataProvider::toJson(QJsonObject &json) const
{
    json["maxreads"] = (qint64)maxreads;
    QJsonArray providersArray;
    for ( int i = 0 ; i < providers->size(); i++ )
        providersArray.append(providers->at(i)->name);
    json["BlastDataProviders"] = providersArray;
}

//=========================================================================
void ChartDataProvider::fromJson(QJsonObject &json)
{
    maxreads = json["maxreads"].toInt();
    QJsonArray providersArray = json["BlastDataProviders"].toArray();
    for ( int i = 0; i < providersArray.count(); i++ )
    {
        QString pName = providersArray[i].toString();
        BlastTaxDataProvider *p = blastTaxDataProviders.providerByName(pName);
        if ( p == NULL )
            continue;
        providers->append(p);
        p->addParent();
    }
    updateCache(false);
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
