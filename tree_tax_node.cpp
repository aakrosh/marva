#include "tree_tax_node.h"
#include "graph_node.h"
#include "taxnodesignalsender.h"
#include "graphview.h"

//=========================================================================
TreeTaxNode::TreeTaxNode(bool collapsed):
    BaseTaxNode(),
    flags(TTN_DIRTY_CHILDREN),
    parent(NULL)
{
    if ( collapsed )
        flags |= TTN_COLLAPSED;
}

//=========================================================================
TreeTaxNode *TreeTaxNode::addChild(TreeTaxNode *node)
{
    children.Add(node);
    node->setLevel(getLevel()+1);
    node->parent = this;
    node->flags |= TTN_DIRTY_CHILDREN;
    return node;
}

inline void setFlag(quint8 &var, quint8 flags, bool set)
{
    if ( set )
        var |= flags;
    else
        var &= ~flags;
}
//=========================================================================
void TreeTaxNode::setCollapsed(bool b, bool updateGnode)
{
    if ( isCollapsed() == b )
        return;
    setFlag(flags, TTN_COLLAPSED, b);
    TaxTreeGraphNode *ttgnode = getTaxTreeGNode();
    if ( ttgnode != NULL && updateGnode )
        ttgnode->onNodeCollapsed(b);
    getTaxNodeSignalSender(this)->CollapsedChanged(b);
}

//=========================================================================
bool TreeTaxNode::hasVisibleChildren()
{
    if ( (flags & TTN_DIRTY_CHILDREN) == 0 )
        return (flags & TTN_VISIBLE_CHILDREN) == TTN_VISIBLE_CHILDREN;
    flags &= ~TTN_DIRTY_CHILDREN;
    bool has_visible_children = false;
    for ( ChildrenList::iterator it = children.begin(); it != children.end(); ++it )
    {
        if ( (*it)->is_visible )
        {
            has_visible_children = true;
            break;
        }
    }
    setFlag(flags, TTN_VISIBLE_CHILDREN, has_visible_children);
    return has_visible_children;
}

//=========================================================================
void TreeTaxNode::setVisible(bool v, bool force)
{
    if ( parent != NULL )
        parent->markChildrenDirty();
    BaseTaxNode::setVisible(v, force);
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
