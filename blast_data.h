#ifndef BLAST_DATA
#define BLAST_DATA

#include "blast_record.h"
#include "loader_thread.h"
#include "tax_map.h"
#include "common.h"
#include "tree_tax_node.h"

#include <QList>
#include <QVector>
#include <QMap>
#include <QReadWriteLock>

class BlastTaxDataProvider;

typedef QMap<quint32, quint32> TaxQuantity;

class BlastTaxNode;

class BlastNodeMap : public QMap<qint32, BlastTaxNode *>
{
public:
    quint32 max_reads;
    BlastNodeMap() : QMap<qint32, BlastTaxNode *>(), max_reads(0) {}
};

class BlastTaxNode : public TreeTaxNode
{
public:
    quint32 reads;
    TreeTaxNode *tNode;
    QVector<qint64> positions;
    BlastTaxNode(TreeTaxNode *refNode, int _count, BlastNodeMap *map);
    virtual ~BlastTaxNode(){}
    BlastTaxNode *createPathToNode(BlastNodeMap *blastNodeMap);

    virtual QString getName() { return tNode->getName(); }
    virtual qint32 getId() { return tNode->getId(); }
    virtual int getLevel() { return tNode->getLevel(); }
    virtual void setLevel(int) { } //The level of tNode is already set and could not be modified
    virtual GraphNode *createGnode(TreeGraphView *gv);
    virtual QString getText() { return tNode->getText(); }
    BlastTaxNode *clone();
    quint32 sum();
};

#endif // BLAST_DATA

