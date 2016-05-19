#include "tree_tax_node.h"
#include "graph_node.h"
#include "taxnodesignalsender.h"
#include "graphview.h"

//=========================================================================
TreeTaxNode::TreeTaxNode(bool _collapsed):
    BaseTaxNode(),
    collapsed(_collapsed),
    parent(NULL)
{

}

//=========================================================================
TreeTaxNode *TreeTaxNode::addChild(TreeTaxNode *node)
{
    children.Add(node);
    node->setLevel(getLevel()+1);
    node->parent = this;
    return node;
}

//=========================================================================
void TreeTaxNode::setCollapsed(bool b, bool updateGnode)
{
    if ( collapsed == b )
        return;
    collapsed = b;
    TaxTreeGraphNode *ttgnode = getTaxTreeGNode();
    if ( ttgnode != NULL && updateGnode )
        ttgnode->onNodeCollapsed(b);
    getTaxNodeSignalSender(this)->CollapsedChanged(b);
}

//=========================================================================
void TreeTaxNode::mergeWith(TreeTaxNode *other, TreeGraphView *gview)
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
            ThreadSafeListLocker<TreeTaxNode *> locker(&children);
            for ( TaxNodeIterator this_it = children.begin(); this_it<children.end(); this_it++ )
            {
                if ((*this_it)->getId() == (*o_it)->getId())
                {
                    TreeTaxNode *n = *this_it;
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
    TaxTreeGraphNode *ttgnode = getTaxTreeGNode();
    if ( ttgnode != NULL && changed )
        ttgnode->markDirty(DIRTY_CHILD, &gview->dirtyList);
}



