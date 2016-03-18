#ifndef TAX_MAP_H
#define TAX_MAP_H

#include "base_tax_node.h"

#include <QMap>
#include <QReadWriteLock>
#include <QColor>
#include <QVariant>

class QString;
class GraphNode;
class GraphView;

class TaxNode : public BaseTaxNode
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
    virtual GraphNode *createGnode(GraphView *gv);

protected:
private:
    QString name;
    qint32 id;
    int level;
    QString text;

    friend class TaxNodeVisitor;
    friend class TreeLoaderThread;
    friend class TaxMap;
    friend class GraphNode;
};

typedef QList<BaseTaxNode *>::iterator TaxNodeIterator;

enum VisitorDirection
{
    RootToLeaves,
    LeavesToRoot
};

class TaxNodeVisitor
{
public:
    TaxNodeVisitor(VisitorDirection _direction, bool visit_collapsed=false, GraphView *gv=NULL, bool createGNodes=false, bool visitNullGnodes = true);
    virtual void Action(BaseTaxNode *root) = 0;
    void Visit(BaseTaxNode *node);
private:
    VisitorDirection direction;
protected:
    bool createGraphNodes;
    bool visitCollapsed;
    GraphView *graphView;
    bool visitNullGnodes;
    void VisitRootToLeaves(BaseTaxNode *node);
    void VisitLeavesToRoot(BaseTaxNode *node);
    bool shouldVisitChildren(BaseTaxNode *node);
    virtual void beforeVisitChildren(BaseTaxNode *){}
    virtual void afterVisitChildren(BaseTaxNode *){}
};
/*
class TaxMap : public QMap<qint32, QString>
{
public:
    TaxMap();
    void insertName(qint32 tid, const char *name);
};
*/
class TaxMap : public QMap<qint32, TaxNode *>
{
public:
    TaxMap();
//    void insertName(qint32 tid, const char *name);
    void setName(qint32 tid, const char *name);
};

typedef TaxMap::const_iterator TaxMapIterator;

class TaxDataProvider
{
public:
    TaxDataProvider(){}
    virtual ~TaxDataProvider(){}
    virtual quint32 count() = 0;
    virtual qint32 id(quint32 index) = 0;
    virtual BaseTaxNode *taxNode(quint32 index) = 0;
    virtual quint32 reads(quint32 index) = 0;
    virtual quint32 indexOf(qint32 id) = 0;
    virtual void updateCache(bool /*ids_only*/) {}
    virtual QColor color(int /*index*/) { return Qt::white; }
    virtual QVariant checkState(int /*index*/) { return QVariant(); }
    virtual void setCheckedState(int /*index*/, QVariant /*value*/) {}
};

#endif // TAX_MAP_H
