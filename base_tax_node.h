#ifndef BASETAXNODE_H
#define BASETAXNODE_H

#include "threadsafelist.h"

#include <QList>
#include <QString>

class GraphView;
class TaxTreeGraphNode;

class BaseTaxNode;

class ChildrenList : public ThreadSafeList<BaseTaxNode *>
{

};

class BaseTaxNode
{
protected:
    bool collapsed;
public:
    TaxTreeGraphNode *gnode;
    bool is_visible;
    ChildrenList children;
    BaseTaxNode *parent;

    BaseTaxNode(bool collapsed);
    virtual ~BaseTaxNode(){}
    virtual BaseTaxNode *addChild(BaseTaxNode *node);
    virtual bool isCollapsed() {return collapsed; }
    void setCollapsed(bool b, bool updateGnode);
    virtual void mergeWith(BaseTaxNode *other, GraphView *gview);

    virtual void removeGnode() { gnode = NULL; }
    virtual TaxTreeGraphNode *getGnode() { return gnode; }
    virtual TaxTreeGraphNode *createGnode(GraphView *gv) = 0;
    // Abstract methods
    virtual QString getName() = 0;
    virtual qint32 getId() = 0;
    virtual int getLevel() = 0;
    virtual void setLevel(int level) = 0;
    virtual QString getText() = 0;
    virtual void setVisible(bool v, bool force=false);
    virtual bool visible() { return is_visible; }

    friend class TaxTreeGraphNode;
    friend class GraphNode;
};

#endif // BASETAXNODE_H
