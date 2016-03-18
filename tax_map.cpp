#include "tax_map.h"
#include "graph_node.h"
#include "graphwidget.h"
#include "edge.h"

#include <QString>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QMessageBox>

//=========================================================================
TaxMap::TaxMap() : QMap<qint32, TaxNode *>(){}

//=========================================================================
void TaxMap::setName(qint32 tid, const char *name)
{
    TaxMapIterator it = find(tid);
    if ( it == end() )
    {
        QMessageBox::information(0, "Cannot set name", QString("No node with id %1 found").arg(tid));
        return;
    }
    it.value()->text = name;
}

//=========================================================================
TaxNodeVisitor::TaxNodeVisitor(
        VisitorDirection _direction,
        bool visit_collapsed,
        GraphView *gv,
        bool createGNodes,
        bool _visitNullGnodes) :
    direction(_direction),
    createGraphNodes(createGNodes),
    visitCollapsed(visit_collapsed),
    graphView(gv),
    visitNullGnodes(_visitNullGnodes)
{

}

//=========================================================================
bool TaxNodeVisitor::shouldVisitChildren(BaseTaxNode *node)
{
    if ( node->children.isEmpty() )
        return false;
    else
        return this->visitCollapsed || !node->isCollapsed();
}

//=========================================================================
void TaxNodeVisitor::VisitRootToLeaves(BaseTaxNode *node)
{
    if ( node->getGnode() == NULL )
    {
        if ( createGraphNodes )
            graphView->CreateGraphNode(node);
        else if ( !visitNullGnodes )
            return;

    }
    bool visit_children = shouldVisitChildren(node);
    Action(node);
    if ( visit_children )
    {
        beforeVisitChildren(node);
        ThreadSafeListLocker<BaseTaxNode *> locker(&node->children);
        for ( TaxNodeIterator it  = node->children.begin(); it != node->children.end(); it++ )
            VisitRootToLeaves(*it);
        afterVisitChildren(node);
    }
}

//=========================================================================
void TaxNodeVisitor::VisitLeavesToRoot(BaseTaxNode *node)
{
    if ( node->getGnode()  == NULL )
    {
        if ( createGraphNodes )
            graphView->CreateGraphNode(node);
        else if ( !visitNullGnodes )
            return;
    }
    bool visit_children = shouldVisitChildren(node);
    if ( visit_children )
    {
        beforeVisitChildren(node);
        ThreadSafeListLocker<BaseTaxNode *> locker(&node->children);
        for ( TaxNodeIterator it  = node->children.begin(); it != node->children.end(); it++ )
            VisitLeavesToRoot(*it);
        afterVisitChildren(node);
    }
    Action(node);
}

//=========================================================================
void TaxNodeVisitor::Visit(BaseTaxNode *node)
{
    if ( node == NULL )
        return;
    if ( direction == RootToLeaves )
        VisitRootToLeaves(node);
    else
        VisitLeavesToRoot(node);
}

//=========================================================================
TaxNode::TaxNode(): BaseTaxNode(true), id(-555), level(0){}

//=========================================================================
TaxNode::TaxNode(qint32 _id): BaseTaxNode(true), id(_id), level(0){}

//=========================================================================
TaxNode *TaxNode::TaxNode::addChildById(quint32 chId)
{
    return (TaxNode *)addChild(new TaxNode(chId));
}

//=========================================================================
GraphNode *TaxNode::createGnode(GraphView *gv)
{
    gnode = new GraphNode(gv, this);
    return gnode;
}
