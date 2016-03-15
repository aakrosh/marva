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
      create_nodes(true),
      curNode(taxTree)
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
void GraphView::expandPathTo(BaseTaxNode *node)
{
    if ( node->parent == NULL )
        return;
    QList<BaseTaxNode *> path;
    BaseTaxNode *n = node;
    while ( n->parent != NULL )
    {
        path.append(n->parent);
        n = n->parent;
    }
    foreach( BaseTaxNode *pn, path )
    {
        if ( pn->isCollapsed() )
            pn->setCollapsed(false, true);
    }
}

//=========================================================================
void GraphView::goUp()
{
    if ( curNode->parent == NULL )
        return;
    QPointF p = curNode->getGnode()->pos();
    qreal y = p.y() + 1 - get_vert_interval();
    QGraphicsItem *gi = NULL;
    qreal h = curNode->getGnode()->boundingRect().height();
    do
    {
        p.setY(y);
        gi = scene()->itemAt(p, transform());
        y -= h;
    }
    while ( ( gi == NULL || gi->type() != GraphNode::Type ) && y > 0 );
    if ( gi != NULL && gi->type() == GraphNode::Type )
    {
        setCurrentNode(((GraphNode *)gi)->tax_node);
        return;
    }

    int ind = curNode->parent->children.indexOf(curNode)-1;
    if ( ind < 0 )
    {
        //goLeft();
        return;
    }
    setCurrentNode(curNode->parent->children.at(ind));
}

//=========================================================================
void GraphView::goDown()
{
    if ( curNode->parent == NULL )
        return;
    QPointF p = curNode->getGnode()->pos();
    qreal y = p.y() - 1 + get_vert_interval();
    QGraphicsItem *gi = NULL;
    qreal h = curNode->getGnode()->boundingRect().height();
    do
    {
        p.setY(y);
        gi = scene()->itemAt(p, transform());
        y += h;
    }
    while ( ( gi == NULL || gi->type() != GraphNode::Type ) && y < scene()->itemsBoundingRect().height() );
    if ( gi != NULL && gi->type() == GraphNode::Type )
    {
        setCurrentNode(((GraphNode *)gi)->tax_node);
        return;
    }
    int ind = curNode->parent->children.indexOf(curNode)+1;
    if ( ind >= curNode->parent->children.count() )
    {
        return;
    }
    setCurrentNode(curNode->parent->children.at(ind));
}

//=========================================================================
void GraphView::goLeft()
{
    if ( curNode->parent == NULL )
        return;
    setCurrentNode(curNode->parent);
}

//=========================================================================
void GraphView::goRight()
{
    if ( curNode->children.empty() )
        return;
    int ind = curNode->children.size()/2;
    setCurrentNode(curNode->children.at(ind));
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
void GraphView::keyPressEvent(QKeyEvent *event)
{
    switch( event->key() )
    {
        case Qt::Key_Right:
            goRight();
            return;
        case Qt::Key_Left:
            goLeft();
            return;
        case Qt::Key_Up:
            goUp();
            return;
        case Qt::Key_Down:
            goDown();
            return;
    }
    QGraphicsView::keyPressEvent(event);
}

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
    adjustAllEdges();
    adjust_scene_boundaries();
}
//=========================================================================
void GraphView::updateDirtyNodes(quint32 flag)
{
    for (DirtyGNodesList::iterator it = dirtyList.begin(); it < dirtyList.end(); it++ )
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
void GraphView::setCurrentNode(BaseTaxNode *node)
{
    if ( curNode == node )
        return;
    onCurrentNodeChanged(node);
    emit currentNodeChanged(node);
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
    updateDirtyNodes(DIRTY_NAME);
}

//=========================================================================
void GraphView::onCurrentNodeChanged(BaseTaxNode *node)
{
    if ( curNode == node )
        return;
    expandPathTo(node);
    BaseTaxNode *oldNode = curNode;
    curNode = node;
    GraphNode *oldGNode = oldNode->getGnode();
    if ( oldGNode != NULL )
        oldGNode->update();
    GraphNode *gnode = curNode->getGnode();
    if ( gnode == NULL )
        return;
    gnode->update();
    ensureVisible(gnode);
}

//=========================================================================
void DirtyGNodesList::Add(GraphNode *node)
{
    if ( contains(node) )
        return;
    ThreadSafeList::Add(node);
}
