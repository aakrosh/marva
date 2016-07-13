#include "bubblechartview.h"
#include "graph_node.h"
#include "taxnodesignalsender.h"
#include "ui_components/bubblechartproperties.h"
#include "colors.h"
#include "config.h"
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
#include <QMessageBox>

//=========================================================================
BubbleChartView::BubbleChartView(BlastTaxDataProviders *_dataProviders, QWidget *parent)
    : DataGraphicsView(NULL, parent)
{
    flags |= DGF_BUBBLES;
    config = new BubbleChartParameters();
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

    propertiesAction = popupMenu.addAction("Properties...");
    connect(propertiesAction, SIGNAL(triggered(bool)), this, SLOT(showPropertiesDialog()));

    if ( _dataProviders->size() > 0 )
    {
        prepareScene();
        showChart();
    }
}

//=========================================================================
BubbleChartView::~BubbleChartView()
{
    scene()->clear();
}

//=========================================================================
void BubbleChartView::setChartRectSize(int w, int h)
{
    chartRect = QRectF(0, 0, w, h);
    scene()->setSceneRect(-2*MARGIN, getConfig()->showTitle ? -MARGIN : 0, chartRect.width()+3*MARGIN, chartRect.height()+MARGIN+(getConfig()->showTitle ? MARGIN : 0));
}

//=========================================================================
void BubbleChartView::resizeEvent(QResizeEvent *e)
{
    QSize s = e->size();
    setChartRectSize(s.width()*0.8, s.height()*0.8);
    if ( dataProvider() != NULL )
        showChart();
}

//=========================================================================
void BubbleChartView::prepareScene()
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
    quint32 columnWidth = qMin((quint32)getConfig()->bubbleSize, swidth/dp->providers->count());
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
    header->setPos(0, 10-MARGIN);
    header->setTextWidth(dp->providers->count()*columnWidth);
}

//=========================================================================
void BubbleChartView::showChart(bool forceNodeUpdate)
{
    if ( dataProvider()->data.size() == 0 )
        return;
    quint32 sheight = (int)this->sceneRect().height()*0.8;
    quint32 swidth = (int)this->sceneRect().height()*0.8;
    quint32 tsize = dataProvider()->visibleTaxNumber();
    quint32 bubbleSize = getConfig()->bubbleSize;
    quint32 maxBubbleSize = tsize == 0 ? 0 : qMin(bubbleSize, sheight/tsize);
    quint32 columnWidth = qMin(bubbleSize, swidth/dataProvider()->providers->count());
    ChartDataProvider *chartDataProvider = dataProvider();
    int rnum = 0;
    quint32 column = 0;
    for ( int j = 0; j < chartDataProvider->data.count(); j++)
    {
        const BlastTaxNodes &btns = chartDataProvider->data.at(j).tax_nodes;
        if ( !chartDataProvider->data.at(j).checked )
            continue;
        column = 0;
        for ( int i = 0; i < btns.size(); i++ )
        {
            if ( !chartDataProvider->getBlastTaxDataProviders()->isVisible(i) )
                continue;
            qreal x1 = (column++)*columnWidth;
            BlastTaxNode *node = btns.at(i);
            if ( node == NULL )
                continue;
            ChartGraphNode *gnode = (ChartGraphNode*)node->getGnode();
            Q_ASSERT_X(gnode != NULL, "showChart", "GraphNode must be created here");            
            gnode->setMaxNodeSize(bubbleSize);
            gnode->setPos(chartRect.x()+x1+columnWidth/2, chartRect.y()+(rnum)*maxBubbleSize+maxBubbleSize/2);
            if ( forceNodeUpdate )
                gnode->update();
        }
        verticalLegend[j]->setPos(chartRect.x()-verticalLegend[j]->boundingRect().width(), rnum*maxBubbleSize);
        ++rnum;
    }
    quint32 rectHeight = rnum*maxBubbleSize + maxBubbleSize/2;
    chartRectGI->setRect(chartRect.x(), chartRect.y(), column*columnWidth, rectHeight);
    if ( header->isVisible() && false )
    {
        header->setPos(0, 10-MARGIN);
        header->setTextWidth(column*columnWidth);
    }
    column = 0;
    for ( int i = 0; i < chartDataProvider->providers->size(); i++ )
    {
        if ( !horizontalLegend.at(i)->isVisible() )
            continue;
        qreal x1 = column*columnWidth;
        grid.at(i)->setRect(chartRect.x()+x1, chartRect.y(), columnWidth, rectHeight);
        horizontalLegend.at(i)->setPos(chartRect.x()+x1+columnWidth/2, chartRect.y()+rectHeight+5);
        column++;
    }
}


//=========================================================================
void BubbleChartView::setVerticalLegendSelected(qint32 index, bool selected)
{
    if ( index >= 0 )
    {
        verticalLegend[index]->setDefaultTextColor(selected ? Qt::red : Qt::black);
        verticalLegend[index]->update();
    }
}

//=========================================================================
void BubbleChartView::setVerticalLegentColor(BaseTaxNode *node, bool selected)
{
    qint32 index = node == NULL ? -1 : dataProvider()->indexOf(node->getId());
    setVerticalLegendSelected(index, selected);
}

//=========================================================================
void BubbleChartView::compareNodesAndUpdate(ChartGraphNode *chartGraphNode, BaseTaxNode *refNode)
{
    if ( refNode != NULL )
    {
        if ( chartGraphNode->getTaxNode()->getId() == refNode->getId() )
            chartGraphNode->update();
    }
}

//=========================================================================
void BubbleChartView::onCurrentNodeChanged(BaseTaxNode *node)
{
    BaseTaxNode *curNode = currentNode();
    if ( curNode == node )
        return;
    BaseTaxNode *oldCurNode = curNode;
    taxDataProvider->current_tax_id = node->getId();
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
void BubbleChartView::CreateGraphNode(BlastTaxNode *node)
{
    ChartGraphNode *gnode = new ChartGraphNode(this, node);
    node->gnode = gnode;
    gnode->updateToolTip();
    scene()->addItem(gnode);
}

//=========================================================================
void BubbleChartView::toJson(QJsonObject &json) const
{
    json["Type"] = QString("ChartView");
    json["Header"] = header->toPlainText();
    json["MaxNodeSize"] = qint32(getConfig()->bubbleSize);
    json["ShowTitle"] = getConfig()->showTitle;
    QJsonObject jDp;
    dataProvider()->toJson(jDp);
    json["Dp"] = jDp;
}

//=========================================================================
void BubbleChartView::fromJson(QJsonObject &json)
{
    try
    {
        QJsonObject jDp = json["Dp"].toObject();
        dataProvider()->fromJson(jDp);
        prepareScene();
        header->setPlainText(json["Header"].toString());
        getConfig()->bubbleSize = json["MaxNodeSize"].toInt();
        getConfig()->maxBubbleSize = getConfig()->bubbleSize*2;
        getConfig()->showTitle = json["ShowTitle"].toBool();
        showChart();
    }
    catch (...)
    {
        QMessageBox::warning(this, "Error occured", "Cannot restore Bubble Chart from project configuration");
    }
}

//=========================================================================
void BubbleChartView::onTaxVisibilityChanged(quint32 index)
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
            ChartGraphNode *gnode = (ChartGraphNode *) node->getGnode();
            if ( gnode != NULL )
                delete gnode;
        }
    }
    verticalLegend[index]->setVisible(chartDataProvider->data.at(index).checked);
    showChart(true);
}

//=========================================================================
void BubbleChartView::onDataSourceVisibilityChanged(int index)
{
    ChartDataProvider *dp = dataProvider();
    BlastTaxDataProviders *btdps = dp->getBlastTaxDataProviders();
    bool visible = btdps->isVisible(index);
    for ( int i = 0; i < dp->data.count(); i++ )
    {
        if ( !dp->data.at(i).checked )
            continue;
        BlastTaxNode *node = dp->data.at(i).tax_nodes[index];
        if ( node == NULL )
            continue;
        GraphNode *gnode = node->getGnode();
        if ( visible )
        {
            if ( gnode == NULL )
                CreateGraphNode(node);
        }
        else
        {
            if ( gnode != NULL )
                delete gnode;
        }
    }
    horizontalLegend.at(index)->setVisible(visible);
    grid.at(index)->setVisible(visible);
    showChart(true);
}

//=========================================================================
void BubbleChartView::onDataChanged()
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
void BubbleChartView::showContextMenu(const QPoint &pos)
{
    QGraphicsItem *item = scene()->itemAt(mapToScene(pos), QTransform());
    ChartGraphNode *cgn = dynamic_cast<ChartGraphNode *>(item);
    QMenu *newMenu = new QMenu(this);
    if ( cgn != NULL )
    {
        QAction *hideCurTax = newMenu->addAction("Hide");
        connect(hideCurTax, SIGNAL(triggered(bool)), this, SLOT(hideCurrentTax()));
        QAction *setCurrentTaxColor = newMenu->addAction("Color...");
        connect(setCurrentTaxColor, SIGNAL(triggered(bool)), this, SLOT(setCurrentTaxColor()));
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
void BubbleChartView::changeMaxBubbleSize(int)
{
    showChart();
}

//=========================================================================
void BubbleChartView::onBubbleSizeChanged(quint32, quint32 newS)
{
    getConfig()->bubbleSize = newS;
    showChart();
}

//=========================================================================
void BubbleChartView::toggleTitleVisibility(bool visible)
{
    QSize s = size();
    header->setVisible(visible);
    setChartRectSize(s.width()*0.8, s.height()*0.8);
    showChart();
}

//=========================================================================
void BubbleChartView::setHeader(QString fileName)
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
    header->setVisible(getConfig()->showTitle);
}


//=========================================================================
ChartGraphNode *BubbleChartView::getGNode(BlastTaxNode *node)
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
void BubbleChartView::goUp()
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
void BubbleChartView::goDown()
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
void BubbleChartView::showPropertiesDialog()
{
    BubbleChartProperties *propertiesDialog = new BubbleChartProperties(this, getConfig(), dataProvider()->getBlastTaxDataProviders());
    connect(propertiesDialog, SIGNAL(maxBubbleSizeChanged(int)), this, SLOT(changeMaxBubbleSize(int)));
    connect(propertiesDialog, SIGNAL(showTitleToggled(bool)), this, SLOT(toggleTitleVisibility(bool)));
    connect(propertiesDialog, SIGNAL(dataSourceVisibilityChanged(int)), this, SLOT(onDataSourceVisibilityChanged(int)));
    propertiesDialog->show();
}

//=========================================================================
void BubbleChartView::showDataSourceDialog()
{

}

//=========================================================================
void BubbleChartView::onColorChanged(BaseTaxNode *node)
{
    quint32 index = dataProvider()->indexOf(node->getId());
    foreach(BlastTaxNode *btn, dataProvider()->data.at(index).tax_nodes)
    {
        if ( btn == NULL )
            continue;
        GraphNode *gn = btn->getGnode();
        if ( gn != NULL )
            gn->update();
    }
}

//=========================================================================
void BubbleChartView::hideCurrentTax()
{
    TaxNodeSignalSender *tnss = getTaxNodeSignalSender(currentNode());
    tnss->VisibilityChanged(false);
}

//=========================================================================
void BubbleChartView::setCurrentTaxColor()
{
    colors->pickColor(taxDataProvider->current_tax_id);
}

//=========================================================================
void BubbleChartView::keyPressEvent(QKeyEvent *event)
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
bool BubbleChartView::eventFilter(QObject *object, QEvent *event)
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
        while ( data.size() > configuration->BubbleChart()->maxChartTaxes() )
            data.removeFirst();
        quint32 dsize = data.size();
        for ( quint32 i = configuration->BubbleChart()->defaultVisibleChartTaxes(); i < dsize; i++ )
            data[dsize-i-1].checked = false;
    }
    emit cacheUpdated();
}

//=========================================================================
QColor ChartDataProvider::color(int index)
{
    return colors->getColor(data[index].id);
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
void ChartDataProvider::toJson(QJsonObject &json)
{
    try
    {
        json["maxreads"] = (qint64)maxreads;
        QJsonArray providersArray;
        for ( int i = 0 ; i < providers->size(); i++ )
            providersArray.append(providers->at(i)->name);
        json["BlastDataProviders"] = providersArray;
        QJsonArray tax_array;
        for ( int i = 0; i < data.size(); i++ )
        {
            QJsonArray tax_node;
            tax_node.append((qint64)data[i].id);
            tax_node.append(data[i].checked ? 1: 0);
            tax_array.append(tax_node);
        }
        json["tax_ids"] = tax_array;
    }
    catch (...)
    {
        QMessageBox::warning(NULL, "Error occured", "Cannot restore data needed for chart");
    }
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
        providers->addProvider(p);
        p->addParent();
    }
    QJsonArray tax_array = json["tax_ids"].toArray();
    data.reserve(tax_array.count());
    for ( int i = 0 ; i < tax_array.count(); i++ )
    {
        QJsonArray tax_node = tax_array[i].toArray();
        quint32 tax_id = (quint32)tax_node[0].toInt();
        bool checked = (quint32)tax_node[1].toInt() == 1;
        data.append(IdBlastTaxNodesPair(tax_id, checked));
    }
    for ( int i = 0; i < providers->size(); i++ )
    {
        BlastTaxDataProvider *p = providers->at(i);
        for ( int j = 0; j < data.count(); j++)
        {
            int id = data[j].id;
            qint32 index = p->indexOf(id);
            quint32 reads = p->reads(id);
            if ( reads > maxreads)
                maxreads = reads;
            data[j].tax_nodes.append( index < 0 ? NULL : ((BlastTaxNode *)p->taxNode(index))->clone());
        }
    }
    updateCache(true);
}

//=========================================================================
BaseTaxNode *ChartDataProvider::taxNode(quint32 index)
{
    if ( ((qint32)index) < 0 )
        return NULL;
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
