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
    inline bool operator==(const IdTaxNodePair &x){ return id == x.id; }
    inline bool operator==(qint32 other_id){ return id == other_id; }
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
    QReadWriteLock lock;
public:
    IdTaxNodeList idTaxNodeList;
    QString name;
    TaxDataProvider(QObject *parent, TaxDataProviderType type = UNKNOWN);
    virtual ~TaxDataProvider();
    virtual quint32 count();
    virtual qint32 id(quint32 index);
    virtual BaseTaxNode *taxNode(quint32 index);
    virtual quint32 reads(quint32 index);
    virtual quint32 sum(quint32 index);
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
    TaxNode *taxTree;
    GlobalTaxMapDataProvider(QObject *parent, TaxMap *tm);
    virtual void updateCache(bool values_only);
public slots:
    void onTreeLoaded();
    void onMapChanged();
};

extern GlobalTaxMapDataProvider *globalTaxDataProvider;
class BlastTaxNode;

// Provides data for blast tree
class BlastTaxDataProvider : public TaxDataProvider
{
    Q_OBJECT
    BlastNodeMap *blastNodeMap;
    quint32 parent_count;
public:
    BlastTaxNode *root;
    BlastTaxDataProvider(QObject *parent);
    ~BlastTaxDataProvider();
    virtual quint32 reads(quint32 index);
    virtual quint32 sum(quint32 index);
    virtual quint32 readsById(quint32 id);
    virtual void updateCache(bool values_only);
    virtual QColor color(int index);
    virtual void sort(int column, Qt::SortOrder order);
    virtual quint32 getMaxReads();
    virtual void addParent();
    virtual void removeParent();
    BlastTaxNode *addTaxNode(qint32 id, qint32 reads, qint64 pos = -1);
    BlastTaxNode *addTaxNode(qint32 id, qint32 reads, QVector<qint64> pos);
    virtual void toJson(QJsonObject &json);
    virtual void fromJson(QJsonObject &json);
    BlastTaxNode *nodeById(qint32 id);

public slots:
    void onBlastProgress(void *);
    void onBlastLoaded(void *);

    friend class BlastDataTreeLoader;
};

class BlastTaxDataProviders : public QList<BlastTaxDataProvider *>
{
    quint64 visibility_mask;
public:
    BlastTaxDataProviders();
    virtual void toJson(QJsonObject &json);
    virtual void fromJson(QJsonObject &json);
    BlastTaxDataProvider *providerByName(const QString &name) const;
    void setVisible(quint8 index, bool visible);
    bool isVisible(quint8 index);
    void addProvider(BlastTaxDataProvider *);
};

extern BlastTaxDataProviders blastTaxDataProviders;

#endif // TAXDATAPROVIDER_H
