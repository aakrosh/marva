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

enum TaxRank
{
    TR_ROOT     = -2,
    TR_DOMAIN   = -1,
    TR_NORANK2   = 0,
    TR_KINGDOM  = 1,
    TR_PHYLUM   = 2,
    TR_CLASS    = 3,
    TR_ORDER    = 4,
    TR_FAMILY   = 5,
    TR_VARIETAS = 98,
    TR_SPECIES  = 100,
    TR_SUBSPIC  = 101,
    TR_NORANK   = -666,
};

class TaxNode : public TreeTaxNode
{
public:
    TaxNode();
    TaxNode(qint32 _id, TaxRank rank);
    TaxNode *addChildById(quint32 chId, TaxRank rank  = TR_NORANK);
    virtual QString getName() { return name; }
    virtual qint32 getId() { return id; }
    virtual int getLevel() { return level; }
    virtual void setLevel(int _level) { level = _level; }
    virtual QString getText() { return text; }
    virtual GraphNode *createGnode(TreeGraphView *gv);
    TaxRank getRank() { return rank; }

protected:
private:
    QString name;
    qint32 id;
    int level;
    QString text;
    TaxRank rank;

    friend class TaxNodeVisitor;
    friend class TreeLoaderThread;
    friend class TaxMap;
    friend class TaxTreeGraphNode;
};


class TaxMap : public QMap<qint32, TaxNode *>
{
public:
    TaxMap();
    void setName(qint32 tid, const char *name, TaxRank rank = TR_NORANK);
};

typedef TaxMap::const_iterator TaxMapIterator;

#endif // TAX_MAP_H
