#include "blast_data.h"
#include "main_window.h"
#include "graph_node.h"
#include "taxdataprovider.h"

BlastNodeMap blastNodeMap;
//=========================================================================
//*************************************************************************
//=========================================================================
BlastTaxNode::BlastTaxNode(TreeTaxNode *refNode, int _count, BlastNodeMap *blastNodeMap):
    TreeTaxNode(false),
    reads(_count),
    tNode(refNode)
{
    if ( blastNodeMap != NULL )
        blastNodeMap->insert(refNode->getId(), this);
}

//=========================================================================
BlastTaxNode *BlastTaxNode::clone()
{
    BlastTaxNode *n = new BlastTaxNode(tNode, reads, NULL);
    n->flags = flags;
    n->gnode = NULL;
    n->is_visible = is_visible;
    return n;
}

//=========================================================================
quint32 BlastTaxNode::sum()
{
    quint32 s = reads;
    for ( int i = 0; i < children.size(); i++ )
        s += ((BlastTaxNode *) children[i])->sum();
    return s;
}

//=========================================================================
BlastTaxNode *BlastTaxNode::createPathToNode(BlastNodeMap *blastNodeMap)
{
    BlastTaxNode *curNode = this;
    while ( curNode->tNode->parent != NULL && curNode->parent == NULL )
    {
        BlastNodeMap::iterator it = blastNodeMap->find(curNode->tNode->parent->getId());
        BlastTaxNode *cur_parent =
                it == blastNodeMap->end()
              ? new BlastTaxNode((TaxNode *)curNode->tNode->parent, 0, blastNodeMap)
              : it.value();
        cur_parent->addChild(curNode);
        curNode = cur_parent;
    }
    return curNode;
}

#include "graph_node.h"
//=========================================================================
GraphNode *BlastTaxNode::createGnode(TreeGraphView *gv)
{
    gnode = new BlastGraphNode(gv, this);
    return gnode;
}
