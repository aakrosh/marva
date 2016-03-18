#ifndef BLAST_DATA
#define BLAST_DATA

#include "blast_record.h"
#include "loader_thread.h"
#include "tax_map.h"


#include <QList>
#include <QMap>
#include <QReadWriteLock>

typedef QMap<quint32, quint32> TaxQuantity;
typedef QMap<qint32, quint32> TaxColorMap;


class TaxColorSrc : public TaxColorMap
{
public:
    virtual quint32 getColor(qint32 tax_id);
};
//extern TaxColorSrc colorSrc;
class BlastTaxNode;
typedef QMap<qint32, BlastTaxNode *> BlastNodeMap;

class BlastTaxNode : public BaseTaxNode
{
public:
    quint32 count;
    bool visible;
    TaxNode *tNode;
    BlastTaxNode(TaxNode *refNode, int _count, BlastNodeMap *map);
    virtual ~BlastTaxNode(){}
    BlastTaxNode *createPathToNode(BlastNodeMap *blastNodeMap);

    virtual QString getName() { return tNode->getName(); }
    virtual qint32 getId() { return tNode->getId(); }
    virtual int getLevel() { return tNode->getLevel(); }
    virtual void setLevel(int) { } //The level of tNode is already set and could not be modified
    // TODO: Fix me
    virtual GraphNode *createGnode(GraphView *gv);
    virtual QString getText() { return tNode->getText(); }
};

class BlastDataTreeLoader: public LoaderThread
{
private:
    BlastFileType type;
    BlastTaxNode *root;
public:
    BlastDataTreeLoader(QObject *parent, QString fileName, BlastFileType _type);
    ~BlastDataTreeLoader();
    BlastNodeMap *blastNodeMap;
protected:
    virtual void processLine(QString &line);

};
class QReadWriteLocker
{
    QReadWriteLock *lock;
public:
    QReadWriteLocker(QReadWriteLock *_lock, bool write=false): lock(_lock)
    {
        if ( write )
            lock->lockForWrite();
        else
            lock->lockForRead();
    }
    ~QReadWriteLocker()
    {
        lock->unlock();
    }
};

extern TaxColorSrc taxColorSrc;

class BlastTaxDataProvider : public TaxDataProvider
{
    QReadWriteLock lock;
    BlastNodeMap *blastNodeMap;
    QList<BlastTaxNode *> taxnodes;
    QList<int> ids;
public:
    BlastTaxDataProvider(BlastNodeMap *bnm);
    virtual quint32 count();
    virtual qint32 id(quint32 index);
    virtual BaseTaxNode *taxNode(quint32 index);
    virtual quint32 reads(quint32 index);
    virtual quint32 indexOf(qint32 id);
    virtual void updateCache(bool values_only);
    virtual QColor color(int index);
    virtual QVariant checkState(int index);

    virtual void setCheckedState(int index, QVariant value);

};

#endif // BLAST_DATA

