
#include "edge.h"
#include "graph_node.h"
#include "graphwidget.h"
#include "tax_map.h"

#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QStyleOption>
#include <QDebug>

bool GraphNodeVisitor::shouldVisitChildren(GraphNode *node)
{
    return (!node->tax_node->children.isEmpty() && (visit_collapsed || !node->tax_node->isCollapsed()));
}

GraphNodeVisitor::GraphNodeVisitor(bool _visit_collapsed) :visit_collapsed(_visit_collapsed){}

void GraphNodeVisitor::visitRootLeave(GraphNode *root)
{
    if ( root == NULL )
        return;
    bool visit_children = shouldVisitChildren(root);
    makeAction(root, visit_children);
    if ( !visit_children )
        return;
    beforeVisitChildren(root);
    QList<TaxNode *> &list = root->tax_node->children;
    for ( TaxNodeIterator it = list.begin(); it < list.end(); it++ )
        visitRootLeave((*it)->gnode);
    afterVisitChildren(root);
}

void GraphNodeVisitor::visitLeaveRoot(GraphNode *root)
{
    if ( root == NULL )
        return;
    bool visit_children = shouldVisitChildren(root);
    if ( visit_children )
    {
        beforeVisitChildren(root);
        QList<TaxNode *> &list = root->tax_node->children;
        for ( TaxNodeIterator it = list.begin(); it < list.end(); it++ )
            visitLeaveRoot((*it)->gnode);
        afterVisitChildren(root);
    }
    makeAction(root, visit_children);
}

//=========================================================================
GraphNode::GraphNode(GraphView *graphWidget, TaxNode *node)
    :
      tax_node(node),
      view(graphWidget)
{
    setFlag(ItemSendsGeometryChanges);
    setCacheMode(DeviceCoordinateCache);
    size = 0;
    color = 0x00000000;
    dirty = 0;
    node->gnode = this;
}

//=========================================================================
GraphNode::~GraphNode()
{
    tax_node->gnode = NULL;
    tax_node = NULL;
}

//=========================================================================
void GraphNode::addEdge(Edge *edge)
{
    edgeList << edge;
    edge->adjust();
}

//=========================================================================
QList<Edge *> GraphNode::edges() const
{
    return edgeList;
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
    path.addEllipse(QPointF(0, 0), 10, 10);
    QFont font;
    QFontMetricsF fm(font);
    qreal w = fm.width(get_const_text());
    qreal h = fm.height();
    if ( tax_node->children.isEmpty() || tax_node->isCollapsed() )
        path.addRect(12, -h/2, w+2, h+2);
    else
        path.addRect(-5 - w/2, -12-h, w+2, h+2);
    if ( size > 0 )
        path.addEllipse(QPointF(0, 0), size, size);
    QPainterPath result;
    result.addRect(path.boundingRect());
    return result;
}

//=========================================================================
void GraphNode::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    if ( size > 0 )
    {
        QRadialGradient gradient(QPointF(0 ,0 ), size);
        gradient.setColorAt(0, QColor(color).darker(150));
        gradient.setColorAt(1, QColor(color).lighter(150));
        painter->setPen(Qt::NoPen);
        painter->setBrush(gradient);
        painter->drawEllipse(QPointF(0, 0), size, size);
    }
    painter->setBrush(Qt::transparent);
    painter->setPen(QPen(tax_node->id >= 0 ? Qt::black : Qt::lightGray, 0));
    painter->drawEllipse(QPointF(0, 0), 10, 10);

    QFont font;
    QFontMetricsF fm(font);
    painter->setPen(Qt::SolidLine);
    painter->setBrush(Qt::lightGray);
    if ( !tax_node->children.isEmpty() )
    {
      painter->drawLine(-7, 0, 7, 0);
      if ( tax_node->isCollapsed() )
        painter->drawLine(0 , -7, 0, 7);            
    }
    QString text = this->text();
    qreal w = fm.width(text);
    qreal h = fm.height();
    if ( tax_node->children.isEmpty() || tax_node->isCollapsed() )
        painter->drawText(QRectF(12, -h/2, w+2, h+2), text);
    else
        painter->drawText(QRectF(-5 - w/2, -12-h, w+2, h+2), text);

    painter->setPen(Qt::black);
    painter->setBrush(Qt::transparent);
}

//=========================================================================
void GraphNode::adjustAdges()
{
    if ( tax_node->isCollapsed() )
        return;
    foreach (Edge *edge, edgeList)
        edge->adjust();
}

//=========================================================================
void GraphNode::markDirty(qint32 dirty_flags, QList<GraphNode *> *dirtyList)
{
    if ( (dirty & dirty_flags) == dirty_flags )
        return;
    dirty |= dirty_flags;
    dirtyList->append(this);
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
  tax_node->setCollapsed(!tax_node->isCollapsed());
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
      for ( TaxNodeIterator it = tax_node->children.begin(); it < tax_node->children.end(); it++ )
      {
        (*it)->gnode->addToScene(scene);
        scene->addItem(new Edge(this, (*it)->gnode));
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
void GraphNode::moveSubTree(int dy)
{
    move(dy);
  /*  foreach ( GraphNode *ch, children )
      if ( !ch->collapsed )
        ch->moveSubTree(dy);*/
}

//=========================================================================
void GraphNode::onNodeCollapsed(bool)
{
    view->reset();
    //view->update();
    view->adjust_scene_boundaries();
}

//=========================================================================
QString GraphNode::text()
{
    TaxMapIterator it = view->tax_map.find(tax_node->id);
    if ( it == view->tax_map.end() )
    {
        markDirty(DIRTY_NAME, &view->dirtyList);
        return (QString("%0").arg(tax_node->id));
    }
    return QString(it.value()).append("(%0)").arg(tax_node->id);
}

//=========================================================================
QString GraphNode::get_const_text() const
{
    TaxMapIterator it = view->tax_map.find(tax_node->id);
    if ( it == view->tax_map.end() )
        return (QString("%0").arg(tax_node->id));
    return QString(it.value()).append("(%0)").arg(tax_node->id);
}

//=========================================================================
bool GraphNode::isGreyedOut()
{
    return tax_node->id <= 0;
}
