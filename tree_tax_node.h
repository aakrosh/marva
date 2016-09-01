#ifndef TREETAXNODE_H
#define TREETAXNODE_H

#include "base_tax_node.h"


class TreeTaxNode;
class TaxTreeGraphNode;

class ChildrenList : public ThreadSafeList<TreeTaxNode *>{};

#define TTN_COLLAPSED           1
#define TTN_DIRTY_CHILDREN      2
#define TTN_VISIBLE_CHILDREN    4

class TreeTaxNode : public BaseTaxNode
{
protected:
    //bool collapsed;
    quint8  flags;
public:
    ChildrenList children;
    TreeTaxNode *parent;

    TreeTaxNode(bool collapsed);
    virtual bool isCollapsed() {/*return collapsed;*/ return (flags & TTN_COLLAPSED) == TTN_COLLAPSED; }
    virtual void setCollapsed(bool b, bool updateGnode);
    virtual bool hasVisibleChildren();
    virtual void markChildrenDirty() { flags |= TTN_DIRTY_CHILDREN; }
    virtual void setVisible(bool v, bool force=false);
    virtual int getLevel() = 0;
    virtual void setLevel(int level) = 0;
    virtual TreeTaxNode *addChild(TreeTaxNode *node);
    virtual void mergeWith(TreeTaxNode *other, TreeGraphView *gview);
    virtual inline TaxTreeGraphNode *getTaxTreeGNode() { return (TaxTreeGraphNode *) getGnode(); }
};

typedef QList<TreeTaxNode *>::iterator TaxNodeIterator;

enum VisitorDirection{ RootToLeaves, LeavesToRoot };

class TaxNodeVisitor
{
public:
    TaxNodeVisitor(VisitorDirection _direction, bool visit_collapsed=false, TreeGraphView *gv=NULL, bool createGNodes=false, bool visitNullGnodes = true, bool _visit_invisible = true);
    virtual void Action(TreeTaxNode *root) = 0;
    void Visit(TreeTaxNode *node);
private:
    VisitorDirection direction;
protected:
    bool createGraphNodes;
    bool visitCollapsed;
    TreeGraphView *graphView;
    bool visitNullGnodes;
    bool visit_invisible;
    void VisitRootToLeaves(TreeTaxNode *node);
    void VisitLeavesToRoot(TreeTaxNode *node);
    bool shouldVisitChildren(TreeTaxNode *node);
    virtual void beforeVisitChildren(TreeTaxNode *){}
    virtual void afterVisitChildren(TreeTaxNode *){}
};

#endif // TREETAXNODE_H
