#ifndef TAXDATAPROVIDER_H
#define TAXDATAPROVIDER_H

#include "tax_map.h"

#include <QList>
#include <QReadWriteLock>
#include <QColor>
#include <QVariant>
#include <QObject>

class BaseTaxNode;
class BlastNodeMap;

class IdTaxNodePair
{
public:
    BaseTaxNode *node;
    qint32 id;
    IdTaxNodePair(BaseTaxNode *n, qint32 _id):node(n), id(_id){}
};

typedef QList<IdTaxNodePair> IdTaxNodeList;

enum TaxDataProviderType
{
    UNKNOWN              = -1,
    GLOBAL_DATA_PROVIDER = 0,
    BLAST_DATA_PROVIDER  = 1
};

class TaxDataProvider : public QObject
{
    Q_OBJECT
protected:
    IdTaxNodeList idTaxNodeList;
public:
    TaxDataProvider(QObject *parent, TaxDataProviderType type = UNKNOWN);
    virtual ~TaxDataProvider(){}
    virtual quint32 count();
    virtual qint32 id(quint32 index);
    virtual BaseTaxNode *taxNode(quint32 index);
    virtual quint32 reads(quint32 index);
    virtual QString text(quint32 index);
    virtual quint32 readsById(quint32 id);
    virtual quint32 indexOf(qint32 id);
    virtual void updateCache(bool /*ids_only*/) {}
    virtual QColor color(int /*index*/) { return Qt::white; }
    virtual QVariant checkState(int /*index*/);
    virtual void setCheckedState(int /*index*/, QVariant /*value*/);
    virtual void sort(int column, Qt::SortOrder order);
    virtual quint32 getMaxReads();
    TaxDataProviderType type;
public slots:
    void onDataLoaded();
signals:
    dataChanged();
};

extern TaxMap taxMap;
// Provides data for global taxonomy tree
class GlobalTaxMapDataProvider : public TaxDataProvider
{
    Q_OBJECT
public:
    TaxMap *taxMap;
    GlobalTaxMapDataProvider(QObject *parent, TaxMap *tm);
    virtual void updateCache(bool values_only);
public slots:
    void onTreeLoaded();
    void onMapChanged();
};

// Provides data for blast tree
class BlastTaxDataProvider : public TaxDataProvider
{
    Q_OBJECT
    QReadWriteLock lock;
    BlastNodeMap *blastNodeMap;
public:
    QString name;
    BlastTaxDataProvider(QObject *parent);
    virtual quint32 reads(quint32 index);
    virtual quint32 readsById(quint32 id);
    virtual void updateCache(bool values_only);
    virtual QColor color(int index);
    virtual void sort(int column, Qt::SortOrder order);
    virtual quint32 getMaxReads();

public slots:
    void onBlastProgress(void *);
    void onBlastLoaded(void *);

    friend class BlastDataTreeLoader;
};

typedef QList<BlastTaxDataProvider*> BlastTaxDataProviders;

#endif // TAXDATAPROVIDER_H
