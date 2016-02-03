/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
**     of its contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "edge.h"
#include "node.h"
#include "graphwidget.h"

#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QStyleOption>
#include <QDebug>

//=========================================================================
Node::Node(GraphWidget *graphWidget, const QString &_text)
    : parent(NULL),
      graph(graphWidget),
      text(_text),
      width(1),
      level(0)
{
    setFlag(ItemIsMovable);
    setFlag(ItemSendsGeometryChanges);
    setCacheMode(DeviceCoordinateCache);
    setZValue(-1);
    setFlag(QGraphicsItem::ItemIsMovable, false);
    collapsed = true;
}

//=========================================================================
void Node::addEdge(Edge *edge)
{
    edgeList << edge;
    edge->adjust();
}

//=========================================================================
QList<Edge *> Node::edges() const
{
    return edgeList;
}

//=========================================================================
void Node::calculateForces()
{
    if (!scene() || scene()->mouseGrabberItem() == this) {
        newPos = pos();
        return;
    }

    // Sum up all forces pushing this item away
    qreal xvel = 0;
    qreal yvel = 0;
    foreach (QGraphicsItem *item, scene()->items()) {
        Node *node = qgraphicsitem_cast<Node *>(item);
        if (!node)
            continue;

        QPointF vec = mapToItem(node, 0, 0);
        qreal dx = vec.x();
        qreal dy = vec.y();
        double l = 2.0 * (dx * dx + dy * dy);
        if (l > 0) {
            xvel += (dx * 150.0) / l;
            yvel += (dy * 150.0) / l;
        }
    }
    // Now subtract all forces pulling items together
    double weight = (edgeList.size() + 1) * 10;
    foreach (Edge *edge, edgeList) {
        QPointF vec;
        if (edge->sourceNode() == this)
            vec = mapToItem(edge->destNode(), 0, 0);
        else
            vec = mapToItem(edge->sourceNode(), 0, 0);
        xvel -= vec.x() / weight;
        yvel -= vec.y() / weight;
    }
    if (qAbs(xvel) < 0.1 && qAbs(yvel) < 0.1)
        xvel = yvel = 0;
    QRectF sceneRect = scene()->sceneRect();
    newPos = pos() + QPointF(xvel, yvel);
    newPos.setX(qMin(qMax(newPos.x(), sceneRect.left() + 10), sceneRect.right() - 10));
    newPos.setY(qMin(qMax(newPos.y(), sceneRect.top() + 10), sceneRect.bottom() - 10));
}

//=========================================================================
bool Node::advance()
{
    if (newPos == pos())
        return false;

    setPos(newPos);
    return true;
}

//=========================================================================
QRectF Node::boundingRect() const
{
    qreal adjust = 2;
    QFont font;
    QFontMetricsF fm(font);
    qreal w = fm.width(text);
    qreal h = fm.height();
    return QRectF( -10 - adjust, -10 -h - adjust, 26 + w + adjust, 23 + h + adjust);
}

//=========================================================================
QPainterPath Node::shape() const
{
    QPainterPath path;
    path.addEllipse(-10, -10, 20, 20);
    QFont font;
    QFontMetricsF fm(font);
    path.addRect(15, -15, fm.width(text)+2, fm.height()+2);
    return path;
}

//=========================================================================
void Node::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *)
{
    painter->setPen(Qt::NoPen);
    painter->setBrush(Qt::darkGray);
    painter->drawEllipse(-7, -7, 20, 20);

    QRadialGradient gradient(-3, -3, 10);
    if (option->state & QStyle::State_Sunken)
    {
      gradient.setCenter(3, 3);
      gradient.setFocalPoint(3, 3);
      gradient.setColorAt(1, QColor(0x88FFFF00).light(120));
      gradient.setColorAt(0, QColor(0xAA008800).light(120));
    }
    else
    {
      gradient.setColorAt(0, QColor(0x88FFFF00));
      gradient.setColorAt(1, QColor(0xAA888800));
    }
    painter->setBrush(gradient);

    painter->setPen(QPen(Qt::transparent, 0));
    painter->drawEllipse(-10, -10, 20, 20);

    QFont font;
    QFontMetricsF fm(font);
    painter->setPen(Qt::SolidLine);
    painter->setBrush(Qt::darkGray);
    if ( !children.isEmpty() )
    {
      painter->drawLine(-7, 0, 7, 0);
      if ( collapsed )
        painter->drawLine(0 , -7, 0, 7);
    }

    painter->drawText(15, -15, text);
}

//=========================================================================
QVariant Node::itemChange(GraphicsItemChange change, const QVariant &value)
{
    switch (change)
    {
      case ItemPositionHasChanged:
          foreach (Edge *edge, edgeList)
              edge->adjust();
          graph->itemMoved();
          break;
      default:
          break;
    };

    return QGraphicsItem::itemChange(change, value);
}

//=========================================================================
void Node::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
  setCollapsed(!collapsed);
  update();
  QGraphicsItem::mousePressEvent(event);
}

//=========================================================================
void Node::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
  update();
  QGraphicsItem::mouseReleaseEvent(event);
}

static int hor_len=0;
//=========================================================================
void Node::addToScene(QGraphicsScene *scene)
{
  if ( !collapsed )
  {
      foreach(Node *n, children)
      {
        n->addToScene(scene);
        scene->addItem(new Edge(this, n));
      }
  }

  if ( collapsed || children.size() == 0 )
      hor_len = level == 0 ? 0 : scene->width()*0.9/level;
  QPointF p = scenePos();
  p.setX(hor_len * level);
  setPos(p);
  scene->addItem(this);
}

//=========================================================================
void Node::setAsRoot(QGraphicsScene *scene)
{
  foreach(QGraphicsItem *i, scene->items())
    scene->removeItem(i);
  scene->addRect(graph->sceneRect());
  addToScene(scene);
}

//=========================================================================
void Node::move(int dy)
{
 QPointF p = pos();
 p.setY(p.y()+dy);
 setPos(p);
}

//=========================================================================
void Node::moveSubTree(int dy)
{
    move(dy);
    foreach ( Node *ch, children )
      if ( !ch->collapsed )
        ch->moveSubTree(dy);
}

//=========================================================================
void Node::setCollapsed(bool b)
{
  collapsed = b;
  graph->resetNodesCoordinates();
  graph->reset();
  graph->adjust_scene_boundaries();
  //graph->fitInView(graph->sceneRect(), Qt::KeepAspectRatio);
  qreal m11 = graph->transform().m11();

  if ( m11 > 1 )
  {
      qreal rm11 = 1.0/m11;
      graph->scale(rm11, rm11);
  }
}

//=========================================================================
void Node::assignNodeCoordinates(int *levels, int *y, int *x, int *width)
{
    if ( parent == NULL )
        graph->max_node_y = -graph->get_vert_interval();
    *y += *width/2;
    int oldY = *y+graph->get_vert_interval() ;
    if ( collapsed || children.isEmpty() )
    {
        *levels = this->level;
        *x = graph->sceneRect().width()-100;
        *width = 0;
        graph->max_node_y += graph->get_vert_interval();
        *y = graph->max_node_y;
    }
    else
    {
        int sumy = 0;
        foreach (Node *n, children)
        {
            n->assignNodeCoordinates(levels, y, x, width);
            sumy += *y;
        }
        sumy /= children.size();
        //*width = *y - oldY;
        //*y = oldY+*width/2;
        *y = sumy;
    }

    qDebug() << "Node " << text;
    qDebug() << "Set node position " << *x << " " << *y;
    setPos(*x, *y);
    qDebug() << " return width = " << *width;
    *x -= (graph->sceneRect().width()-100)/level;
}
