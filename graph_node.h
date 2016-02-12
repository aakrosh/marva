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

#ifndef NODE_H
#define NODE_H

#include <QGraphicsItem>
#include <QList>

#define HOR_INTERVAL  150

class Edge;
class GraphView;
QT_BEGIN_NAMESPACE
class QGraphicsSceneMouseEvent;
QT_END_NAMESPACE

class GraphNodeVisitor;
class TaxNode;

#define DIRTY_NAME  0x1
#define DIRTY_CHILD 0x2
#define DIRTY_ALL   DIRTY_NAME|DIRTY_CHILD

class GraphNode : public QGraphicsItem
{
public:
    GraphNode(GraphView *graphWidget, TaxNode *node);
    ~GraphNode();

    QList<Edge *> edges() const;
    TaxNode *tax_node;

    void addEdge(Edge *edge);

    enum { Type = UserType + 1 };
    int type() const Q_DECL_OVERRIDE { return Type; }

    QRectF boundingRect() const Q_DECL_OVERRIDE;
    QPainterPath shape() const Q_DECL_OVERRIDE;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) Q_DECL_OVERRIDE;
    void setAsRoot(QGraphicsScene *scene);
    void assignNodeYCoordinate(int *levels, int *y);

    void setSize(int s) { size = s; }
    void setColor(quint32 c) { color = c; }
    QString text();
    QString get_const_text() const;
    bool isGreyedOut();
    void onNodeCollapsed(bool b);
    void adjustAdges();
    void markDirty(qint32 dirty_flags,QList<GraphNode *> *dirtyList);

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) Q_DECL_OVERRIDE;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) Q_DECL_OVERRIDE;
    void addToScene(QGraphicsScene *scene);

    void move(int dy);
    void moveSubTree(int dy);

private:
    GraphView *view;
    QList<Edge *> edgeList;
    int size;
    quint32 color;
    qint32 dirty;

friend class TreeLoaderThread;
friend class GraphView;
friend class GraphNodeVisitor;
};

class GraphNodeVisitor
{
private:
    bool visit_collapsed;
    inline bool shouldVisitChildren(GraphNode *node);
public:
    GraphNodeVisitor(bool _visit_collapsed);
    void visitRootLeave(GraphNode *root);
    void visitLeaveRoot(GraphNode *root);
    virtual void beforeVisitChildren(GraphNode *) {};
    virtual void afterVisitChildren(GraphNode *) {};
    virtual void makeAction(GraphNode *node, bool visit_children) = 0;
};

#endif // NODE_H
