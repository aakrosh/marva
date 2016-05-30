#include "tax_map.h"
#include "graph_node.h"
#include "graphview.h"
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
TaxNodeVisitor::TaxNodeVisitor(VisitorDirection _direction,
        bool visit_collapsed,
        TreeGraphView *gv,
        bool createGNodes,
        bool _visitNullGnodes,
        bool _visit_invisible) :
    direction(_direction),
    createGraphNodes(createGNodes),
    visitCollapsed(visit_collapsed),
    graphView(gv),
    visitNullGnodes(_visitNullGnodes),
    visit_invisible(_visit_invisible)
{

}

//=========================================================================
bool TaxNodeVisitor::shouldVisitChildren(TreeTaxNode *node)
{
    if ( node->children.isEmpty() )
        return false;
    else if ( !visitCollapsed && node->isCollapsed() )
        return false;
    else if ( !visit_invisible && !node->is_visible )
        return false;
    return true;
}

//=========================================================================
void TaxNodeVisitor::VisitRootToLeaves(TreeTaxNode *node)
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
        ThreadSafeListLocker<TreeTaxNode *> locker(&node->children);
        for ( TaxNodeIterator it  = node->children.begin(); it != node->children.end(); it++ )
            VisitRootToLeaves(*it);
        afterVisitChildren(node);
    }
}

//=========================================================================
void TaxNodeVisitor::VisitLeavesToRoot(TreeTaxNode *node)
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
        ThreadSafeListLocker<TreeTaxNode *> locker(&node->children);
        for ( TaxNodeIterator it  = node->children.begin(); it != node->children.end(); it++ )
            VisitLeavesToRoot(*it);
        afterVisitChildren(node);
    }
    Action(node);
}

//=========================================================================
void TaxNodeVisitor::Visit(TreeTaxNode *node)
{
    if ( node == NULL )
        return;
    if ( direction == RootToLeaves )
        VisitRootToLeaves(node);
    else
        VisitLeavesToRoot(node);
}

//=========================================================================
TaxNode::TaxNode(): TreeTaxNode(true), id(-555), level(0){}

//=========================================================================
TaxNode::TaxNode(qint32 _id): TreeTaxNode(true), id(_id), level(0){}

//=========================================================================
TaxNode *TaxNode::TaxNode::addChildById(quint32 chId)
{
    return (TaxNode *)addChild(new TaxNode(chId));
}

//=========================================================================
GraphNode *TaxNode::createGnode(TreeGraphView *gv)
{
    gnode = new TaxTreeGraphNode(gv, this);
    return gnode;
}
