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

#include "blast_data.h"

#include <QGraphicsItem>
#include <QList>

class Edge;
class TreeGraphView;
QT_BEGIN_NAMESPACE
class QGraphicsSceneMouseEvent;
QT_END_NAMESPACE

class GraphNodeVisitor;
class BaseTaxNode;
class DirtyGNodesList;
class DataGraphicsView;

#define DIRTY_NAME  0x1
#define DIRTY_CHILD 0x2
#define DIRTY_ALL   DIRTY_NAME|DIRTY_CHILD
#define GRAPHICS_NODE_TYPE  UserType+1

class GraphNode : public QGraphicsItem
{
public:
    GraphNode(DataGraphicsView *view, BaseTaxNode *node);
    ~GraphNode();

    enum { Type = GRAPHICS_NODE_TYPE };
    int type() const Q_DECL_OVERRIDE { return Type; }

    virtual QRectF boundingRect() const Q_DECL_OVERRIDE;
    virtual QPainterPath shape() const Q_DECL_OVERRIDE = 0;
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) Q_DECL_OVERRIDE = 0;
    quint32 color();
    inline BaseTaxNode *getTaxNode() { return tax_node; }

protected:
    virtual void updateToolTip();
    DataGraphicsView *view;
    BaseTaxNode *tax_node;
};

class TreeTaxNode;

class TaxTreeGraphNode : public GraphNode
{
public:
    TaxTreeGraphNode(TreeGraphView *gView, TreeTaxNode *node);
    ~TaxTreeGraphNode();

    QPainterPath shape() const Q_DECL_OVERRIDE;
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) Q_DECL_OVERRIDE;
    virtual void setAsRoot(QGraphicsScene *scene);

    QString text();
    QString get_const_text() const;
    bool isGreyedOut();
    void onNodeCollapsed(bool b);
    void markDirty(qint32 dirty_flags, DirtyGNodesList *dirtyList=NULL);
    void clearDirtyFlag(qint32 dirty_flag);

    inline TreeTaxNode *getTaxNode() const { return (TreeTaxNode *)tax_node; }
    inline TreeGraphView *getView() const { return (TreeGraphView *)view; }

    virtual void addEdge(Edge *edge);
    void adjustAdges();

    Edge *edge;

protected:
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value) Q_DECL_OVERRIDE;
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) Q_DECL_OVERRIDE;
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) Q_DECL_OVERRIDE;
    virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) Q_DECL_OVERRIDE;
    void addToScene(QGraphicsScene *scene);
    void deleteChildrenNodes();
    qint32 dirty;

friend class TreeLoaderThread;
friend class TreeGraphView;
QPainterPath getTextPath() const;
};

class BlastGraphNode: public TaxTreeGraphNode
{
public:
    BlastGraphNode(TreeGraphView *graphWidget, BlastTaxNode *node);
    QPainterPath shape() const Q_DECL_OVERRIDE;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) Q_DECL_OVERRIDE;
    int size() const;
    inline BlastTaxNode *getTaxNode() { return (BlastTaxNode *)tax_node; }
    inline BlastTaxNode *getTaxNode() const { return (BlastTaxNode *)tax_node; }
protected:
    virtual void updateToolTip();
};

class ChartGraphNode: public GraphNode
{
public:
    ChartGraphNode(DataGraphicsView *view, BaseTaxNode *node);
    ~ChartGraphNode(){}
    QPainterPath shape() const Q_DECL_OVERRIDE;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) Q_DECL_OVERRIDE;
    void setMaxNodeSize(quint32 size);
    virtual void updateToolTip();
    inline BaseTaxNode *getTaxNode() { return (BaseTaxNode *)tax_node; }
protected:
    int size() const;
    quint32 reads() const;
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) Q_DECL_OVERRIDE;

private:
    quint32 maxNodeRadius;
};

#endif // NODE_H
