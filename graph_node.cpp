
#include "edge.h"
#include "graph_node.h"
#include "graphview.h"
#include "tax_map.h"
#include "taxdataprovider.h"
#include "taxnodesignalsender.h"
#include "bubblechartview.h"
#include "colors.h"
#include "config.h"

#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QStyleOption>
#include <QDebug>
#include <QScrollBar>
#include <QMenu>

//=========================================================================
GraphNode::GraphNode(DataGraphicsView *view, BaseTaxNode *node):
    view(view),
    tax_node(node)
{
    setFlag(ItemSendsGeometryChanges);
    setCacheMode(DeviceCoordinateCache);
    updateToolTip();
}

//=========================================================================
GraphNode::~GraphNode()
{
    view->scene()->removeItem(this);
    tax_node->removeGnode();
}


//=========================================================================
void GraphNode::updateToolTip()
{
    setToolTip(tax_node->getText().append("\nid = %1").arg(tax_node->getId()));
}

//=========================================================================
quint32 GraphNode::color()
{
    return colors->getColor(tax_node->getId());
}


//=========================================================================
QRectF GraphNode::boundingRect() const
{
    return shape().boundingRect();
}

//=========================================================================
TaxTreeGraphNode::TaxTreeGraphNode(TreeGraphView *gView, TreeTaxNode *node):
    GraphNode(gView, node),
    edge(NULL),
    dirty(0)
{
    node->gnode = this;
}

//=========================================================================
void TaxTreeGraphNode::adjustAdges()
{
    if ( edge != NULL )
        edge->adjust();
}

//=========================================================================
void TaxTreeGraphNode::addEdge(Edge *_edge)
{
    edge = _edge;
    edge->adjust();
}

//=========================================================================
TaxTreeGraphNode::~TaxTreeGraphNode()
{
    if  ( edge != NULL )
    {
        view->scene()->removeItem(edge);
        delete edge;
    }
}

//=========================================================================
QPainterPath TaxTreeGraphNode::getTextPath() const
{
    QPainterPath path;
    QFont font;
    QFontMetricsF fm(font);
    static qreal w = fm.width("XXXXXXXXXXXXXXXXXXXX"); // Instead of calculation of width of actual width
                                                       // We will use the precalculated length of 20 big symbols
                                                       // Its not just about the performance. The bounding rectangle
                                                       // of scene does not depend on width of text for visible nodes
    qreal h = fm.height();
    int ncs = configuration->GraphNode()->nodeCircleSize();
    if ( getTaxNode()->children.isEmpty() || getTaxNode()->isCollapsed() )
        path.addRect(ncs+2, -h/2, w+2, h+2);
    else
        path.addRect(-5 - w/2, -ncs-2-h, w+2, h+2);

    return path;
}

//=========================================================================
QPainterPath TaxTreeGraphNode::shape() const
{
    QPainterPath path = getTextPath();
    int ncs = configuration->GraphNode()->nodeCircleSize();
    path.addEllipse(QPointF(0, 0), ncs, ncs);
    return path;
}

//=========================================================================
void TaxTreeGraphNode::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    QColor mainColor = tax_node == view->currentNode() ? Qt::red : Qt::black;
    painter->setBrush(Qt::transparent);
    painter->setPen(QPen(tax_node->getId() >= 0 ? mainColor : Qt::lightGray, 0));
    int ncs = configuration->GraphNode()->nodeCircleSize();
    painter->drawEllipse(QPointF(0, 0), ncs, ncs);

    QFont font;
    QFontMetricsF fm(font);
    painter->setPen(QPen(mainColor, 0.5, Qt::SolidLine));
    if ( !getTaxNode()->children.isEmpty() )
    {
        int hps = configuration->GraphNode()->halfPlusSize();
        painter->drawLine(-hps, 0, hps, 0);
        if ( getTaxNode()->isCollapsed() )
            painter->drawLine(0 , -hps, 0, hps);
    }
    QString text = this->text();
    if ( text.length() > 20 )
        text = text.mid(0, 17).append("...");
    qreal w = fm.width(text);
    qreal h = fm.height();
    if ( getTaxNode()->children.isEmpty() || getTaxNode()->isCollapsed() )
        painter->drawText(QRectF(ncs+2, -h/2, w+2, h+2), text);
    else
        painter->drawText(QRectF(-5 - w/2, -ncs-2-h, w+2, h+2), text);
}

//=========================================================================
void TaxTreeGraphNode::markDirty(qint32 dirty_flags, DirtyGNodesList *dirtyList)
{
    if ( dirtyList == NULL )
        dirtyList = &((TreeGraphView *)view)->dirtyList;
    if ( (dirty & dirty_flags) == dirty_flags )
        return;
    dirty |= dirty_flags;
    dirtyList->Add(this);
}

//=========================================================================
void TaxTreeGraphNode::clearDirtyFlag(qint32 dirty_flag)
{
    dirty &= ~dirty_flag;
    updateToolTip();
}

//=========================================================================
QVariant TaxTreeGraphNode::itemChange(GraphicsItemChange change, const QVariant &value)
{
    switch (change)
    {
      case ItemPositionHasChanged:
          //adjustAdges();
          break;
      default:
          break;
    };

    return QGraphicsItem::itemChange(change, value);
}

//=========================================================================
void TaxTreeGraphNode::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
  TaxNodeSignalSender *tnss = getTaxNodeSignalSender(tax_node);
  tnss->makeCurrent();
  int ncs = configuration->GraphNode()->nodeCircleSize();
  if ( QRectF(-ncs, -ncs, 2*ncs, 2*ncs).contains(event->pos()) )
    getTaxNode()->setCollapsed(!getTaxNode()->isCollapsed(), true);
  update();
  QGraphicsItem::mousePressEvent(event);
}

//=========================================================================
void TaxTreeGraphNode::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
  update();
  QGraphicsItem::mouseReleaseEvent(event);
}

//=========================================================================
void TaxTreeGraphNode::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    ((TreeGraphView *)view)->nodePopupMenu->popup(event->screenPos());
}

//=========================================================================
void TaxTreeGraphNode::addToScene(QGraphicsScene *scene)
{
  if ( !getTaxNode()->isCollapsed() )
  {
      ThreadSafeListLocker<TreeTaxNode *> locker(&getTaxNode()->children);
      for ( TaxNodeIterator it = getTaxNode()->children.begin(); it < getTaxNode()->children.end(); it++ )
      {
        TaxTreeGraphNode *gnode = (TaxTreeGraphNode *)((*it)->getGnode());
        gnode->addToScene(scene);
        scene->addItem(new Edge(this, gnode));
      }
  }
  scene->addItem(this);
}

//=========================================================================
void TaxTreeGraphNode::setAsRoot(QGraphicsScene *scene)
{
  foreach(QGraphicsItem *i, scene->items())
    scene->removeItem(i);
  addToScene(scene);
}

//=========================================================================
void TaxTreeGraphNode::deleteChildrenNodes()
{
    class GNodeDestroyer : public TaxNodeVisitor
    {
        TreeTaxNode *excl;
    public:
        int children_deleted;
        GNodeDestroyer(TreeGraphView *gv, TreeTaxNode *excludeNode): TaxNodeVisitor(LeavesToRoot, false, gv), excl(excludeNode), children_deleted(0){}
        virtual void Action(TreeTaxNode *node)
        {
            if ( excl == node )
                return;
            TaxTreeGraphNode *gnode = node->getTaxTreeGNode();
            if ( gnode != NULL )
            {
                graphView->dirtyList.Delete(gnode);
                delete gnode;
                children_deleted++;
            }
        }
    };
    GNodeDestroyer gNodeDestroyer(((TreeGraphView *)view), getTaxNode());
    gNodeDestroyer.Visit(getTaxNode());
}

//=========================================================================
void TaxTreeGraphNode::onNodeCollapsed(bool collapsed)
{
    TreeGraphView *gview = ((TreeGraphView *)view);
    NodePositionKeeper keeper(gview, this->tax_node);
    if ( collapsed )
    {
        // Make this small hack to visit children nodes. Otherwise current collapsed node will be skipped
        getTaxNode()->setCollapsed(false, false);
        deleteChildrenNodes();
        getTaxNode()->setCollapsed(true, false);
        gview->resetNodesCoordinates();
    }
    else
    {
        gview->createMissedGraphNodes();
    }
    gview->adjust_scene_boundaries();
}

//=========================================================================
QString TaxTreeGraphNode::text()
{
    QString txt = tax_node->getText();
    if ( txt.isEmpty() )
    {
        markDirty(DIRTY_NAME, &((TreeGraphView *)view)->dirtyList);
        return (QString("%0").arg(tax_node->getId()));
    }
    return QString(txt).append("(%0)").arg(tax_node->getId());
}

//=========================================================================
QString TaxTreeGraphNode::get_const_text() const
{
    return QString(tax_node->getText()).append("(%0)").arg(tax_node->getId());
}

//=========================================================================
bool TaxTreeGraphNode::isGreyedOut()
{
    return tax_node->getId() <= 0;
}

//=========================================================================
//*************************************************************************
//=========================================================================
BlastGraphNode::BlastGraphNode(TreeGraphView *graphWidget, BlastTaxNode *node) :
    TaxTreeGraphNode(graphWidget, node)
{

}

//=========================================================================
QPainterPath BlastGraphNode::shape() const
{
    QPainterPath path = TaxTreeGraphNode::getTextPath();
    int s = size();
    int ncs = configuration->GraphNode()->nodeCircleSize();
    if ( s < ncs )
        s = ncs;
    path.addEllipse(QPointF(0, 0), s, s);
    return path;
}

//=========================================================================
void BlastGraphNode::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    int s = size();
    if ( s > 0 )
    {
        QRadialGradient gradient(QPointF(0 ,0 ), s);
        QColor col_orig = color();
        QColor col = col_orig;
        QColor col0 = col.darker(50);
        col0.setAlphaF(0.8);
        QColor col1 = col.lighter(150);
        col1.setAlphaF(0.2);
        gradient.setColorAt(0, col0);
        gradient.setColorAt(1, col1);
        QPen pen = QPen(col_orig);
        pen.setWidthF(0.1);
        painter->setPen(col_orig);
        painter->setBrush(gradient);
        painter->drawEllipse(QPointF(0, 0), s, s);
    }
    TaxTreeGraphNode::paint(painter, option, widget);
}

//=========================================================================
int BlastGraphNode::size() const
{
    BlastGraphView *bgv = (BlastGraphView *)view;
    quint32 maxReads = bgv->taxDataProvider->getMaxReads();
    if ( maxReads == 0 )
        return 0;
    quint32 reads = 0;
    BlastTaxNode *btn = getTaxNode();
    if ( btn->children.size() == 0 )
        reads = bgv->taxDataProvider->readsById(btn->getId());
    else if ( btn->isCollapsed() )
        reads = bgv->taxDataProvider->sumById(btn->getId());

    if ( reads == 0 )
        return 0;

    return reads * bgv->getConfig()->bubbleSize/maxReads;
}

//=========================================================================
void BlastGraphNode::updateToolTip()
{
    quint32 reads = ((TreeGraphView *)view)->taxDataProvider->readsById(tax_node->getId());
    setToolTip(tax_node->getText().append("\nid = %0\nreads=%1").arg(tax_node->getId()).arg(reads));
}

//=========================================================================
//*************************************************************************
//=========================================================================
ChartGraphNode::ChartGraphNode(DataGraphicsView *view, BaseTaxNode *node):
    GraphNode(view, node)
{
    maxNodeRadius = configuration->BubbleChart()->defaultMaxBubbleSize();
}

//=========================================================================
QPainterPath ChartGraphNode::shape() const
{
    QPainterPath path;
    int s = qMax(size(), 10);
    path.addEllipse(QPointF(0, 0), s, s);
    return path;
}

//=========================================================================
void ChartGraphNode::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    int s = size();
    if ( s > 0 )
    {
        QRadialGradient gradient(QPointF(0 ,0 ), s);
        QColor col_orig = color();
        QColor col = col_orig;
        QColor col0 = col.darker(50);
        col0.setAlphaF(0.8);
        QColor col1 = col.lighter(150);
        col1.setAlphaF(0.2);
        gradient.setColorAt(0, col0);
        gradient.setColorAt(1, col1);
        QPen pen = QPen(col_orig);
        pen.setWidthF(0.1);
        painter->setPen(col_orig);
        painter->setBrush(gradient);
        painter->drawEllipse(QPointF(0, 0), s, s);
    }
    BaseTaxNode *cNode = view->currentNode();
    if ( cNode == NULL )
        return;
    if ( tax_node->getId() == cNode->getId() )
    {
        painter->setPen(Qt::red);
        painter->drawEllipse(QPointF(0, 0), s, s);
    }
}

//=========================================================================
void ChartGraphNode::setMaxNodeSize(quint32 size)
{
    if ( maxNodeRadius*2 != size )
    {
        maxNodeRadius = size/2;
        update();
    }
}

//=========================================================================
int ChartGraphNode::size() const
{
    BubbleChartView * ch_view = (BubbleChartView *)view;
    return qMax(1., ((qreal)reads())/ch_view->dataProvider()->getMaxReads()*maxNodeRadius);
}

//=========================================================================
quint32 ChartGraphNode::reads() const
{
    return ((BlastTaxNode *)tax_node)->reads;
}

//=========================================================================
void ChartGraphNode::updateToolTip()
{
    setToolTip(tax_node->getText().append("\nid = %0\nreads=%1").arg(tax_node->getId()).arg(reads()));
}

//=========================================================================
void ChartGraphNode::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    TaxNodeSignalSender *tnss = getTaxNodeSignalSender(tax_node);
    tnss->makeCurrent();
    update();
    QGraphicsItem::mousePressEvent(event);
}
