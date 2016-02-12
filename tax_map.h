#ifndef TAX_MAP_H
#define TAX_MAP_H

#include <QMap>
#include <QReadWriteLock>

class QString;
class GraphNode;
class GraphView;

class TaxNode
{
public:
    qint32 id;
    TaxNode();
    TaxNode(qint32 _id);
    QString name;
    int level;
    QList<TaxNode *> children;
    GraphNode *gnode;
    TaxNode *parent;
    TaxNode *addChild(quint32 chId);
    TaxNode *addChild(TaxNode *node);
    inline bool isCollapsed() {return collapsed; }
    void setCollapsed(bool b);
    void mergeWith(TaxNode *other, GraphView *gview);
protected:
private:
    bool collapsed;
};

typedef QList<TaxNode *>::iterator TaxNodeIterator;

enum VisitorDirection
{
    RootToLeaves,
    LeavesToRoot
};

class TaxNodeVisitor
{
public:
    TaxNodeVisitor(VisitorDirection _direction);
    virtual void Action(TaxNode *root) = 0;
    void Visit(TaxNode *node);
private:
    VisitorDirection direction;
    void VisitRootToLeaves(TaxNode *node);
    void VisitLeavesToRoot(TaxNode *node);

};

class TaxMap : public QMap<qint32, QString>
{
public:
    TaxMap();
    void insertName(qint32 tid, const char *name);
};

typedef TaxMap::const_iterator TaxMapIterator;
#endif // TAX_MAP_H
