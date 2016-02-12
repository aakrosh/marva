#include "tax_map.h"
#include "graph_node.h"
#include "graphwidget.h"

#include <QString>
#include <QFile>
#include <QTextStream>
#include <QDebug>

TaxMap::TaxMap() : QMap<qint32, QString>(){}

void TaxMap::insertName(qint32 tid, const char *name)
{
    insert(tid, name);
    //TaxNode *node = insertTaxId(tid);
    //node->name = name;
}

TaxNodeVisitor::TaxNodeVisitor(VisitorDirection _direction) : direction(_direction) {}

void TaxNodeVisitor::VisitRootToLeaves(TaxNode *node)
{
    Action(node);
    for ( TaxNodeIterator it  = node->children.begin(); it != node->children.end(); it++ )
        VisitRootToLeaves(*it);
}

void TaxNodeVisitor::VisitLeavesToRoot(TaxNode *node)
{
    for ( TaxNodeIterator it  = node->children.begin(); it != node->children.end(); it++ )
        VisitLeavesToRoot(*it);
    Action(node);
}

void TaxNodeVisitor::Visit(TaxNode *node)
{
    if ( direction == RootToLeaves )
        VisitRootToLeaves(node);
    else
        VisitLeavesToRoot(node);
}


TaxNode::TaxNode(): id(-555), level(0), gnode(NULL), parent(NULL), collapsed(true){}

TaxNode::TaxNode(qint32 _id):id(_id), level(0), gnode(NULL), parent(NULL), collapsed(true){}

TaxNode *TaxNode::addChild(quint32 chId)
{
    return addChild(new TaxNode(chId));
}

TaxNode *TaxNode::addChild(TaxNode *node)
{
    children.append(node);
    node->level = level+1;
    node->parent = this;
    return node;
}

void TaxNode::setCollapsed(bool b)
{
    collapsed = b;
    if ( gnode != NULL )
        gnode->onNodeCollapsed(b);
}

void TaxNode::mergeWith(TaxNode *other, GraphView *gview)
{
    if ( id != other->id )
        return;
    bool changed = false;
    for ( TaxNodeIterator o_it = other->children.begin(); o_it<other->children.end(); o_it++ )
    {
        bool found = false;
        for ( TaxNodeIterator this_it = children.begin(); this_it<children.end(); this_it++ )
        {
            if ((*this_it)->id == (*o_it)->id)
            {
                (*this_it)->mergeWith(*o_it, gview);
                found = true;
                break;
            }
        }
        if ( found )
            continue;
        addChild(*o_it);
        changed = true;
    }
    if ( gnode != NULL && changed )
        gnode->markDirty(DIRTY_CHILD, &gview->dirtyList);
}
