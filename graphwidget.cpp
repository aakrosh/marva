#include <QApplication>
#include <QDebug>
#include <QKeyEvent>
#include <QMessageBox>
#include <QScrollBar>
#include <QMenu>
#include <QAction>

#include "graphwidget.h"
#include "edge.h"
#include "graph_node.h"
#include "map_loader_thread.h"
#include "tree_loader_thread.h"
#include "blast_data.h"

#define RIGHT_NODE_MARGIN   200
#define MIN_DX 70

//=========================================================================
void GraphView::markAllNodesDirty()
{
    class DirtyMarker : public TaxNodeVisitor
    {
    public:
        DirtyMarker(GraphView *gv) : TaxNodeVisitor(LeavesToRoot, false, gv){}
        virtual void Action(BaseTaxNode *node)
        {
            GraphNode *gnode = node->getGnode();
            if ( gnode == NULL )
                return;
            gnode->markDirty(DIRTY_ALL, &graphView->dirtyList);
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
      curNode(taxTree),
      treeDepth(0)
{
    QGraphicsScene *scene = new QGraphicsScene(this);
    scene->setItemIndexMethod(QGraphicsScene::NoIndex);
    setScene(scene);
    scene->setSceneRect(-30, 0, size().width(), size().height());
    setCacheMode(CacheBackground);
    setViewportUpdateMode(BoundingRectViewportUpdate);
    setRenderHint(QPainter::Antialiasing);
    setTransformationAnchor(AnchorUnderMouse);
    setMinimumSize(300, 300);
    setWindowTitle(tr("Tree view"));
    hor_interval = 300;
    set_vert_interval(30);
    reset();
    markAllNodesDirty();
    nodePopupMenu = new QMenu();
    hideNodeAction = new QAction("Hide node", this);
    nodePopupMenu->addAction(hideNodeAction);
    connect(hideNodeAction, SIGNAL(triggered()), this, SLOT(hideCurrent()));
}

//=========================================================================
void GraphView::updateXCoord()
{
    quint32 max_w = this->sceneRect().width()-RIGHT_NODE_MARGIN;
    qreal dx = max_w/treeDepth;
    if ( dx < MIN_DX )
        dx = MIN_DX;
    class NodeXChanger: public TaxNodeVisitor
    {
        public:
        qreal dx;
        qreal max_w;
        NodeXChanger(qreal _dx, qreal _max_w):TaxNodeVisitor(RootToLeaves, false, NULL, false, false), dx(_dx), max_w(_max_w) {}
        virtual void Action(BaseTaxNode *node)
        {
            if ( shouldVisitChildren(node) )
                node->getGnode()->setX(node->getLevel()*dx);
            else
                node->getGnode()->setX(max_w);
        }
    };
    NodeXChanger nodeXChanger(dx, max_w);
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
    int max_count = 10;
    do
    {
        if ( max_count-- <=0 )
            break;
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
        return;
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
    int max_count = 10;
    do
    {
        if ( max_count-- <=0 )
            break;
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
        return;
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
    qreal newW = e->size().width()-20;
    quint32 minW = MIN_DX*treeDepth+RIGHT_NODE_MARGIN;
    if ( newW < minW )
        newW = minW;
    r.setWidth(newW);
    setSceneRect(r);
    updateXCoord();
}

//=========================================================================
void GraphView::adjust_scene_boundaries()
{
    QRectF rect = sceneRect();
    QRectF ibr = scene()->itemsBoundingRect();
    rect.setHeight(ibr.height()+30);
    qreal sizew = size().width()-20;
    rect.setWidth(ibr.width() > sizew ? ibr.width() : sizew);
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
    if ( !node->visible )
        return;
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
        if ( event->delta() > 0 )
            shrink_vertically();
        else
            expand_vertically();
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
        case Qt::Key_Enter:
        case Qt::Key_Return:
            curNode->setCollapsed(!curNode->isCollapsed(), true);
            return;
        case Qt::Key_Minus:
            if ( (event->modifiers() & Qt::ControlModifier) == Qt::ControlModifier )
                shrink_vertically();
            else
                curNode->setCollapsed(true, true);
            return;
        case Qt::Key_Plus:
            if ( (event->modifiers() & Qt::ControlModifier) == Qt::ControlModifier )
                expand_vertically();
            else
                curNode->setCollapsed(false, true);
            return;
    }
    QGraphicsView::keyPressEvent(event);
}

//=========================================================================
void GraphView::shrink_vertically(int s)
{
    if ( get_vert_interval() <20 )
        return;
    qreal old_vert_int = vert_interval;
    vert_interval-=s;
    updateYCoord(((qreal) vert_interval)/old_vert_int);
}

//=========================================================================
void GraphView::expand_vertically(int s)
{
    if ( get_vert_interval() > 100 )
        return;
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
            int n = 0;
            for ( TaxNodeIterator it = list.begin(); it < list.end(); it++ )
            {
                GraphNode *gnode = (*it)->getGnode();
                if ( gnode != NULL )
                {
                    sum_y += gnode->y();
                    n++;
                }
            }
            if ( n > 0 )
                y = sum_y / n;
            max_node_y += graphView->get_vert_interval()/4;
        }
    };

    NodeYSetter y_setter(0, this);
    y_setter.Visit(root);
    treeDepth = y_setter.maxLevel;
    int w = size().width()-RIGHT_NODE_MARGIN;
    int dx = treeDepth == 0 ? 0 : w/treeDepth;
    if ( dx < MIN_DX )
        dx = MIN_DX;
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
    NodeXSetter x_setter(dx, dx*treeDepth);
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
            if ( node->getGnode() == NULL && node->visible )
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
void GraphView::hideNode(BaseTaxNode *node)
{
    if ( node == NULL )
        return;
    node->visible = false;
    GraphNode *gnode = node->getGnode();
    if ( gnode == NULL || !scene()->items().contains(gnode) )
        return;
    class NodeRemover : public TaxNodeVisitor
    {
    public:
        NodeRemover(GraphView *gv):TaxNodeVisitor(RootToLeaves, false, gv, false, false){}
        virtual void Action(BaseTaxNode *node)
        {
            GraphNode *gnode = node->getGnode();
            if ( gnode != NULL )
            {
                graphView->dirtyList.Delete(gnode);
                delete gnode;
            }
        }
    };
    NodeRemover nr(this);
    nr.Visit(node);
    resetNodesCoordinates();
}

//=========================================================================
void GraphView::showNode(BaseTaxNode *node)
{
    GraphNode *gnode = node->getGnode();
    if ( gnode == NULL || !scene()->items().contains(gnode) )
    {
        if ( node->parent != NULL )
        {
            GraphNode *pgnode = node->parent->getGnode();
            if ( pgnode == NULL || !scene()->items().contains(pgnode) )
                return;
        }
        AddNodeToScene(node);
        resetNodesCoordinates();
    }
}

//=========================================================================
void GraphView::hideCurrent()
{
    hideNode(curNode);
}

//=========================================================================
void GraphView::onNodeVisibilityChanged(BaseTaxNode *node, bool node_visible)
{
    if ( node_visible )
        showNode(node);
    else
        hideNode(node);
}

//=========================================================================
void DirtyGNodesList::Add(GraphNode *node)
{
    if ( contains(node) )
        return;
    ThreadSafeList::Add(node);
}
