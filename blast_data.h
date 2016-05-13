#ifndef BLAST_DATA
#define BLAST_DATA

#include "blast_record.h"
#include "loader_thread.h"
#include "tax_map.h"
#include "common.h"

#include <QList>
#include <QMap>
#include <QReadWriteLock>

class BlastTaxDataProvider;

typedef QMap<quint32, quint32> TaxQuantity;
typedef QMap<qint32, quint32> TaxColorMap;


class TaxColorSrc : public TaxColorMap
{
public:
    virtual quint32 getColor(qint32 tax_id);
};
class BlastTaxNode;

class BlastNodeMap : public QMap<qint32, BlastTaxNode *>
{
public:
    quint32 max_reads;
    BlastNodeMap() : QMap<qint32, BlastTaxNode *>(), max_reads(0) {}
};

class BlastTaxNode : public BaseTaxNode
{
public:
    quint32 reads;
    TaxNode *tNode;
    BlastTaxNode(TaxNode *refNode, int _count, BlastNodeMap *map);
    virtual ~BlastTaxNode(){}
    BlastTaxNode *createPathToNode(BlastNodeMap *blastNodeMap);

    virtual QString getName() { return tNode->getName(); }
    virtual qint32 getId() { return tNode->getId(); }
    virtual int getLevel() { return tNode->getLevel(); }
    virtual void setLevel(int) { } //The level of tNode is already set and could not be modified
    virtual TaxTreeGraphNode *createGnode(GraphView *gv);
    virtual QString getText() { return tNode->getText(); }
    BlastTaxNode *clone();
};

class BlastDataTreeLoader: public LoaderThread
{
private:
    BlastFileType type;
    BlastTaxNode *root;
    BlastTaxDataProvider *dataProvider;
public:
    BlastDataTreeLoader(QObject *parent, QString fileName, BlastTaxDataProvider *dp, BlastFileType _type);
    ~BlastDataTreeLoader();
protected:
    virtual void processLine(QString &line);
    virtual void finishProcessing();

};

extern TaxColorSrc taxColorSrc;


#endif // BLAST_DATA

