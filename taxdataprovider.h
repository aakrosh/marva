#ifndef TAXDATAPROVIDER_H
#define TAXDATAPROVIDER_H

#include "tax_map.h"
#include "tree_tax_node.h"

#include <QList>
#include <QReadWriteLock>
#include <QColor>
#include <QVariant>
#include <QObject>
#include <QThread>
#include <QJsonArray>
#include <QJsonObject>
#include <QFile>

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
    quint32 current_tax_id;
    TaxDataProvider(QObject *parent, TaxDataProviderType type = UNKNOWN);
    virtual ~TaxDataProvider();
    virtual quint32 count();
    virtual qint32 id(quint32 index);
    virtual BaseTaxNode *taxNode(qint32 index);
    virtual BaseTaxNode *taxNodeById(qint32 id);
    virtual BaseTaxNode *currentTaxNode() { return taxNodeById(current_tax_id); }
    virtual quint32 reads(quint32 index);
    virtual quint32 sum(quint32 index);
    virtual quint32 sumById(quint32 id);
    virtual QString text(quint32 index);
    virtual quint32 readsById(quint32 id);
    virtual qint32 indexOf(qint32 id);
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
    void dataChanged();
};

extern TaxMap taxMap;
// Provides data for global taxonomy tree
class GlobalTaxMapDataProvider : public TaxDataProvider
{
    Q_OBJECT
public:
    TaxMap *taxMap;    
    QReadWriteLock lock;
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
    QString fileName;
    quint64 totalReads;
    BlastTaxDataProvider(QObject *parent);
    ~BlastTaxDataProvider();
    virtual quint32 reads(quint32 index);
    virtual quint32 readsById(quint32 id);
    virtual quint32 sum(quint32 index);
    virtual quint32 sumById(quint32 id);
    virtual void updateCache(bool values_only);
    virtual QColor color(int index);
    virtual void sort(int column, Qt::SortOrder order);
    virtual quint32 getMaxReads();
    virtual void addParent();
    virtual void removeParent();
    BlastTaxNode *addTaxNode(qint32 id, qint32 reads, quint64 pos);
    BlastTaxNode *addTaxNode(qint32 id, qint32 reads, QVector<quint64> &pos);
    virtual void toJson(QJsonObject &json);
    virtual void fromJson(QJsonObject &json);
    BlastTaxNode *nodeById(qint32 id);
    void serialize(QFile &file);
    void deserialize(QFile &file, qint32 version);    

public slots:
    void onBlastProgress(void *);
    void onBlastLoaded(void *);

    friend class BlastDataTreeLoader;
};

class BlastTaxDataProviders : public QList<BlastTaxDataProvider *>
{
    quint64 visibility_mask;
    QJsonArray jProviders;
    QJsonObject *big_json;
    int serializingProviders;
public:
    BlastTaxDataProviders();
    virtual void fromJson(QJsonObject &json);
    BlastTaxDataProvider *providerByName(const QString &name) const;
    void setVisible(quint8 index, bool visible);
    bool isVisible(quint8 index);
    void addProvider(BlastTaxDataProvider *);
};

class ProvidersSerializationThread : public QThread
{
    Q_OBJECT
    BlastTaxDataProvider *provider;
public:
    QJsonObject json;
    ProvidersSerializationThread(BlastTaxDataProvider *p);
    virtual void run();
signals:
    void completed(ProvidersSerializationThread *);
};

extern BlastTaxDataProviders blastTaxDataProviders;

#endif // TAXDATAPROVIDER_H
