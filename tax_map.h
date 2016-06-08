#ifndef TAX_MAP_H
#define TAX_MAP_H

#include "tree_tax_node.h"

#include <QMap>
#include <QReadWriteLock>
#include <QColor>
#include <QVariant>

class QString;
class TaxTreeGraphNode;
class TreeGraphView;

class TaxNode : public TreeTaxNode
{
public:
    TaxNode();
    TaxNode(qint32 _id);
    TaxNode *addChildById(quint32 chId);
    virtual QString getName() { return name; }
    virtual qint32 getId() { return id; }
    virtual int getLevel() { return level; }
    virtual void setLevel(int _level) { level = _level; }
    virtual QString getText() { return text; }
    virtual GraphNode *createGnode(TreeGraphView *gv);

protected:
private:
    QString name;
    qint32 id;
    int level;
    QString text;

    friend class TaxNodeVisitor;
    friend class TreeLoaderThread;
    friend class TaxMap;
    friend class TaxTreeGraphNode;
};

typedef QList<TreeTaxNode *>::iterator TaxNodeIterator;

enum VisitorDirection
{
    RootToLeaves,
    LeavesToRoot
};

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

class TaxMap : public QMap<qint32, TaxNode *>
{
public:
    TaxMap();
    void setName(qint32 tid, const char *name);
};

typedef TaxMap::const_iterator TaxMapIterator;

#endif // TAX_MAP_H
