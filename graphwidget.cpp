#include <QApplication>
#include <QDebug>
#include <QKeyEvent>
#include <QMessageBox>
#include <QScrollBar>

#include "graphwidget.h"
#include "edge.h"
#include "graph_node.h"
#include "map_loader_thread.h"
#include "tree_loader_thread.h"
#include "blast_data.h"

#define RIGHT_NODE_MARGIN   200

//=========================================================================
void GraphView::markAllNodesDirty()
{
    class DirtyMarker : public TaxNodeVisitor
    {
    public:
        DirtyMarker(GraphView *gv) : TaxNodeVisitor(LeavesToRoot, false, gv){}
        virtual void Action(BaseTaxNode *node)
        {
            node->getGnode()->markDirty(DIRTY_ALL, &graphView->dirtyList);
        }
    };

    DirtyMarker dirtyMarker(this);
    dirtyMarker.Visit(root);
}

//=========================================================================
GraphView::GraphView(QWidget *parent, TaxNode *taxTree)
    : QGraphicsView(parent),
      root(taxTree),
      create_nodes(true)
{
    QGraphicsScene *scene = new QGraphicsScene(this);
    scene->setItemIndexMethod(QGraphicsScene::NoIndex);
    setScene(scene);
    scene->setSceneRect(-30, 0, size().width(), size().height());
    setCacheMode(CacheBackground);
    setViewportUpdateMode(BoundingRectViewportUpdate);
    setRenderHint(QPainter::Antialiasing);
    setTransformationAnchor(AnchorUnderMouse);
    setMinimumSize(200, 300);
    setWindowTitle(tr("Tree view"));
    //setDragMode(QGraphicsView::ScrollHandDrag);
    hor_interval = 300;
    set_vert_interval(30);
    reset();
    markAllNodesDirty();
}

//=========================================================================
void GraphView::updateXCoord(qreal factor)
{
    class NodeXChanger: public TaxNodeVisitor
    {
        public:
        qreal factor;
        NodeXChanger(qreal _factor):TaxNodeVisitor(RootToLeaves, false, NULL, false, false), factor(_factor){}
        virtual void Action(BaseTaxNode *node)
        {
            node->getGnode()->setX(factor*node->getGnode()->x());
        }
    };
    NodeXChanger nodeXChanger(factor);
    nodeXChanger.Visit(root);
    adjustAllEdges();
    adjust_scene_boundaries();
}

//=========================================================================
void GraphView::resizeEvent(QResizeEvent *e)
{
    QRectF r = sceneRect();
    qreal oldW = r.width() - RIGHT_NODE_MARGIN;
    r.setWidth(e->size().width()-20);
    setSceneRect(r);
    scene()->setSceneRect(r);
    qreal newW = sceneRect().width() - RIGHT_NODE_MARGIN;
    updateXCoord(newW/oldW);
}

//=========================================================================
void GraphView::adjust_scene_boundaries()
{
    QRectF rect = sceneRect();
    rect.setHeight(scene()->itemsBoundingRect().height()+30);
    setSceneRect(rect);
}

//=========================================================================
void GraphView::CreateGraphNode(BaseTaxNode *node)
{
    node->createGnode(this);
}

//=========================================================================
void GraphView::AddNodeToScene(BaseTaxNode *node)
{
    if ( node->getGnode() == NULL )
    {
        if ( create_nodes )
            CreateGraphNode(node);
        else
            return;
    }
    GraphNode *gnode = node->getGnode();
    scene()->addItem(gnode);
    Edge *e = gnode->edge;
    if ( e == NULL && node->parent != NULL )
        e = new Edge(node->parent->getGnode(), gnode);
    if ( e != NULL )
        scene()->addItem(e);
    if ( node->isCollapsed() )
        return;
    ThreadSafeListLocker<BaseTaxNode *> locker(&node->children);
    for (TaxNodeIterator it=node->children.begin(); it!=node->children.end(); it++ )
        AddNodeToScene(*it);
}

//=========================================================================
void GraphView::adjustAllEdges()
{
    class EdgesAdjustor: public TaxNodeVisitor
    {
        public:
        EdgesAdjustor():TaxNodeVisitor(RootToLeaves, false, NULL, false, false ){}
        virtual void Action(BaseTaxNode *node)
        {
            Edge *e = node->getGnode()->edge;
            if ( e != NULL )
                e->adjust();
        }
    };
    EdgesAdjustor edgesAdjustor;
    edgesAdjustor.Visit(root);
}

//=========================================================================
void GraphView::reset()
{
    dirtyList.clear();
    scene()->clear();
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
        QPoint p = event->pos();
        qreal oldY = mapToScene(p).y();
        qreal oldH = sceneRect().height();
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
        qreal newH = sceneRect().height();
        qreal newY = oldY*newH/oldH;
        verticalScrollBar()->setValue(verticalScrollBar()->value()+newY-oldY);
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
    class NodeYChanger: public TaxNodeVisitor
    {
        public:
        qreal factor;
        NodeYChanger(qreal _factor):TaxNodeVisitor(RootToLeaves, false, NULL, false, false), factor(_factor){}
        virtual void Action(BaseTaxNode *node)
        {
            node->getGnode()->setY(factor*node->getGnode()->y());
        }
    };
    NodeYChanger nodeYChanger(factor);
    nodeYChanger.Visit(root);
    adjustAllEdges();
    adjust_scene_boundaries();
}

//=========================================================================
void GraphView::resetNodesCoordinates()
{
    class NodeYSetter: public TaxNodeVisitor
    {
    public:
        int maxLevel;
        int y;
        quint64 max_node_y;
        NodeYSetter(int inity, GraphView *gv):TaxNodeVisitor(LeavesToRoot, false, gv, false, false), maxLevel(0), y(inity), max_node_y(0) {}
        virtual void Action(BaseTaxNode *node)
        {            
            if ( !shouldVisitChildren(node) )
            {
                if ( node->getLevel() > maxLevel )
                    maxLevel = node->getLevel();
                y = max_node_y;
                max_node_y += graphView->get_vert_interval();
            }
            node->getGnode()->setY(y);
        }
        virtual void beforeVisitChildren(BaseTaxNode *node)
        {
            max_node_y += node->getGnode()->view->get_vert_interval()/4;
        }
        virtual void afterVisitChildren(BaseTaxNode *node)
        {
            quint64 sum_y = 0;
            QList<BaseTaxNode *> &list = node->children;
            for ( TaxNodeIterator it = list.begin(); it < list.end(); it++ )
                sum_y += (*it)->getGnode() == NULL ? 0 : (*it)->getGnode()->y();
            y = sum_y / list.size();
            max_node_y += graphView->get_vert_interval()/4;
        }
    };

    NodeYSetter y_setter(0, this);
    y_setter.Visit(root);
    int maxLevel = y_setter.maxLevel;
    int w = this->sceneRect().width()-RIGHT_NODE_MARGIN;
    int dx = maxLevel == 0 ? 0 : w/maxLevel;
    class NodeXSetter : public TaxNodeVisitor
    {
        int dx;
        int max_x;
    public:
        NodeXSetter(int _dx, int _max_x): TaxNodeVisitor(RootToLeaves, false, NULL, false, false), dx(_dx), max_x(_max_x){}
        virtual void Action(BaseTaxNode *node)
        {
            int x = max_x;
            if ( node->getLevel() == 0 )
                x = 0;
            else if ( shouldVisitChildren(node) )
                x = node->getLevel()*dx;
            node->getGnode()->setX(x);
        }
    };
    NodeXSetter x_setter(dx, w);
    x_setter.Visit(root);
//    QRectF sr = sceneRect();
//    sr.setHeight(y_setter.max_node_y+40);
    adjustAllEdges();
    adjust_scene_boundaries();
//    setSceneRect(sr);
}
//=========================================================================
void GraphView::updateDirtyNodes(quint32 flag)
{
    for (DirtyGNodesList::iterator it = dirtyList.begin(); it < dirtyList.end(); it++ )
    //foreach(GraphNode *n, dirtyList)
    {
        GraphNode *n = *it;
        if ( (n->dirty & flag) == 0 )
            continue;
        n->update();
        n->clearDirtyFlag(flag);
        if ( n->dirty == 0 )
            dirtyList.Delete(n);
    }
}

//=========================================================================
void GraphView::createMissedGraphNodes()
{
    class GNodesCreator : public TaxNodeVisitor
    {
    public:
        int nodesCreated;
        GNodesCreator(GraphView *gv) : TaxNodeVisitor(RootToLeaves, false, gv), nodesCreated(0){}
        virtual void Action(BaseTaxNode *node)
        {
            if ( node->getGnode() == NULL )
            {
                graphView->CreateGraphNode(node);
                graphView->AddNodeToScene(node);
                nodesCreated++;
            }
        }
    };
    GNodesCreator gnCreator(this);
    gnCreator.Visit(root);
    if ( gnCreator.nodesCreated > 0 )        
        resetNodesCoordinates();
}

//=========================================================================
void GraphView::blastLoadingProgress(void *obj)
{
    static bool updating = false;
    if ( updating )
        return;
    updating = true;
    if ( root != obj )
        root = (BlastTaxNode *)obj;
    createMissedGraphNodes();
    //reset();
    updateDirtyNodes(DIRTY_NAME);
    updating = false;
}

//=========================================================================
void GraphView::blastIsLoaded(void *obj)
{
    if ( root != obj )
        root = (BlastTaxNode *)obj;
    createMissedGraphNodes();
    //reset();
    updateDirtyNodes(DIRTY_NAME);
}

//=========================================================================
void DirtyGNodesList::Add(GraphNode *node)
{
    if ( contains(node) )
        return;
    ThreadSafeList::Add(node);
}
