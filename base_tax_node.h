#ifndef BASETAXNODE_H
#define BASETAXNODE_H

#include "threadsafelist.h"

#include <QList>
#include <QString>

class TreeGraphView;
class GraphNode;

class BaseTaxNode
{
public:
    GraphNode *gnode;

    BaseTaxNode();
    virtual ~BaseTaxNode(){}

    virtual void removeGnode() { gnode = NULL; }
    virtual GraphNode *getGnode() { return gnode; }
    virtual GraphNode *createGnode(TreeGraphView *gv) = 0;
    // Abstract methods
    virtual QString getName() = 0;
    virtual qint32 getId() = 0;
    virtual QString getText() = 0;
    virtual void setVisible(bool v, bool force=false);
    virtual bool visible() { return is_visible; }
protected:
    bool is_visible;

    friend class TaxTreeGraphNode;
    friend class GraphNode;
};

#endif // BASETAXNODE_H
