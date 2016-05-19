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
#include "graph_node.h"
#include "tax_map.h"
#include "tree_tax_node.h"

#include <math.h>

#include <QPainter>
#include <QDebug>

static const double Pi = 3.14159265358979323846264338327950288419717;

//=========================================================================
Edge::Edge(TaxTreeGraphNode *sourceNode, TaxTreeGraphNode *destNode)
    : arrowSize(10)
{
    setAcceptedMouseButtons(0);
    source = sourceNode;
    dest = destNode;
    dest->addEdge(this);
    adjust();
}

//=========================================================================
TaxTreeGraphNode *Edge::sourceNode() const
{
    return source;
}

//=========================================================================
TaxTreeGraphNode *Edge::destNode() const
{
    return dest;
}

//=========================================================================
void Edge::adjust()
{
    if (!source || !dest)
        return;

    prepareGeometryChange();
    points[0] = mapFromItem(source, 10, 0);
    points[1] = mapFromItem(source, LINE_BREAK_X, 0);
    points[2] = mapFromItem(source, LINE_BREAK_X+30, dest->y()-source->y());
    points[3] = mapFromItem(dest, -10, 0);
}

//=========================================================================
QRectF Edge::boundingRect() const
{
    if (!source || !dest)
        return QRectF();

    qreal penWidth = 1;
    qreal extra = (penWidth + arrowSize) / 2.0;

    return QRectF(points[0], points[3])
        .normalized()
        .adjusted(-extra, -extra, extra, extra);
}

//=========================================================================
void Edge::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    if (!source || !dest)
        return;

    // Draw the line itself
    painter->setPen(QPen(dest->isGreyedOut() ? Qt::lightGray : Qt::black, 0.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));

    if ( dest->x()-source->x()<60 )
    {
        painter->drawLine(points[0], points[3]);
    }
    else
    {
        if ( dest->getTaxNode() != source->getTaxNode()->children[0] )
            painter->drawPolyline(&points[1], 3);
        else
            painter->drawPolyline(points, 4);
        // Draw the arrows
        QPointF destArrowP1 = points[3] + QPointF(sin(-Pi / 2.5) * arrowSize,
                                                  cos(-Pi / 2.5) * arrowSize);
        QPointF destArrowP2 = points[3] + QPointF(sin(-Pi + Pi / 2.5) * arrowSize,
                                                  cos(-Pi + Pi / 2.5) * arrowSize);
        painter->setBrush(dest->isGreyedOut() ? Qt::lightGray : Qt::black);
        painter->drawPolygon(QPolygonF() << points[3] << destArrowP1 << destArrowP2);
    }

}
