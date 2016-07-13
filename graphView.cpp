#include <QApplication>
#include <QDebug>
#include <QKeyEvent>
#include <QScrollBar>
#include <QMenu>
#include <QAction>
#include <QJsonObject>
#include <QJsonArray>
#include <QMessageBox>

#include "graphview.h"
#include "edge.h"
#include "graph_node.h"
#include "map_loader_thread.h"
#include "tree_loader_thread.h"
#include "blast_data.h"
#include "taxnodesignalsender.h"
#include "taxdataprovider.h"
#include "taxnodesignalsender.h"
#include "colors.h"

#define RIGHT_NODE_MARGIN   150
#define MIN_DX 70

//=========================================================================
void TreeGraphView::markAllNodesDirty()
{
    class DirtyMarker : public TaxNodeVisitor
    {
    public:
        DirtyMarker(TreeGraphView *gv) : TaxNodeVisitor(LeavesToRoot, false, gv){}
        virtual void Action(TreeTaxNode *node)
        {
            TaxTreeGraphNode *gnode = node->getTaxTreeGNode();
            if ( gnode == NULL )
                return;
            gnode->markDirty(DIRTY_ALL, &graphView->dirtyList);
        }
    };

    DirtyMarker dirtyMarker(this);
    dirtyMarker.Visit(root);
}

//=========================================================================
TreeGraphView::TreeGraphView(QWidget *parent, TaxNode *taxTree)
    : DataGraphicsView(NULL, parent),
      root(taxTree),
      hiddenNodes(0),
      create_nodes(true),
      treeDepth(0)
{
//    curNode = NULL; // Do not change it. It must be NULL at the beginning
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
    setCurrentNode(taxTree);
    if ( root != NULL )
        centerOn(root->getGnode());
    nodePopupMenu = new QMenu();
    hideNodeAction = new QAction("Hide node", this);
    hideAllButNodeAction = new QAction("Hide all but this", this);
    showAllNodesAction = new QAction("Show all", this);
    colorAction = new QAction("Color...", this);
    nodePopupMenu->addAction(hideNodeAction);
    nodePopupMenu->addAction(hideAllButNodeAction);
    nodePopupMenu->addAction(showAllNodesAction);
    nodePopupMenu->addAction(colorAction);
    connect(hideNodeAction, SIGNAL(triggered()), this, SLOT(hideCurrent()));
    connect(hideAllButNodeAction, SIGNAL(triggered()), this, SLOT(hideAllButCurrent()));
    connect(showAllNodesAction, SIGNAL(triggered()), this, SLOT(showAllNodes()));
    connect(colorAction, SIGNAL(triggered()), this, SLOT(changeCurrentTaxColor()));

    connect(this, SIGNAL(allNodesShown()), this, SLOT(onNodeVisiblityChanged()));
    setAttribute(Qt::WA_DeleteOnClose);
}

//=========================================================================
TreeGraphView::~TreeGraphView()
{
    if ( taxDataProvider->type == BLAST_DATA_PROVIDER ) // Shows blast content
    {

    }
}

//=========================================================================
void TreeGraphView::updateXCoord()
{
    if ( treeDepth == 0 )
        return;
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
        virtual void Action(TreeTaxNode *node)
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
void TreeGraphView::expandPathTo(TreeTaxNode *node)
{
    if ( node->parent == NULL )
        return;
    if ( node->getGnode() != NULL )
        return;
    QList<TreeTaxNode *> path;
    TreeTaxNode *n = node;
    while ( n->parent != NULL )
    {
        path.append(n->parent);
        n = n->parent;
    }
    foreach( TreeTaxNode *pn, path )
    {
        if ( pn->isCollapsed() )
            pn->setCollapsed(false, true);
    }
}

//=========================================================================
void TreeGraphView::goUp()
{
    TreeTaxNode *curNode = getCurNode();
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
    while ( ( gi == NULL || gi->type() != TaxTreeGraphNode::Type ) && y > 0 );
    if ( gi != NULL && gi->type() == TaxTreeGraphNode::Type )
    {
        setCurrentNode(((TaxTreeGraphNode *)gi)->tax_node);
        return;
    }

    int ind = getCurNode()->parent->children.indexOf(getCurNode())-1;
    if ( ind < 0 )
        return;
    setCurrentNode(getCurNode()->parent->children.at(ind));
}

//=========================================================================
void TreeGraphView::goDown()
{
    TreeTaxNode *curNode = getCurNode();
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
    while ( ( gi == NULL || gi->type() != TaxTreeGraphNode::Type ) && y < scene()->itemsBoundingRect().height() );
    if ( gi != NULL && gi->type() == TaxTreeGraphNode::Type )
    {
        setCurrentNode(((TaxTreeGraphNode *)gi)->tax_node);
        return;
    }
    int ind = getCurNode()->parent->children.indexOf(getCurNode())+1;
    if ( ind >= getCurNode()->parent->children.count() )
        return;
    setCurrentNode(getCurNode()->parent->children.at(ind));
}

//=========================================================================
void TreeGraphView::goLeft()
{
    if ( getCurNode()->parent == NULL )
        return;
    setCurrentNode(getCurNode()->parent);
}

//=========================================================================
void TreeGraphView::goRight()
{
    if ( getCurNode()->children.empty() )
        return;
    int ind = getCurNode()->children.size()/2;
    setCurrentNode(getCurNode()->children.at(ind));
}

//=========================================================================
void TreeGraphView::setNodeInvisible(BaseTaxNode *bnode)
{
    if  ( !bnode->is_visible )
        return;
    bnode->setVisible(false);
    hiddenNodes++;
    emit nodeHidden(bnode);
}

//=========================================================================
void TreeGraphView::hideNodes(quint32 oldT, quint32 newT)
{
    class NodeHider : public TaxNodeVisitor
    {
        quint32 oldT;
        quint32 newT;
        bool has_visible;
    public:
        NodeHider(TreeGraphView *gv, quint32 _oldT, quint32 _newT):
            TaxNodeVisitor(LeavesToRoot, false, gv, false, true),
            oldT(_oldT),
            newT(_newT)
        {

        }

        virtual void Action(TreeTaxNode *node)
        {
            BlastTaxNode *bnode = dynamic_cast<BlastTaxNode *>(node);
            if ( bnode->reads == 0 )
            {
                if ( !has_visible )
                    graphView->setNodeInvisible(bnode);
            }
            else if ( bnode->is_visible && bnode->reads < newT )
                graphView->setNodeInvisible(bnode);
        }
        virtual void afterVisitChildren(TreeTaxNode *node)
        {
            QList<TreeTaxNode *> &list = node->children;
            has_visible = false;
            for ( TaxNodeIterator it = list.begin(); it < list.end(); it++ )
            {
                if ( (*it)->visible() )
                {
                    has_visible = true;
                    break;
                }
            }
        }
    };
    NodeHider nh(this, oldT, newT); // Mark nodes as non-visible
    nh.Visit(root);

    class GraphNodeHider : public TaxNodeVisitor
    {
    public:
        GraphNodeHider(TreeGraphView *gv):TaxNodeVisitor(RootToLeaves, false, gv, false, false) {}
        virtual void Action(TreeTaxNode *node)
        {
            if ( !node->visible() )
                graphView->hideNode(node, false);
        }
    };
    GraphNodeHider gh(this); // remove invisible graph nodes
    gh.Visit(root);
    resetNodesCoordinates();
}

//=========================================================================
void TreeGraphView::unhideNodes(quint32 oldT, quint32 newT)
{
    class NodeUnHider : public TaxNodeVisitor
    {
        quint32 oldT;
        quint32 newT;
        bool has_visible;
    public:
        NodeUnHider(TreeGraphView *gv, quint32 _oldT, quint32 _newT):
            TaxNodeVisitor(LeavesToRoot, false, gv, false, true),
            oldT(_oldT),
            newT(_newT)
        {

        }
        virtual void Action(TreeTaxNode *node)
        {
            BlastTaxNode *bnode = dynamic_cast<BlastTaxNode *>(node);
            if ( bnode->reads == 0 )
            {
                if ( has_visible )
                    bnode->setVisible(true);
            }
            else if ( bnode->reads > newT && bnode->reads <= oldT )
               bnode->setVisible(true);
        }
        virtual void afterVisitChildren(TreeTaxNode *node)
        {
            QList<TreeTaxNode *> &list = node->children;
            has_visible = false;
            for ( TaxNodeIterator it = list.begin(); it < list.end(); it++ )
            {
                if ( (*it)->visible() )
                {
                    has_visible = true;
                    break;
                }
            }
        }
    };
    NodeUnHider nh(this, oldT, newT);
    nh.Visit(root);
    createMissedGraphNodes();
}

//=========================================================================
void TreeGraphView::resizeEvent(QResizeEvent *e)
{
    if ( e->oldSize().width() != e->size().width() )
    {
        QRectF r = sceneRect();
        qreal newW = e->size().width()-60;
        quint32 minW = MIN_DX*treeDepth+RIGHT_NODE_MARGIN;
        if ( newW < minW )
            newW = minW;
        r.setWidth(newW);
        setSceneRect(r);
        updateXCoord();
    }
    DataGraphicsView::resizeEvent(e);
}

//=========================================================================
void TreeGraphView::adjust_scene_boundaries()
{
    QRectF rect = sceneRect();
    QRectF ibr = scene()->itemsBoundingRect();
    rect.setHeight(ibr.height()+60);
    qreal sizew = size().width()-20;
    rect.setWidth(ibr.width() > sizew ? ibr.width() : sizew);
    setSceneRect(rect);
}

//=========================================================================
void TreeGraphView::CreateGraphNode(TreeTaxNode *node)
{
    node->createGnode(this);
}

//=========================================================================
void TreeGraphView::AddNodeToScene(TreeTaxNode *node)
{
    if ( node == NULL || !node->visible() )
        return;
    if ( node->getGnode() == NULL )
    {
        if ( create_nodes )
            CreateGraphNode(node);
        else
            return;
    }
    TaxTreeGraphNode *gnode = ((TaxTreeGraphNode *)node->getGnode());
    scene()->addItem(gnode);
    Edge *e = gnode->edge;
    if ( e == NULL && node->parent != NULL )
        e = new Edge(((TaxTreeGraphNode *)node->parent->getGnode()), gnode);
    if ( e != NULL )
        scene()->addItem(e);
    if ( node->isCollapsed() )
        return;
    ThreadSafeListLocker<TreeTaxNode *> locker(&node->children);
    for (TaxNodeIterator it=node->children.begin(); it!=node->children.end(); it++ )
        AddNodeToScene(*it);
}

//=========================================================================
void TreeGraphView::adjustAllEdges()
{
    class EdgesAdjustor: public TaxNodeVisitor
    {
        public:
        EdgesAdjustor():TaxNodeVisitor(RootToLeaves, false, NULL, false, false ){}
        virtual void Action(TreeTaxNode *node)
        {
            Edge *e = node->getTaxTreeGNode()->edge;
            if ( e != NULL )
                e->adjust();
        }
    };
    EdgesAdjustor edgesAdjustor;
    edgesAdjustor.Visit(root);
}

//=========================================================================
void TreeGraphView::reset()
{
    dirtyList.clear();
    scene()->clear();
    if ( root != NULL )
    {
        AddNodeToScene(root);
        if ( !root->isCollapsed() )
            resetNodesCoordinates();
    }
    adjustAllEdges();
}

//=========================================================================
void TreeGraphView::onTreeChanged()
{
    updateDirtyNodes(DIRTY_CHILD);
    createMissedGraphNodes();
}

//=========================================================================
void TreeGraphView::onNodeNamesChanged()
{
    updateDirtyNodes(DIRTY_NAME);
}

#ifndef QT_NO_WHEELEVENT

//=========================================================================
void TreeGraphView::wheelEvent(QWheelEvent *event)
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
void TreeGraphView::keyPressEvent(QKeyEvent *event)
{
    TreeTaxNode *curNode = getCurNode();
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
            {
                if ( curNode != NULL )
                    curNode->setCollapsed(!getCurNode()->isCollapsed(), true);
            }
            return;
        case Qt::Key_Minus:
            if ( (event->modifiers() & Qt::ControlModifier) == Qt::ControlModifier )
                shrink_vertically();
            else
            {
                if ( curNode != NULL )
                    curNode->setCollapsed(true, true);
            }
            return;
        case Qt::Key_Plus:
            if ( (event->modifiers() & Qt::ControlModifier) == Qt::ControlModifier )
                expand_vertically();
            else
                if ( curNode != NULL )
                    curNode->setCollapsed(false, true);
            return;
    }
    QGraphicsView::keyPressEvent(event);
}

//=========================================================================
void TreeGraphView::showContextMenu(const QPoint &p)
{
    QMenu *newMenu = new QMenu(this);
    newMenu->addActions(nodePopupMenu->actions());
    newMenu->addSeparator();
    newMenu->addActions(popupMenu.actions());
    newMenu->exec(mapToGlobal(p));
}

//=========================================================================
void TreeGraphView::shrink_vertically(int s)
{
    if ( get_vert_interval() <20 )
        return;
    qreal old_vert_int = vert_interval;
    vert_interval-=s;
    updateYCoord(((qreal) vert_interval)/old_vert_int);
}

//=========================================================================
void TreeGraphView::expand_vertically(int s)
{
    if ( get_vert_interval() > 100 )
        return;
    qreal old_vert_int = vert_interval;
    vert_interval+=s;
    updateYCoord(((qreal) vert_interval)/old_vert_int);
}

//=========================================================================
void TreeGraphView::updateYCoord(qreal factor)
{
    class NodeYChanger: public TaxNodeVisitor
    {
        public:
        qreal factor;
        NodeYChanger(qreal _factor):TaxNodeVisitor(RootToLeaves, false, NULL, false, false), factor(_factor){}
        virtual void Action(TreeTaxNode *node)
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
void TreeGraphView::resetNodesCoordinates()
{
    class NodeYSetter: public TaxNodeVisitor
    {
    public:
        int maxLevel;
        int y;
        quint64 max_node_y;
        NodeYSetter(int inity, TreeGraphView *gv):TaxNodeVisitor(LeavesToRoot, false, gv, false, false), maxLevel(0), y(inity), max_node_y(0) {}
        virtual void Action(TreeTaxNode *node)
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
        virtual void beforeVisitChildren(TreeTaxNode *)
        {
            max_node_y += graphView->get_vert_interval()/4;
        }
        virtual void afterVisitChildren(TreeTaxNode *node)
        {
            quint64 sum_y = 0;
            QList<TreeTaxNode *> &list = node->children;
            int n = 0;
            for ( TaxNodeIterator it = list.begin(); it < list.end(); it++ )
            {
                TaxTreeGraphNode *gnode = (*it)->getTaxTreeGNode();
                if ( gnode != NULL )
                {
                    sum_y += gnode->y();
                    n++;
                }
            }
            if ( n > 0 )
                y = sum_y / n;
            else
                y = max_node_y;
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
        virtual void Action(TreeTaxNode *node)
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
void TreeGraphView::updateDirtyNodes(quint32 flag)
{
//    for (DirtyGNodesList::iterator it = dirtyList.begin(); it < dirtyList.end(); it++ )
    for (DirtyGNodesList::iterator it = dirtyList.end()-1; it >= dirtyList.begin(); it-- )
    {
        TaxTreeGraphNode *n = *it;
        if ( n == NULL || (n->dirty & flag) == 0 )
            continue;
        n->update();
        n->clearDirtyFlag(flag);
        if ( n->dirty == 0 )
            dirtyList.Delete(n);
    }
}

//=========================================================================
void TreeGraphView::createMissedGraphNodes()
{
    class GNodesCreator : public TaxNodeVisitor
    {
    public:
        int nodesCreated;
        GNodesCreator(TreeGraphView *gv) : TaxNodeVisitor(RootToLeaves, false, gv, false, true, false), nodesCreated(0){}
        virtual void Action(TreeTaxNode *node)
        {
            if ( node->getGnode() == NULL && node->visible() )
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
void TreeGraphView::onCurrentNodeChanged(BaseTaxNode *node)
{
    TreeTaxNode *oldNode = getCurNode();
    if ( oldNode == node )
        return;
    TreeTaxNode *ttn = (TreeTaxNode *)node;
    expandPathTo(ttn);
    taxDataProvider->current_tax_id = node->getId();
    if ( oldNode != NULL)
    {
        TaxTreeGraphNode *oldGNode = oldNode->getTaxTreeGNode();
        if ( oldGNode != NULL )
            oldGNode->update();
    }
    TaxTreeGraphNode *gnode = ((TreeTaxNode *)node)->getTaxTreeGNode();
    if ( gnode == NULL )
        return;
    gnode->update();

    QRect portRect = viewport()->rect();
    QRectF sceneRect = mapToScene(portRect).boundingRect();
    QRectF sceneRectInItemCoord = gnode->mapFromScene(sceneRect).boundingRect();

    if ( !sceneRectInItemCoord.contains(gnode->boundingRect()) )
        ensureVisible(gnode);
}

//=========================================================================
void TreeGraphView::hideNode(TreeTaxNode *node, bool resetCoordinates)
{
    if ( node == NULL )
        return;
    TaxTreeGraphNode *gnode = node->getTaxTreeGNode();
    if ( gnode == NULL || !scene()->items().contains(gnode) )
        return;
    class NodeRemover : public TaxNodeVisitor
    {
    public:
        NodeRemover(TreeGraphView *gv):TaxNodeVisitor(RootToLeaves, false, gv, false, false){}
        virtual void Action(TreeTaxNode *node)
        {
            node->setVisible(false);
            TaxTreeGraphNode *gnode = node->getTaxTreeGNode();
            if ( gnode != NULL )
            {
                graphView->dirtyList.Delete(gnode);
                delete gnode;
            }
        }
    };
    NodeRemover nr(this);
    nr.Visit(node);
    if ( resetCoordinates )
        resetNodesCoordinates();
}

//=========================================================================
void TreeGraphView::hideNodeCheckParents(TreeTaxNode *node, bool resetCoordinates)
{
    hideNode(node, resetCoordinates);
    TreeTaxNode *p = node->parent;
    while ( p != NULL )
    {
        bool has_visible_ch = false;
        ChildrenList &list = p->children;
        for ( TaxNodeIterator it = list.begin(); it < list.end(); it++ )
        {
            if ( (*it)->visible() )
            {
                has_visible_ch = true;
                break;
            }
        }
        if ( has_visible_ch )
            break;
        else
            hideNode(p, resetCoordinates);
        p = p->parent;
    }
}

//=========================================================================
void TreeGraphView::showNode(TreeTaxNode *node, bool resetCoordinates)
{
    TaxTreeGraphNode *gnode = node->getTaxTreeGNode();
    if ( gnode == NULL || !scene()->items().contains(gnode) )
    {
        TreeTaxNode *pnode = node->parent;
        if ( pnode != NULL )
        {
            TaxTreeGraphNode *pgnode = pnode->getTaxTreeGNode();
            if ( pgnode == NULL  || !scene()->items().contains(pgnode) )
                pnode->setVisible(true, true);
        }
        if ( pnode == NULL || !pnode->isCollapsed() )
        {
            AddNodeToScene(node);
            if ( resetCoordinates )
                resetNodesCoordinates();
        }
    }
}

//=========================================================================
void TreeGraphView::hideCurrent()
{
    TreeTaxNode *curNode = getCurNode();
    NodePositionKeeper keeper(this, ((TreeTaxNode *)curNode)->parent);

    setNodeInvisible(curNode);
    hideNodeCheckParents(curNode);
}

//=========================================================================
void TreeGraphView::hideAllButCurrent()
{
    TreeTaxNode *curNode = getCurNode();
    NodePositionKeeper keeper(this, curNode);
    curNode->setVisible(true);
    TreeTaxNode *vNode = ((TreeTaxNode *)curNode);
    TreeTaxNode *p = vNode->parent;
    while ( p != NULL )
    {
        ChildrenList &list = p->children;
        for ( TaxNodeIterator it = list.begin(); it < list.end(); it++ )
            (*it)->setVisible((*it) == vNode);
        vNode = p;
        p = vNode->parent;
    }
    ensureVisible(curNode->getGnode());
}

//=========================================================================
void TreeGraphView::showAllNodes()
{
    class NodeShower : public TaxNodeVisitor
    {
    public:
        NodeShower(TreeGraphView *gv):TaxNodeVisitor(RootToLeaves, false, gv, true, true, true){}
        virtual void Action(TreeTaxNode *node)
        {
            if ( !node->visible() )
            {
                node->setVisible(true);
                TaxNodeSignalSender *tnss = getTaxNodeSignalSender(node);
                tnss->VisibilityChanged(true);
            }
        }
    };
    NodeShower nr(this);
    nr.Visit(root);
    createMissedGraphNodes();
    resetNodesCoordinates();
    hiddenNodes = 0;
    emit allNodesShown();
}

//=========================================================================
void TreeGraphView::changeCurrentTaxColor()
{
    colors->pickColor(taxDataProvider->current_tax_id);
}

//=========================================================================
void TreeGraphView::onNodeVisiblityChanged()
{
    showAllNodesAction->setVisible(hiddenNodes == 0);
}

//=========================================================================
void TreeGraphView::onNodeVisibilityChanged(BaseTaxNode *node, bool node_visible)
{
    NodePositionKeeper keeper(this, getCurNode());

    TreeTaxNode *ttn = (TreeTaxNode *)node;
    if ( node_visible )
        showNode(ttn, false);
    else
        hideNodeCheckParents(ttn, false);
    resetNodesCoordinates();
}


//=========================================================================
void DirtyGNodesList::Add(TaxTreeGraphNode *node)
{
    if ( contains(node) )
        return;
    ThreadSafeList::Add(node);
}

//=========================================================================
//*************************************************************************
//=========================================================================
BlastGraphView::BlastGraphView(BlastTaxDataProvider *blastTaxDataProvider, QWidget *parent, TaxNode *taxTree):
    TreeGraphView(parent, taxTree),
    reads_threshold(0)
{
    flags |= DGF_BUBBLES|DGF_READS;
    config = new BubbledGraphViewConfig();
    taxDataProvider = (TaxDataProvider*)blastTaxDataProvider;
    if ( blastTaxDataProvider != NULL )
    {
        blastTaxDataProvider->setParent(NULL);
        blastTaxDataProvider->addParent();
    }
    dirtyList.clear();
    scene()->clear();
    root = NULL;

}

//=========================================================================
BlastGraphView::~BlastGraphView()
{
    blastTaxDataProvider()->removeParent();
    emit blast_view_closed();
}

//=========================================================================
void BlastGraphView::blastLoadingProgress(void *obj)
{
    static bool updating = false;
    if ( updating || obj == NULL )
        return;
    updating = true;
    bool centeron = root == NULL;
    if ( root != obj )
        root = (BlastTaxNode *)obj;
    createMissedGraphNodes();
    if ( centeron )
        centerOn(root->getGnode());
    updateDirtyNodes(DIRTY_NAME);
    updating = false;
}

//=========================================================================
void BlastGraphView::blastIsLoaded(void *obj)
{
    if ( root != obj )
        root = (BlastTaxNode *)obj;
    if ( root == NULL )
        return;
    createMissedGraphNodes();
    updateDirtyNodes(DIRTY_NAME);
}

//=========================================================================
void BlastGraphView::onColorChanged(BaseTaxNode *n)
{
    BlastTaxNode *btn = blastTaxDataProvider()->nodeById(n->getId());
    if ( btn == NULL )
        return;
    GraphNode *gn = btn->getGnode();
    if ( gn == NULL )
        return;
    gn->update();
}

//=========================================================================
void BlastGraphView::toJson(QJsonObject &json) const
{
    json["Type"] = "BlastGraphView";
    json["Threshold"] = (int)reads_threshold;
    json["DataProvider"] = blastTaxDataProvider()->name;
}

//=========================================================================
void BlastGraphView::fromJson(QJsonObject &json)
{
    try
    {
        reads_threshold = json["Threshold"].toInt();
        QString dpName = json["DataProvider"].toString();
        BlastTaxDataProvider *p = blastTaxDataProviders.providerByName(dpName);
        if ( p == NULL )
        {
             close();
             return;
        }
        p->addParent();
        taxDataProvider = p;
        root = blastTaxDataProvider()->root;
        createMissedGraphNodes();
    }
    catch (...)
    {
        QMessageBox::warning(this, "Error occured", "Cannot restore graph view");
    }
}

//=========================================================================
void BlastGraphView::onReadsThresholdChanged(quint32 oldT, quint32 newT)
{
    if ( reads_threshold == newT )
        return;
    reads_threshold = newT;
    getTaxNodeSignalSender(NULL)->sendSignals = false;
    if ( oldT == newT )
        return;
    if ( oldT < newT )
        hideNodes(oldT, newT);
    else
        unhideNodes(oldT, newT);
    getTaxNodeSignalSender(NULL)->sendSignals = true;
}

//=========================================================================
void BlastGraphView::onBubbleSizeChanged(quint32, quint32 newSize)
{
    getConfig()->bubbleSize = newSize;
    scene()->update(mapToScene(viewport()->geometry()).boundingRect());
    QList<QGraphicsItem *> items = scene()->items(mapToScene(viewport()->geometry()));
    foreach( QGraphicsItem *item, items)
    {
        if ( item->type() == BlastGraphNode::Type )
        {
            BlastGraphNode *gnode = (BlastGraphNode *)item;
            if ( gnode->getTaxNode()->reads > 0 )
                gnode->update();
        }
    }
}

//=========================================================================
NodePositionKeeper::NodePositionKeeper(TreeGraphView *gv, BaseTaxNode *n):
    graphView(gv)
{
    gnode = n->getGnode();
    if ( gnode == NULL )
        return;
    pos = graphView->mapFromScene(gnode->pos());
}

//=========================================================================
NodePositionKeeper::~NodePositionKeeper()
{
    if ( gnode == NULL )
        return;
    QPointF p_new = graphView->mapFromScene(gnode->pos());
    graphView->verticalScrollBar()->setValue(graphView->verticalScrollBar()->value()+p_new.y()-pos.y());
    graphView->update();
}
