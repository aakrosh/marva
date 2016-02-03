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

#include <math.h>

#include <QPainter>

static const double Pi = 3.14159265358979323846264338327950288419717;
//static double TwoPi = 2.0 * Pi;

//=========================================================================
Edge::Edge(Node *sourceNode, Node *destNode)
    : arrowSize(10)
{
    setAcceptedMouseButtons(0);
    source = sourceNode;
    dest = destNode;
    source->addEdge(this);
    dest->addEdge(this);
    adjust();
}

//=========================================================================
Node *Edge::sourceNode() const
{
    return source;
}

//=========================================================================
Node *Edge::destNode() const
{
    return dest;
}

//=========================================================================
void Edge::adjust()
{
    if (!source || !dest)
        return;

    QPointF sp = mapFromItem(source, 0, 0);
    QPointF dp = mapFromItem(dest, 0, 0);

    QLineF line1(sp, QPointF(sp.x(), dp.y()));
    QLineF line2(QPointF(sp.x(), dp.y()), dp);

    qreal length2 = line2.length();
    qreal length1 = line1.length();

    prepareGeometryChange();

    if ( length2 > qreal(20.))
    {
       if ( length1 != 0 )
       {
         QPointF edgeOffset1((line1.dx() * 10) / length1, (line1.dy() * 10) / length1);
         sourcePoint = line1.p1() + edgeOffset1;
       }
       else
       {
         sourcePoint = line1.p1() + QPointF(10, 0);
       }
       QPointF edgeOffset2((line2.dx() * 10) / length2, (line2.dy() * 10) / length2);
       destPoint = line2.p2() - edgeOffset2;
    }
    else
    {
        sourcePoint = destPoint = line1.p2();
    }
}

//=========================================================================
QRectF Edge::boundingRect() const
{
    if (!source || !dest)
        return QRectF();

    qreal penWidth = 1;
    qreal extra = (penWidth + arrowSize) / 2.0;

    return QRectF(sourcePoint, QSizeF(destPoint.x() - sourcePoint.x(),
                                      destPoint.y() - sourcePoint.y()))
        .normalized()
        .adjusted(-extra, -extra, extra, extra);
}

//=========================================================================
void Edge::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    if (!source || !dest)
        return;

    QLineF line1(QPointF(sourcePoint.x(), sourcePoint.y()), QPointF(sourcePoint.x(), destPoint.y()));
    QLineF line2(QPointF(sourcePoint.x(), destPoint.y()), QPointF(destPoint.x(), destPoint.y()));

    // Draw the line itself
    painter->setPen(QPen(Qt::black, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter->drawLine(line1);
    painter->drawLine(line2);

    // Draw the arrows
    QPointF destArrowP1 = destPoint + QPointF(sin(-Pi / 3) * arrowSize,
                                              cos(-Pi / 3) * arrowSize);
    QPointF destArrowP2 = destPoint + QPointF(sin(-Pi + Pi / 3) * arrowSize,
                                              cos(-Pi + Pi / 3) * arrowSize);

    painter->setBrush(Qt::black);
    painter->drawPolygon(QPolygonF() << line2.p2() << destArrowP1 << destArrowP2);
}
