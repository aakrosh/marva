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

#include "graphwidget.h"
#include "edge.h"
#include "node.h"

#include <math.h>

#include <QKeyEvent>
#include <QDebug>
#include <QApplication>

//=========================================================================
GraphWidget::GraphWidget(QWidget *parent)
    : QGraphicsView(parent), timerId(0)
{
    QGraphicsScene *scene = new QGraphicsScene(this);
    scene->setItemIndexMethod(QGraphicsScene::NoIndex);
    setScene(scene);
    scene->setSceneRect(0, 0, size().width(), 200);
    setCacheMode(CacheBackground);
    setViewportUpdateMode(BoundingRectViewportUpdate);
    setRenderHint(QPainter::Antialiasing);
    setTransformationAnchor(AnchorUnderMouse);
    setMinimumSize(200, 300);
    setWindowTitle(tr("Tree view"));
    setDragMode(QGraphicsView::ScrollHandDrag);
    root = new Node(this, "Root item");
    root->setPos(0, 0);
    hor_interval = 300;
    set_vert_interval(80);
    GenerateChildrenNodes(root, 30, "Item ");
    Node *ch2 = root->children.at(1);
    GenerateChildrenNodes(ch2, 200, "Item 2");
    GenerateChildrenNodes(root->children.at(0), 20, "Item 2");
    GenerateChildrenNodes(root->children.at(2), 400, "Item 2");
    GenerateChildrenNodes(ch2->children.at(1), 500, "Item 21");
    root->setCollapsed(false);
    reset();
}

//=========================================================================
void GraphWidget::resizeEvent(QResizeEvent *e)
{
    QRectF r = sceneRect();
    r.setWidth(e->size().width()-20);
    setSceneRect(r);
    scene()->setSceneRect(r);
    reset();
}

//=========================================================================
void GraphWidget::GenerateChildrenNodes(Node *parent, int child_num, QString prefix)
{
    for ( int i=0; i < child_num; i++ )
        parent->addChild( new Node(this, QString(prefix).append(QString::number(i))));
}

//=========================================================================
void GraphWidget::adjust_scene_boundaries()
{
    setSceneRect(this->scene()->itemsBoundingRect());
}

//=========================================================================
void GraphWidget::reset()
{
  root->setAsRoot(scene());
}

//=========================================================================
void GraphWidget::itemMoved()
{
/*    if (!timerId)
        timerId = startTimer(1000 / 25);*/
}

//=========================================================================
void GraphWidget::keyPressEvent(QKeyEvent *event)
{
    switch (event->key())
    {
      /*
      case Qt::Key_Up:
          centerNode->moveBy(0, -20);
          break;
      case Qt::Key_Down:
          centerNode->moveBy(0, 20);
          break;
      case Qt::Key_Left:
          centerNode->moveBy(-20, 0);
          break;
      case Qt::Key_Right:
          centerNode->moveBy(20, 0);
          break;
          */
      case Qt::Key_Plus:
          zoomIn();
          break;
      case Qt::Key_Minus:
          zoomOut();
          break;
          /*
      case Qt::Key_Space:
      case Qt::Key_Enter:
          shuffle();
          break;*/
      default:
          QGraphicsView::keyPressEvent(event);
    }
}

#ifndef QT_NO_WHEELEVENT

//=========================================================================
void GraphWidget::wheelEvent(QWheelEvent *event)
{
    if ( (QApplication::keyboardModifiers() & Qt::ControlModifier) == Qt::ControlModifier )
    {
        if ( event->delta()>0 )
        {
            if ( get_vert_interval() >= 20 )
                shrink_vertically();
         //   else
         //       scaleView(pow((double)2, -event->delta() / 480.0));
        }
        else
        {
            //qreal m11 = transform().m11();

            if ( get_vert_interval() <= 100 )
                expand_vertically();
        //    else
        //        scaleView(pow((double)2, -event->delta() / 480.0));
        }
    }
    else
    {
        QGraphicsView::wheelEvent(event);
    }
        //scaleView(1 / qreal(1.2));
  //const QPointF p0scene = mapToScene(event->pos());
  //scaleView(pow((double)2, -event->delta() / 240.0));
 // const QPointF p1mouse = mapFromScene(p0scene);
 // QPointF d = p1mouse - event->pos();
  //transform().translate(d.x(), d.y());
     /*
  const QPointF p0scene = mapToScene(event->pos());

  qreal factor = std::pow(1.01, event->delta());
  scale(factor, factor);

  const QPointF p1mouse = mapFromScene(p0scene);
  const QPointF move = p1mouse - event->pos(); // The move
  horizontalScrollBar()->setValue(move.x() + horizontalScrollBar()->value());
 // verticalScrollBar()->setValue(move.y() + verticalScrollBar()->value());*/
}
#endif

//=========================================================================
void GraphWidget::scaleView(qreal scaleFactor)
{
    qreal factor = transform().scale(scaleFactor, scaleFactor).mapRect(QRectF(0, 0, 1, 1)).width();
    if (factor < 0.07 || factor > 100)
        return;
    scale(scaleFactor, scaleFactor);
}

//=========================================================================
void GraphWidget::shuffle()
{
    foreach (QGraphicsItem *item, scene()->items()) {
        if (qgraphicsitem_cast<Node *>(item))
            item->setPos(-150 + qrand() % 300, -150 + qrand() % 300);
    }
}

//=========================================================================
void GraphWidget::zoomIn()
{
    scaleView(qreal(1.2));
}

//=========================================================================
void GraphWidget::zoomOut()
{
    if ( get_vert_interval() < 20 )
        scaleView(1 / qreal(1.2));
    else
        shrink_vertically();
}

//=========================================================================
void GraphWidget::shrink_vertically(int s)
{
    vert_interval-=s;
    resetNodesCoordinates();
    reset();
}

//=========================================================================
void GraphWidget::expand_vertically(int s)
{
    vert_interval+=s;
    resetNodesCoordinates();
    reset();
}

//=========================================================================
void GraphWidget::resetNodesCoordinates()
{
    int x = this->sceneRect().width()-100;
    int y = -get_vert_interval();
    int levels = 0;
    int width = 0;
    root->assignNodeCoordinates(&levels, &y, &x, &width);
    QRectF sr = sceneRect();
    //if ( max_node_y > sr.height() )
    //{
        sr.setHeight(max_node_y+40);
        setSceneRect(sr);
        /*
        int n = max_node_y / vert_interval;
        int vi = sceneRect().height()/n;
        set_vert_interval(vi < 20 ? 20 : vi);
        int x = sceneRect().width()-100;
        int y = -get_vert_interval();
        int levels = 0;
        int width = 0;
        root->assignNodeCoordinates(&levels, &y, &x, &width);
        */
    //}
}
