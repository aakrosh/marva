#include <QKeyEvent>
#include <QDebug>
#include <QApplication>

#include <QMessageBox>

#include "graphwidget.h"
#include "edge.h"
#include "graph_node.h"
#include "map_loader_thread.h"
#include "tree_loader_thread.h"

void GraphView::generateDefaultNodes()
{
    tax_map.clear();
    tax_map.insertName(1, "root");
    tax_map.insertName(131567, "cellular organisms");
    tax_map.insertName(2, "Bacteria");
    tax_map.insertName(201174, "Actinobacteria <phylum>");
    tax_map.insertName(2157, "Archaea");
    tax_map.insertName(2759, "Eukaryota");
    tax_map.insertName(10239, "Viruses");
    tax_map.insertName(39759, "Deltavirus");
    tax_map.insertName(12884, "Viroids");
    tax_map.insertName(185752, "Avsunviroidae");
    tax_map.insertName(12908, "unclassified sequences");
    tax_map.insertName(28384, "other sequences");

    // To improve the startup speed, first create hardcoded default nodes, they will be updated then the whole tree will be loaded
    root = new TaxNode(1);
    TaxNode *n131567 = root->addChild(131567);
    TaxNode *n2 = n131567->addChild(2);
    n2->addChild(201174);
    n131567->addChild(2157);
    n131567->addChild(2759);
    n131567->setCollapsed(false);
    TaxNode *n10239 = root->addChild(10239);
    n10239->addChild(39759);
    TaxNode *n12884 = root->addChild(12884);
    n12884->addChild(185752);
    root->addChild(12908);
    root->addChild(28384);
    root->setCollapsed(false);
}

void GraphView::markAllNodesDirty()
{
    class DirtyMarker : public GraphNodeVisitor
    {
        GraphView *graph_view;
    public:
        DirtyMarker(GraphView *gv) : GraphNodeVisitor(false), graph_view(gv){}
        virtual void makeAction(GraphNode *node, bool)
        {
            node->markDirty(DIRTY_ALL, &graph_view->dirtyList);
        }
    };

    DirtyMarker dirtyMarker(this);
    dirtyMarker.visitRootLeave(root->gnode);
}

GraphView::GraphView(QWidget *parent)
    : QGraphicsView(parent)
{

    MapLoaderThread *mlThread = new MapLoaderThread(this, true, this, &tax_map);
    connect(mlThread, SIGNAL(progress()), this, SLOT(updateLoadedNames()));
    connect(mlThread, SIGNAL(resultReady()), this, SLOT(mapIsLoaded()));
    connect(mlThread, SIGNAL(finished()), mlThread, SLOT(deleteLater()));
    mlThread->start();

    tlThread = new TreeLoaderThread(this, true);
    connect(tlThread, SIGNAL(resultReady(TaxNode *)), this, SLOT(treeIsLoaded(TaxNode *)));
    tlThread->start();

    QGraphicsScene *scene = new QGraphicsScene(this);
    scene->setItemIndexMethod(QGraphicsScene::NoIndex);
    setScene(scene);
    scene->setSceneRect(-30, 0, size().width(), 200);
    setCacheMode(CacheBackground);
    setViewportUpdateMode(BoundingRectViewportUpdate);
    setRenderHint(QPainter::Antialiasing);
    setTransformationAnchor(AnchorUnderMouse);
    setMinimumSize(200, 300);
    setWindowTitle(tr("Tree view"));
    setDragMode(QGraphicsView::ScrollHandDrag);
    hor_interval = 300;
    set_vert_interval(30);
    generateDefaultNodes();
    reset();
    markAllNodesDirty();
}

//=========================================================================
void GraphView::resizeEvent(QResizeEvent *e)
{
    QRectF r = sceneRect();
    r.setWidth(e->size().width()-20);
    setSceneRect(r);
    scene()->setSceneRect(r);
    reset();
}

//=========================================================================
void GraphView::adjust_scene_boundaries()
{
    QRectF rect = sceneRect();
    rect.setHeight(scene()->itemsBoundingRect().height());
    setSceneRect(rect);
}

//=========================================================================
void GraphView::CreateGraphNode(TaxNode *node)
{
    node->gnode = new GraphNode(this, node);
    if ( node->parent != NULL )
        new Edge(node->parent->gnode, node->gnode);
}

//=========================================================================
void GraphView::AddNodeToScene(TaxNode *node)
{
    if ( node->gnode == NULL )
        CreateGraphNode(node);
    scene()->addItem(node->gnode);
    foreach(Edge *e, node->gnode->edges())
        scene()->addItem(e);
    if ( node->isCollapsed() )
        return;
    if ( node->children.size() > 100 )
        qDebug() << "Very big node";
    for (TaxNodeIterator it=node->children.begin(); it!=node->children.end(); it++ )
        AddNodeToScene(*it);
}

//=========================================================================
void GraphView::adjustAllEdges()
{
    class EdgesAdjustor: public GraphNodeVisitor
    {
        public:
        EdgesAdjustor():GraphNodeVisitor(false){}
        virtual void makeAction(GraphNode *node, bool)
        {
            foreach(Edge *edge, node->edges())
                edge->adjust();
        }
    };
    EdgesAdjustor edgesAdjustor;
    edgesAdjustor.visitRootLeave(root->gnode);
}

//=========================================================================
void GraphView::reset()
{
    // Do NOT call clene->clean(). It will destroy all QGraphicItems in scene
    foreach(QGraphicsItem *i, scene()->items())
      scene()->removeItem(i);
    AddNodeToScene(root);
    if ( !root->isCollapsed() )
        resetNodesCoordinates();
    adjustAllEdges();
}

#ifndef QT_NO_WHEELEVENT

//=========================================================================
void GraphView::wheelEvent(QWheelEvent *event)
{
    if ( (QApplication::keyboardModifiers() & Qt::ControlModifier) == Qt::ControlModifier )
    {
        if ( event->delta()>0 )
        {
            if ( get_vert_interval() >= 20 )
                shrink_vertically();
        }
        else
        {
            if ( get_vert_interval() <= 100 )
                expand_vertically();
        }
    }
    else
    {
        QGraphicsView::wheelEvent(event);
    }
}
#endif

//=========================================================================
void GraphView::shrink_vertically(int s)
{
    qreal old_vert_int = vert_interval;
    vert_interval-=s;
    updateYCoord(((qreal) vert_interval)/old_vert_int);
}

//=========================================================================
void GraphView::expand_vertically(int s)
{
    qreal old_vert_int = vert_interval;
    vert_interval+=s;
    updateYCoord(((qreal) vert_interval)/old_vert_int);
}

//=========================================================================
void GraphView::updateYCoord(qreal factor)
{
    class NodeYChanger: public GraphNodeVisitor
    {
        public:
        qreal factor;
        NodeYChanger(qreal _factor):GraphNodeVisitor(false), factor(_factor){}
        virtual void makeAction(GraphNode *node, bool)
        {
            node->setY(factor*node->y());
        }
    };
    NodeYChanger nodeYChanger(factor);
    nodeYChanger.visitRootLeave(root->gnode);
    adjustAllEdges();
    adjust_scene_boundaries();
}

//=========================================================================
void GraphView::resetNodesCoordinates()
{
    class NodeYSetter: public GraphNodeVisitor
    {
    public:
        int maxLevel;
        int y;
        quint64 max_node_y;
        NodeYSetter(int inity):GraphNodeVisitor(false), maxLevel(0), y(inity), max_node_y(0) {}
        virtual void makeAction(GraphNode *node, bool visit_children)
        {
            if ( !visit_children )
            {
                if ( node->tax_node->level > maxLevel )
                    maxLevel = node->tax_node->level;
                y = max_node_y;
                max_node_y += node->view->get_vert_interval();
            }
            node->setY(y);  // x-coordinate MUST be set later
        }
        virtual void beforeVisitChildren(GraphNode *node)
        {
            max_node_y += node->view->get_vert_interval()/4;
        }
        virtual void afterVisitChildren(GraphNode *node)
        {
            quint64 sum_y = 0;
            QList<TaxNode *> &list = node->tax_node->children;
            for ( TaxNodeIterator it = list.begin(); it < list.end(); it++ )
                sum_y += (*it)->gnode->y();
            y = sum_y / list.size();
            max_node_y += node->view->get_vert_interval()/4;
        }
    };

    NodeYSetter y_setter(0);
    y_setter.visitLeaveRoot(root->gnode);
    int maxLevel = y_setter.maxLevel;
    int w = this->sceneRect().width()-200;
    int dx = maxLevel == 0 ? 0 : w/maxLevel;
    class NodeXSetter : public GraphNodeVisitor
    {
        int dx;
        int max_x;
    public:
        NodeXSetter(int _dx, int _max_x): GraphNodeVisitor(false), dx(_dx), max_x(_max_x){}
        virtual void makeAction(GraphNode *node, bool visit_children)
        {
            int x = visit_children ? node->tax_node->level*dx : max_x;
            node->setX(x);
        }
    };
    NodeXSetter x_setter(dx, w);
    x_setter.visitRootLeave(root->gnode);
    QRectF sr = sceneRect();
    sr.setHeight(y_setter.max_node_y+40);
    setSceneRect(sr);
}
//=========================================================================
void GraphView::updateDirtyNodes(quint32 flag)
{
    foreach(GraphNode *n, dirtyList)
    {
        if ( (n->dirty & flag) == 0 )
            continue;
        n->update();
        n->dirty &= ~flag;
        if ( n->dirty == 0 )
            dirtyList.removeAll(n);
    }
}

//=========================================================================
void GraphView::mapIsLoaded()
{
    qDebug() << "Map Loading is finished";
    // Handle end of mapLoading
    updateDirtyNodes(DIRTY_NAME);
    update();
}

//=========================================================================
void GraphView::updateLoadedNames()
{
    updateDirtyNodes(DIRTY_NAME);
}

//=========================================================================
void GraphView::treeIsLoaded(TaxNode *tree)
{
    qDebug() << "Tree Loading is finished";
    root->mergeWith(tree, this);
    tlThread->deleteLater();
    tlThread = NULL;
    reset();
    updateDirtyNodes(DIRTY_CHILD);
    update();
}
