#include "base_tax_node.h"
#include "graph_node.h"
#include "graphview.h"
#include "taxnodesignalsender.h"

//=========================================================================
BaseTaxNode::BaseTaxNode():gnode(NULL), is_visible(true)
{

}

//=========================================================================
void BaseTaxNode::setVisible(bool v, bool force)
{
    if ( is_visible == v && !force)
        return;
    is_visible = v;
    getTaxNodeSignalSender(this)->VisibilityChanged(v);
}
