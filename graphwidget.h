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

#ifndef GRAPHWIDGET_H
#define GRAPHWIDGET_H

#include <QGraphicsView>
#include "tax_map.h"
#include "threadsafelist.h"
#include "blast_data.h"

class GraphNode;
class TreeLoaderThread;
class TaxColorSrc;
class BlastData;
class QMenu;
class TaxDataProvider;

class DirtyGNodesList: public ThreadSafeList<GraphNode *>
{
public:
    virtual void Add(GraphNode *node);
};

class GraphView : public QGraphicsView
{
    Q_OBJECT

public:
    GraphView(QWidget *parent, TaxNode *taxTree);
    ~GraphView();
    int max_node_y;
    TaxMap *tax_map;
    BaseTaxNode *root;
    //BlastNodeMap *blastNodeMap;
    TaxDataProvider *taxDataProvider;
    QMenu *nodePopupMenu;
    QAction *hideNodeAction;
    bool persistant;

    void adjust_scene_boundaries();
    inline void set_vert_interval(int interval) { vert_interval = interval; }
    inline int get_vert_interval() const { return vert_interval; }
    virtual void resizeEvent(QResizeEvent *e);
    void resetNodesCoordinates();
    void generateTestNodes();
    void generateDefaultNodes();
    void AddNodeToScene(BaseTaxNode *node);
    void CreateGraphNode(BaseTaxNode *node);
    void adjustAllEdges();
    void markAllNodesDirty();
    void updateDirtyNodes(quint32 flag);
    void createMissedGraphNodes();
    void setCurrentNode(BaseTaxNode *);
    inline BaseTaxNode *currentNode() { return curNode; }

    DirtyGNodesList dirtyList;

    void hideNode(BaseTaxNode *node, bool resetCoordinates=true);
    void showNode(BaseTaxNode *node);

protected:
#ifndef QT_NO_WHEELEVENT
    virtual void wheelEvent(QWheelEvent *event) Q_DECL_OVERRIDE;
#endif
    virtual void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;
private:
    int hor_interval;
    int vert_interval;
    bool create_nodes;
    int treeDepth;
    BaseTaxNode *curNode;

    void shrink_vertically(int s=4);
    void expand_vertically(int s=4);
    void updateYCoord(qreal factor);
    void updateXCoord();
    void expandPathTo(BaseTaxNode *node);
    void goUp();
    void goDown();
    void goLeft();
    void goRight();

    void hideNodes(quint32 oldT, quint32 newT);
    void unhideNodes(quint32 oldT, quint32 newT);

signals:
    currentNodeChanged(BaseTaxNode *);
private slots:
    void blastLoadingProgress(void *bdata);
    void blastIsLoaded(void *bdata);
    void onCurrentNodeChanged(BaseTaxNode *);
    void hideCurrent();
public slots:
    void onNodeVisibilityChanged(BaseTaxNode*,bool);
    void onReadsThresholdChanged(quint32 oldT, quint32 newT);
    void reset();
    void onTreeChanged();
    void onNodeNamesChanged();
};

#endif // GRAPHWIDGET_H
