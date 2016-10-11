#ifndef GRAPHWIDGET_H
#define GRAPHWIDGET_H

#include <QGraphicsView>
#include "tax_map.h"
#include "threadsafelist.h"
#include "blast_data.h"
#include "datagraphicsview.h"
#include "config.h"

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
    BubbledGraphViewConfig(): bubbleSize(MAX_NODE_SIZE), maxBubbleSize(MAX_NODE_SIZE*2), calcMethod(METHOD_LINEAR){}
    int bubbleSize;
    quint32 maxBubbleSize;
    int calcMethod;

    void toJson(QJsonObject &);
    void fromJson(QJsonObject &);
};

class TreeGraphView : public DataGraphicsView
{
    Q_OBJECT

public:
    TreeGraphView(QWidget *parent, TaxNode *taxTree);
    virtual ~TreeGraphView();
    int max_node_y;
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

    int hor_interval;
protected:
#ifndef QT_NO_WHEELEVENT
    virtual void wheelEvent(QWheelEvent *event) Q_DECL_OVERRIDE;
#endif
    virtual void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;
    virtual void showContextMenu(const QPoint&);
private:
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
    void setNodeInvisible(TreeTaxNode *bnode);
    void hideNodes(quint32 oldT, quint32 newT);
    void unhideNodes(quint32 oldT, quint32 newT);

signals:
    void currentNodeChanged(BaseTaxNode *);
    void nodeHidden(BaseTaxNode *bnode);
    void allNodesShown();
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


struct BlastGraphViewState
{
    quint32 threshold;
    TaxRank rank;
    BlastGraphViewState() :threshold(0), rank(TR_SPECIES){}
};

class BlastGraphView : public TreeGraphView
{
    Q_OBJECT
public:    
    BlastGraphViewState state;
    QAction *nodeDetailsAction;
    BlastFileType type;
    bool dirty;
    BlastGraphView(BlastTaxDataProvider *blastTaxDataProvider, QWidget *parent, TaxNode *taxTree);
    ~BlastGraphView();
    inline BlastTaxDataProvider *blastTaxDataProvider() { return (BlastTaxDataProvider *)taxDataProvider; }
    inline BlastTaxDataProvider *blastTaxDataProvider() const { return (BlastTaxDataProvider *)taxDataProvider; }
    virtual void toJson(QJsonObject &json) const;
    virtual void fromJson(QJsonObject &json);
    BubbledGraphViewConfig *getConfig() { return (BubbledGraphViewConfig *)config; }
    void updateView();

protected:
    virtual void showEvent(QShowEvent * event);
    virtual void hideEvent(QHideEvent *event);
    void onTaxRankChanged(TaxRank rank, BlastTaxNode *node);
private slots:
    void blastLoadingProgress(LoaderThread *loader);
    void blastIsLoaded(LoaderThread *loader);
public slots:
    virtual void onReadsThresholdChanged(quint32 oldT, quint32 newT);
    virtual void onBubbleSizeChanged(quint32 /*oldS*/, quint32 /*newS*/);
    virtual void onTaxRankChanged(TaxRank);
    virtual void onColorChanged(BaseTaxNode *);
    virtual void showCurrentNodeDetails();
signals:
    void blast_view_closed();
};

class NodePositionKeeper
{
public:
    TreeGraphView *graphView;
    GraphNode *gnode;
    QPointF pos;
    NodePositionKeeper(TreeGraphView *gv, BaseTaxNode *node);
    ~NodePositionKeeper();
};
#endif // GRAPHWIDGET_H
