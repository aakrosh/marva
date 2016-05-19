#ifndef TREETAXNODE_H
#define TREETAXNODE_H

#include "base_tax_node.h"


class TreeTaxNode;
class TaxTreeGraphNode;

class ChildrenList : public ThreadSafeList<TreeTaxNode *>{};

class TreeTaxNode : public BaseTaxNode
{
protected:
    bool collapsed;
public:
    ChildrenList children;
    TreeTaxNode *parent;

    TreeTaxNode(bool _collapsed);
    virtual bool isCollapsed() {return collapsed; }
    void setCollapsed(bool b, bool updateGnode);
    virtual int getLevel() = 0;
    virtual void setLevel(int level) = 0;
    virtual TreeTaxNode *addChild(TreeTaxNode *node);
    virtual void mergeWith(TreeTaxNode *other, TreeGraphView *gview);
    virtual inline TaxTreeGraphNode *getTaxTreeGNode() { return (TaxTreeGraphNode *) getGnode(); }

};

#endif // TREETAXNODE_H
