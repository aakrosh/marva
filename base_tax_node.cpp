#include "base_tax_node.h"
#include "graph_node.h"
#include "graphwidget.h"

//=========================================================================
BaseTaxNode::BaseTaxNode(bool _collapsed): collapsed(_collapsed), gnode(NULL), parent(NULL)
{

}

//=========================================================================
BaseTaxNode *BaseTaxNode::addChild(BaseTaxNode *node)
{
    children.Add(node);
    node->setLevel(getLevel()+1);
    node->parent = this;
    return node;
}

//=========================================================================
void BaseTaxNode::setCollapsed(bool b, bool updateGnode)
{
    if ( collapsed == b )
        return;
    collapsed = b;
    if ( gnode != NULL && updateGnode )
        gnode->onNodeCollapsed(b);
}

//=========================================================================
void BaseTaxNode::mergeWith(BaseTaxNode *other, GraphView *gview)
{
    if ( other == NULL )
        return;
    if ( getId() != other->getId() )
        return;
    if ( children.count() == other->children.count() )
        return;
    bool changed = false;
    for ( TaxNodeIterator o_it = other->children.begin(); o_it<other->children.end(); o_it++ )
    {
        bool found = false;
        {
            ThreadSafeListLocker<BaseTaxNode *> locker(&children);
            for ( TaxNodeIterator this_it = children.begin(); this_it<children.end(); this_it++ )
            {
                if ((*this_it)->getId() == (*o_it)->getId())
                {
                    BaseTaxNode *n = *this_it;
                    n->mergeWith(*o_it, gview);
                    found = true;
                    break;
                }
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
