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
#include "datagraphicsview.h"

class TaxTreeGraphNode;
class TreeLoaderThread;
class BlastData;
class QMenu;
class TaxDataProvider;

class DirtyGNodesList: public ThreadSafeList<TaxTreeGraphNode *>
{
public:
    virtual void Add(TaxTreeGraphNode *node);
};

#define MAX_NODE_SIZE 60
class BubbledGraphViewConfig : public GraphicsViewConfig
{
public:
    BubbledGraphViewConfig(): bubbleSize(MAX_NODE_SIZE), maxBubbleSize(MAX_NODE_SIZE*2){}
    int bubbleSize;
    quint32 maxBubbleSize;
};

class TreeGraphView : public DataGraphicsView
{
    Q_OBJECT

public:
    TreeGraphView(QWidget *parent, TaxNode *taxTree);
    virtual ~TreeGraphView();
    int max_node_y;
    TaxMap *tax_map;
    TreeTaxNode *root;
    QMenu *nodePopupMenu;
    QAction *hideNodeAction;
    QAction *hideAllButNodeAction;
    QAction *showAllNodesAction;
    QAction *colorAction;

    int hiddenNodes;

    void adjust_scene_boundaries();
    inline void set_vert_interval(int interval) { vert_interval = interval; }
    inline int get_vert_interval() const { return vert_interval; }
    virtual void resizeEvent(QResizeEvent *e);
    void resetNodesCoordinates();
    void generateTestNodes();
    void generateDefaultNodes();
    void AddNodeToScene(TreeTaxNode *node);
    void CreateGraphNode(TreeTaxNode *node);
    void adjustAllEdges();
    void markAllNodesDirty();
    void updateDirtyNodes(quint32 flag);
    void createMissedGraphNodes();

    DirtyGNodesList dirtyList;

    void hideNode(TreeTaxNode *node, bool resetCoordinates=true);
    void hideNodeCheckParents(TreeTaxNode *node, bool resetCoordinates=true);
    void showNode(TreeTaxNode *node, bool resetCoordinates=true);

    inline TreeTaxNode *getCurNode() { return (TreeTaxNode*) currentNode(); }

protected:
#ifndef QT_NO_WHEELEVENT
    virtual void wheelEvent(QWheelEvent *event) Q_DECL_OVERRIDE;
#endif
    virtual void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;
    virtual void showContextMenu(const QPoint&);
private:
    int hor_interval;
    int vert_interval;
    bool create_nodes;
    int treeDepth;

    void shrink_vertically(int s=4);
    void expand_vertically(int s=4);
    void updateYCoord(qreal factor);
    void updateXCoord();
    void expandPathTo(TreeTaxNode *node);
    void goUp();
    void goDown();
    void goLeft();
    void goRight();

protected:
    void setNodeInvisible(BaseTaxNode *bnode);
    void hideNodes(quint32 oldT, quint32 newT);
    void unhideNodes(quint32 oldT, quint32 newT);

signals:
    currentNodeChanged(BaseTaxNode *);
    nodeHidden(BaseTaxNode *bnode);
    allNodesShown();
private slots:
    void hideCurrent();
    void hideAllButCurrent();
    void showAllNodes();
    void changeCurrentTaxColor();
    void onNodeVisiblityChanged();
protected slots:
    virtual void onCurrentNodeChanged(BaseTaxNode *);
public slots:
    virtual void onNodeVisibilityChanged(BaseTaxNode *, bool);
    virtual void reset();
    virtual void onTreeChanged();
    virtual void onNodeNamesChanged();
};

class BlastGraphView : public TreeGraphView
{
    Q_OBJECT
public:    
    quint32 reads_threshold;
    QAction *nodeDetailsAction;
    BlastGraphView(BlastTaxDataProvider *blastTaxDataProvider, QWidget *parent, TaxNode *taxTree);
    ~BlastGraphView();
    inline BlastTaxDataProvider *blastTaxDataProvider() { return (BlastTaxDataProvider *)taxDataProvider; }
    inline BlastTaxDataProvider *blastTaxDataProvider() const { return (BlastTaxDataProvider *)taxDataProvider; }
    virtual void toJson(QJsonObject &json) const;
    virtual void fromJson(QJsonObject &json);
    BubbledGraphViewConfig *getConfig() { return (BubbledGraphViewConfig *)config; }
private slots:
    void blastLoadingProgress(void *bdata);
    void blastIsLoaded(void *bdata);
public slots:
    virtual void onReadsThresholdChanged(quint32 oldT, quint32 newT);
    virtual void onBubbleSizeChanged(quint32 /*oldS*/, quint32 /*newS*/);
    virtual void onColorChanged(BaseTaxNode *);
    virtual void showCurrentNodeDetails();
signals:
    blast_view_closed();
};

class NodePositionKeeper
{
    TreeGraphView *graphView;
    GraphNode *gnode;
    QPointF pos;
public:
    NodePositionKeeper(TreeGraphView *gv, BaseTaxNode *node);
    ~NodePositionKeeper();
};
#endif // GRAPHWIDGET_H
