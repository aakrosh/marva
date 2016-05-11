
#include "edge.h"
#include "graph_node.h"
#include "graphview.h"
#include "tax_map.h"
#include "taxdataprovider.h"
#include "taxnodesignalsender.h"
#include "chartview.h"
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QStyleOption>
#include <QDebug>
#include <QScrollBar>
#include <QMenu>

#define NODE_CIRCLE_SIZE  8
#define HALF_PLUS_SIZE 3
#define MAX_NODE_RADIUS  30


//=========================================================================
GraphNode::GraphNode(DataGraphicsView *view, BaseTaxNode *node):
      tax_node(node),
      edge(NULL),
      view(view)
{
    setFlag(ItemSendsGeometryChanges);
    setCacheMode(DeviceCoordinateCache);
    updateToolTip();
}

//=========================================================================
GraphNode::~GraphNode()
{
    if  ( edge != NULL )
    {
        view->scene()->removeItem(edge);
        delete edge;
    }
    view->scene()->removeItem(this);
    tax_node->removeGnode();
}


//=========================================================================
void GraphNode::updateToolTip()
{
    setToolTip(tax_node->getText().append("\nid = %1").arg(tax_node->getId()));
}

//=========================================================================
void GraphNode::addEdge(Edge *_edge)
{
    edge = _edge;
    edge->adjust();
}

//=========================================================================
TaxTreeGraphNode::TaxTreeGraphNode(GraphView *gView, BaseTaxNode *node):
    GraphNode(gView, node),
    dirty(0)
{
    node->gnode = this;
}

//=========================================================================
void GraphNode::adjustAdges()
{
    if ( edge != NULL )
        edge->adjust();
}

//=========================================================================
quint32 GraphNode::color()
{
    return taxColorSrc.getColor(tax_node->getId());
}


//=========================================================================
QRectF GraphNode::boundingRect() const
{
    return shape().boundingRect();
}

//=========================================================================
TaxTreeGraphNode::~TaxTreeGraphNode()
{

}

//=========================================================================
QPainterPath TaxTreeGraphNode::shape() const
{
    QPainterPath path;
    path.addEllipse(QPointF(0, 0), NODE_CIRCLE_SIZE, NODE_CIRCLE_SIZE);
    QFont font;
    QFontMetricsF fm(font);
    qreal w = fm.width(get_const_text());
    qreal h = fm.height();
    if ( tax_node->children.isEmpty() || tax_node->isCollapsed() )
        path.addRect(NODE_CIRCLE_SIZE+2, -h/2, w+2, h+2);
    else
        path.addRect(-5 - w/2, -NODE_CIRCLE_SIZE-2-h, w+2, h+2);
    QPainterPath result;
    result.addRect(path.boundingRect());
    return result;
}

//=========================================================================
void TaxTreeGraphNode::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    QColor mainColor = tax_node == view->currentNode() ? Qt::red : Qt::black;
    painter->setBrush(Qt::transparent);
    painter->setPen(QPen(tax_node->getId() >= 0 ? mainColor : Qt::lightGray, 0));
    painter->drawEllipse(QPointF(0, 0), NODE_CIRCLE_SIZE, NODE_CIRCLE_SIZE);

    QFont font;
    QFontMetricsF fm(font);
    painter->setPen(QPen(mainColor, 0.5, Qt::SolidLine));
    if ( !tax_node->children.isEmpty() )
    {
      painter->drawLine(-HALF_PLUS_SIZE, 0, HALF_PLUS_SIZE, 0);
      if ( tax_node->isCollapsed() )
        painter->drawLine(0 , -HALF_PLUS_SIZE, 0, HALF_PLUS_SIZE);
    }
    QString text = this->text();
    if ( text.length() > 20 )
        text = text.mid(0, 17).append("...");
    qreal w = fm.width(text);
    qreal h = fm.height();
    if ( tax_node->children.isEmpty() || tax_node->isCollapsed() )
        painter->drawText(QRectF(NODE_CIRCLE_SIZE+2, -h/2, w+2, h+2), text);
    else
        painter->drawText(QRectF(-5 - w/2, -NODE_CIRCLE_SIZE-2-h, w+2, h+2), text);
}

//=========================================================================
void TaxTreeGraphNode::markDirty(qint32 dirty_flags, DirtyGNodesList *dirtyList)
{
    if ( dirtyList == NULL )
        dirtyList = &((GraphView *)view)->dirtyList;
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
  if ( QRectF(-NODE_CIRCLE_SIZE, -NODE_CIRCLE_SIZE, 2*NODE_CIRCLE_SIZE, 2*NODE_CIRCLE_SIZE).contains(event->pos()) )
    tax_node->setCollapsed(!tax_node->isCollapsed(), true);
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
    ((GraphView *)view)->nodePopupMenu->popup(event->screenPos());
}

//=========================================================================
void TaxTreeGraphNode::addToScene(QGraphicsScene *scene)
{
  if ( !tax_node->isCollapsed() )
  {
      ThreadSafeListLocker<BaseTaxNode *> locker(&tax_node->children);
      for ( TaxNodeIterator it = tax_node->children.begin(); it < tax_node->children.end(); it++ )
      {
        (*it)->getGnode()->addToScene(scene);
        scene->addItem(new Edge(this, (*it)->getGnode()));
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
        BaseTaxNode *excl;
    public:
        int children_deleted;
        GNodeDestroyer(GraphView *gv, BaseTaxNode *excludeNode): TaxNodeVisitor(LeavesToRoot, false, gv), excl(excludeNode), children_deleted(0){}
        virtual void Action(BaseTaxNode *node)
        {
            if ( excl == node )
                return;
            TaxTreeGraphNode *gnode = node->getGnode();
            if ( gnode != NULL )
            {
                graphView->dirtyList.Delete(gnode);
                delete gnode;
                children_deleted++;
            }
        }
    };
    GNodeDestroyer gNodeDestroyer(((GraphView *)view), tax_node);
    gNodeDestroyer.Visit(tax_node);
}

//=========================================================================
void TaxTreeGraphNode::onNodeCollapsed(bool collapsed)
{
    GraphView *gview = ((GraphView *)view);
    QPointF p_old = view->mapFromScene(pos());
    if ( collapsed )
    {
        // Make this small hack to visit children nodes. Otherwise current collapsed node will be skipped
        tax_node->setCollapsed(false, false);
        deleteChildrenNodes();
        tax_node->setCollapsed(true, false);
        gview->resetNodesCoordinates();
    }
    else
    {
        gview->createMissedGraphNodes();
    }
    gview->adjust_scene_boundaries();
    QPointF p_new = view->mapFromScene(pos());
    view->verticalScrollBar()->setValue(view->verticalScrollBar()->value()+p_new.y()-p_old.y());
    view->update();
}

//=========================================================================
QString TaxTreeGraphNode::text()
{
    QString txt = tax_node->getText();
    if ( txt.isEmpty() )
    {
        markDirty(DIRTY_NAME, &((GraphView *)view)->dirtyList);
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
BlastGraphNode::BlastGraphNode(GraphView *graphWidget, BlastTaxNode *node) :
    TaxTreeGraphNode(graphWidget, node)
{

}

//=========================================================================
QPainterPath BlastGraphNode::shape() const
{
    QPainterPath path = TaxTreeGraphNode::shape();
    int s = size();
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
        QColor col = color();
        col.setAlpha(64);
        gradient.setColorAt(0, col.darker(150));
        gradient.setColorAt(1, col.lighter(150));
        painter->setPen(Qt::NoPen);
        painter->setBrush(gradient);
        painter->drawEllipse(QPointF(0, 0), s, s);
    }
    TaxTreeGraphNode::paint(painter, option, widget);
}

//=========================================================================
int BlastGraphNode::size() const
{
    quint32 maxReads = ((GraphView *)view)->taxDataProvider->getMaxReads();
    if ( maxReads == 0 )
        return 0;
    quint32 reads = ((GraphView *)view)->taxDataProvider->readsById(tax_node->getId());
    if ( reads == 0 )
        return 0;
    return reads*MAX_NODE_RADIUS/maxReads;
}

//=========================================================================
void BlastGraphNode::updateToolTip()
{
    quint32 reads = ((GraphView *)view)->taxDataProvider->readsById(tax_node->getId());
    setToolTip(tax_node->getText().append("\nid = %0\nreads=%1").arg(tax_node->getId()).arg(reads));
}

//=========================================================================
//*************************************************************************
//=========================================================================
ChartGraphNode::ChartGraphNode(DataGraphicsView *view, BaseTaxNode *node):
    GraphNode(view, node),
    maxNodeRadius(MAX_NODE_RADIUS)
{

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
        QColor col = color();
        col.setAlpha(64);
        gradient.setColorAt(0, col.darker(150));
        gradient.setColorAt(1, col.lighter(150));
        painter->setPen(Qt::NoPen);
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
    maxNodeRadius = size/2;
}

//=========================================================================
int ChartGraphNode::size() const
{
    return ((qreal)reads())/((ChartView *)view)->dataProvider()->getMaxReads()*maxNodeRadius;
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
