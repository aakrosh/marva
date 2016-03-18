
#include "edge.h"
#include "graph_node.h"
#include "graphwidget.h"
#include "tax_map.h"

#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QStyleOption>
#include <QDebug>
#include <QScrollBar>

#define NODE_CIRCLE_SIZE  8
#define HALF_PLUS_SIZE 3
#define MAX_NODE_SIZE  10

//=========================================================================
GraphNode::GraphNode(GraphView *graphWidget, BaseTaxNode *node)
    :
      tax_node(node),
      edge(NULL),
      view(graphWidget)
{
    setFlag(ItemSendsGeometryChanges);
    setCacheMode(DeviceCoordinateCache);
    dirty = 0;
    node->gnode = this;
    updateToolTip();
}

//=========================================================================
GraphNode::~GraphNode()
{
    if  ( edge != NULL )
        delete edge;
    tax_node->removeGnode();
    tax_node = NULL;
}

//=========================================================================
void GraphNode::addEdge(Edge *_edge)
{
    //edgeList << edge;
    edge = _edge;
    edge->adjust();
}

//=========================================================================
QRectF GraphNode::boundingRect() const
{
    return shape().boundingRect();
}

//=========================================================================
QPainterPath GraphNode::shape() const
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
void GraphNode::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
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
void GraphNode::adjustAdges()
{
    if ( tax_node->isCollapsed() )
        return;
    if ( edge != NULL )
        edge->adjust();
}

//=========================================================================
void GraphNode::markDirty(qint32 dirty_flags, DirtyGNodesList *dirtyList)
{
    if ( dirtyList == NULL )
        dirtyList = &view->dirtyList;
    if ( (dirty & dirty_flags) == dirty_flags )
        return;
    dirty |= dirty_flags;
    dirtyList->Add(this);
}

//=========================================================================
void GraphNode::clearDirtyFlag(qint32 dirty_flag)
{
    dirty &= ~dirty_flag;
    updateToolTip();
}

//=========================================================================
QVariant GraphNode::itemChange(GraphicsItemChange change, const QVariant &value)
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
void GraphNode::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
  view->setCurrentNode(tax_node);
  if ( QRectF(-NODE_CIRCLE_SIZE, -NODE_CIRCLE_SIZE, 2*NODE_CIRCLE_SIZE, 2*NODE_CIRCLE_SIZE).contains(event->pos()) )
    tax_node->setCollapsed(!tax_node->isCollapsed(), true);
  update();
  QGraphicsItem::mousePressEvent(event);
}

//=========================================================================
void GraphNode::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
  update();
  QGraphicsItem::mouseReleaseEvent(event);
}

//=========================================================================
void GraphNode::addToScene(QGraphicsScene *scene)
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
void GraphNode::setAsRoot(QGraphicsScene *scene)
{
  foreach(QGraphicsItem *i, scene->items())
    scene->removeItem(i);
  addToScene(scene);
}

//=========================================================================
void GraphNode::move(int dy)
{
     QPointF p = pos();
     p.setY(p.y()+dy);
     setPos(p);
}

//=========================================================================
void GraphNode::deleteChildrenNodes()
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
            GraphNode *gnode = node->getGnode();
            if ( gnode != NULL )
            {
                graphView->dirtyList.Delete(gnode);
                if ( gnode->edge != NULL )
                    graphView->scene()->removeItem(gnode->edge);
                graphView->scene()->removeItem(gnode);
                delete gnode;
                children_deleted++;
            }
        }
    };
    GNodeDestroyer gNodeDestroyer(view, tax_node);
    gNodeDestroyer.Visit(tax_node);
}

//=========================================================================
void GraphNode::updateToolTip()
{
    setToolTip(tax_node->getText().append("\nid = %1").arg(tax_node->getId()));
}

//=========================================================================
void GraphNode::onNodeCollapsed(bool collapsed)
{
    QPointF p_old = view->mapFromScene(pos());
    if ( collapsed )
    {
        // Make this small hack to visit children nodes. Otherwise current collapsed node will be skipped
        tax_node->setCollapsed(false, false);
        deleteChildrenNodes();
        tax_node->setCollapsed(true, false);
        view->resetNodesCoordinates();
    }
    else
    {
        view->createMissedGraphNodes();    
    }
    view->adjust_scene_boundaries();
    QPointF p_new = view->mapFromScene(pos());
    view->verticalScrollBar()->setValue(view->verticalScrollBar()->value()+p_new.y()-p_old.y());
    view->update();
}

//=========================================================================
QString GraphNode::text()
{
    QString txt = tax_node->getText();
    if ( txt.isEmpty() )
    {
        markDirty(DIRTY_NAME, &view->dirtyList);
        return (QString("%0").arg(tax_node->getId()));
    }
    return QString(txt).append("(%0)").arg(tax_node->getId());
}

//=========================================================================
QString GraphNode::get_const_text() const
{
    return QString(tax_node->getText()).append("(%0)").arg(tax_node->getId());
}

//=========================================================================
bool GraphNode::isGreyedOut()
{
    return tax_node->getId() <= 0;
}

//=========================================================================
//*************************************************************************
//=========================================================================
BlastGraphNode::BlastGraphNode(GraphView *graphWidget, BlastTaxNode *node) :
    GraphNode(graphWidget, node)
{

}

//=========================================================================
QPainterPath BlastGraphNode::shape() const
{
    QPainterPath path = GraphNode::shape();
    int s = size()/10;
    if ( s > 10 )
    {
        if ( s > 1000 )
            s = 24;
        else if ( s > 10 )
            s = 15;
        path.addEllipse(QPointF(0, 0), s, s);
    }
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
        s /= 10;
        if ( s <= MAX_NODE_SIZE )
        {
            painter->drawEllipse(QPointF(0, 0), s, s);
        }
        else
        {
            int d = 100;
            qreal f = 10;
            qreal w = 3.;
            painter->drawEllipse(QPointF(0, 0), f, f);
            QPen pen(col);
            painter->setBrush(Qt::transparent);
            while ( s >= MAX_NODE_SIZE )
            {
                s /= d;
                //d *= 10;
                pen.setWidth(w);
                w *= 0.8;
                f *= 1.5;
                painter->setPen(pen);
                painter->drawEllipse(QPointF(0, 0), f, f);
            }
        }
    }
    GraphNode::paint(painter, option, widget);
}

//=========================================================================
int BlastGraphNode::size() const
{
    BlastNodeMap::iterator it = view->blastNodeMap->find(tax_node->getId());
    if ( it == view->blastNodeMap->end() )
        return 0;
    return it.value()->count;
}

//=========================================================================
quint32 BlastGraphNode::color()
{
    return taxColorSrc.getColor(tax_node->getId());
}

//=========================================================================
void BlastGraphNode::updateToolTip()
{
    setToolTip(tax_node->getText().append("\nid = %0\nreads=%1").arg(tax_node->getId()).arg(size()));
}
